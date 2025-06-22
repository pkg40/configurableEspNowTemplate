/*
 * MIT License
 *
 * Copyright (c) 2025 Peter K Green (pkg40)
 * Email: pkg40@yahoo.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

 #include "beaconHandler.hpp"
#include "configManager2.h"
#include "ringBuffer.hpp"
#include "espNowCoPilot.hpp"
#include "platformCompat.hpp"
#include <mbedtls/md.h>
#include <Arduino.h>

beaconHandler* beaconHandler::instance = nullptr;
extern espNowCoPilot<deviceDataPacket> radio;

beaconHandler::beaconHandler() {}

void beaconHandler::begin(configManager2* cfg, uint8_t channel) {
    config = cfg;
    wifiChannel = channel;
    paired = false;
    broadcasting = true;
    isBoss = false;

    int userInterval = config->getValue("espnow", "beaconInterval").toInt();
    if (userInterval >= MIN_BEACON_INTERVAL_MS && userInterval <= MAX_BEACON_INTERVAL_MS) {
        beaconIntervalMs = static_cast<unsigned long>(userInterval);
    }

    setWiFiChannel(wifiChannel);
}

void beaconHandler::beginPairing(configManager2* cfg) {
    config = cfg;
    isBoss = true;
    paired = false;
    broadcasting = false;

    instance = this;
    WiFi.mode(WIFI_STA);
    esp_wifi_set_promiscuous(true);
    esp_wifi_set_promiscuous_filter(nullptr);
    esp_wifi_set_promiscuous_rx_cb(sniffCallback);

    Serial.println("[PAIR] Boss is listening for beacons...");
}

void beaconHandler::loop(unsigned long) {
    if (isBoss) {
        if (!paired) processQueue();
    } else {
        if (!broadcasting || millis() - lastBeaconTime < beaconIntervalMs) return;
        lastBeaconTime = millis();
        sendBeacon(true);
    }
}

void beaconHandler::sendBeacon(bool verbose) {
    beaconPacket pkt{};
    pkt.version = 1;
    pkt.deviceType = 1;
    pkt.sequenceId = ++sequenceId;
    pkt.channel = wifiChannel;

    WiFi.macAddress(pkt.mac);

    String secret = config->getValue("security", "secret");
    bool encrypt = config->getValue("security", "encrypt") == "true";
    pkt.unencrypted = !encrypt;

    if (encrypt && !secret.isEmpty()) {
        signBeacon(pkt, reinterpret_cast<const uint8_t*>(secret.c_str()), secret.length());
    } else {
        memset(pkt.hmac, 0, sizeof(pkt.hmac));
        memcpy(pkt.sharedSecret, secret.c_str(), sizeof(pkt.sharedSecret));
    }

    esp_now_send(nullptr, reinterpret_cast<uint8_t*>(&pkt), sizeof(pkt));
    lastSentPacket = pkt;

    if (verbose) {
        Serial.printf("[BEACON] Sent: #%u | Mode: %s\n",
                      pkt.sequenceId, pkt.unencrypted ? "CLEAR" : "SECURE");
    }
}

void beaconHandler::signBeacon(beaconPacket& packet, const uint8_t* key, size_t len) {
    const mbedtls_md_info_t* md = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    if (!md) return;

    mbedtls_md_context_t ctx;
    mbedtls_md_init(&ctx);

    if (mbedtls_md_setup(&ctx, md, 1) != 0) {
        mbedtls_md_free(&ctx);
        return;
    }

    mbedtls_md_hmac_starts(&ctx, key, len);
    mbedtls_md_hmac_update(&ctx, packet.mac, sizeof(packet.mac));
    mbedtls_md_hmac_finish(&ctx, packet.hmac);
    mbedtls_md_free(&ctx);
}

void beaconHandler::sniffCallback(void* buf, wifi_promiscuous_pkt_type_t type) {
    if (!instance || instance->paired || type != WIFI_PKT_MGMT) return;

    const wifi_promiscuous_pkt_t* pkt =
        reinterpret_cast<const wifi_promiscuous_pkt_t*>(buf);
    if (pkt->rx_ctrl.sig_len < sizeof(beaconPacket)) return;

    beaconPacket candidate;
    memcpy(&candidate, pkt->payload, sizeof(beaconPacket));

    Serial.printf("* [RX] MAC: %02X:%02X:%02X:%02X:%02X:%02X | Seq: %u | Mode: %s\n",
                  candidate.mac[0], candidate.mac[1], candidate.mac[2],
                  candidate.mac[3], candidate.mac[4], candidate.mac[5],
                  candidate.sequenceId,
                  candidate.unencrypted ? "CLEAR" : "SECURE");

    String encryptFlag = instance->config->getValue("security", "encrypt");
    bool expectSecure = (encryptFlag == "true");

    if (expectSecure && !instance->validateHMAC(candidate)) {
        Serial.println("[REJECT] Invalid HMAC");
        return;
    } else if (!expectSecure && !candidate.unencrypted) {
        Serial.println("[REJECT] Encrypted beacon but secure mode is disabled");
        return;
    }

    instance->queueCandidate(candidate);
}

bool beaconHandler::validateHMAC(const beaconPacket& pkt) {
    String secret = config->getValue("security", "secret");
    if (secret.isEmpty()) return false;

    uint8_t expected[32];
    const mbedtls_md_info_t* md = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    if (!md) return false;

    mbedtls_md_context_t ctx;
    mbedtls_md_init(&ctx);

    if (mbedtls_md_setup(&ctx, md, 1) != 0) {
        mbedtls_md_free(&ctx);
        return false;
    }

    mbedtls_md_hmac_starts(&ctx,
        reinterpret_cast<const uint8_t*>(secret.c_str()), secret.length());
    mbedtls_md_hmac_update(&ctx, pkt.mac, sizeof(pkt.mac));
    mbedtls_md_hmac_finish(&ctx, expected);
    mbedtls_md_free(&ctx);

    return memcmp(pkt.hmac, expected, sizeof(expected)) == 0;
}

void beaconHandler::queueCandidate(const beaconPacket& pkt) {
    beaconBuffer.push(const_cast<beaconPacket&>(pkt));
}

void beaconHandler::processQueue() {
    beaconPacket pkt;
    while (beaconBuffer.pop(pkt)) {
        String secret = config->getValue("security", "secret");
        if (secret.length() < sizeof(pkt.sharedSecret)) continue;

        if (!pkt.unencrypted && !validateHMAC(pkt)) continue;

        if (pkt.unencrypted &&
            memcmp(pkt.sharedSecret, secret.c_str(), sizeof(pkt.sharedSecret)) != 0) {
            Serial.println("[REJECT] Shared secret mismatch");
            continue;
        }

        char macStr[18];
        sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X",
                pkt.mac[0], pkt.mac[1], pkt.mac[2],
                pkt.mac[3], pkt.mac[4], pkt.mac[5]);

        config->setValue("espnow", "remotemac", String(macStr));
        config->setValue("espnow", "channel", String(pkt.channel));
        config->saveToJson("/config.json", config->getConfig());

        uint8_t lmk[16] = {0};  // placeholder
        radio.addPeer(pkt.mac, pkt.channel, lmk);

        paired = true;
        broadcasting = false;

        Serial.printf("[PAIRED] Registered peer on channel %u\n", pkt.channel);
        break;
    }
}

void beaconHandler::debugDump() {
    Serial.println(F("===== BeaconHandler DEBUG DUMP ====="));
    Serial.printf("Paired: %s\n", paired ? "YES" : "NO");
    Serial.printf("Broadcasting: %s\n", broadcasting ? "YES" : "NO");
    Serial.printf("Sequence ID: %u\n", sequenceId);

    char macStr[18];
    sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X",
            lastSentPacket.mac[0], lastSentPacket.mac[1], lastSentPacket.mac[2],
            lastSentPacket.mac[3], lastSentPacket.mac[4], lastSentPacket.mac[5]);

    Serial.printf("Last Beacon MAC: %s\n", macStr);
    Serial.printf("Last Shared Secret (hex): ");
    for (size_t i = 0; i < sizeof(lastSentPacket.sharedSecret); ++i)
        Serial.printf("%02X ", lastSentPacket.sharedSecret[i]);
    Serial.println();

    if (config) {
        Serial.printf("Config Secret: %s\n", config->getValue("security", "secret").c_str());
        Serial.printf("Stored MAC: %s\n", config->getValue("espnow", "remotemac").c_str());
        Serial.printf("Stored Channel: %s\n", config->getValue("espnow", "channel").c_str());
    } else {
        Serial.println("⚠️ Config pointer is null!");
    }

    Serial.printf("This = %p | Static instance = %p\n", this, instance);
    Serial.println(F("====================================="));
}