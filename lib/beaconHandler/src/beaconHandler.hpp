
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

 #pragma once

#include <stdint.h>
#include <stddef.h>
#include <Arduino.h>
#include <mbedtls/md.h>
#include <globalConstants.h>

#if defined(ESP8266)
extern "C" {
#include <user_interface.h>
}
#include <ESP8266WiFi.h>
#include <espnow.h>
#elif defined(ESP32)
#include <esp_wifi.h>
#include <WiFi.h>
#include <esp_now.h>
#else
#error "Only support ESP32 and ESP8266"
#endif

#include "ringBuffer.hpp"
class configManager2;

#if defined(ESP32)
#define MY_IRAM_ATTR IRAM_ATTR
#else
#define MY_IRAM_ATTR
#endif

struct beaconPacket {
    uint8_t version = 1;
    uint8_t deviceType = 1;
    uint8_t mac[6];
    uint8_t sequenceId;
    uint8_t channel = 6;
    uint8_t hmac[32];
    bool unencrypted = true;
    uint8_t sharedSecret[8];  // Direct comparison in unencrypted mode
};

// Configurable beacon timing
constexpr unsigned long MIN_BEACON_INTERVAL_MS = 1000; // 1 Second
constexpr unsigned long MAX_BEACON_INTERVAL_MS = 1000*10; //10 Seconds
constexpr unsigned long BEACON_INTERVAL_MS = MAX_BEACON_INTERVAL_MS;  // Default
constexpr unsigned long BEACON_TIMEOUT = MAX_BEACON_INTERVAL_MS*10;     // Pairing window

class beaconHandler {
public:
    static beaconHandler* instance;

    beaconHandler();
    ~beaconHandler() = default;

    void begin(configManager2* cfg, uint8_t channel = 6);
    void loop(unsigned long);

    void beginPairing(configManager2* cfg);

    // Beacon emission
    void sendBeacon(bool verbose = false);
    void debugDump();

    // Static sniff callback (used in boss mode)
    static void MY_IRAM_ATTR sniffCallback(void* buf, wifi_promiscuous_pkt_type_t type);

private:
    // Role state
    bool isBoss = false;
    bool paired = false;
    bool broadcasting = false;

    // Wi-Fi & timing
    uint8_t wifiChannel = 6;
    uint8_t sequenceId = 0;
    unsigned long lastBeaconTime = 0;
    unsigned long beaconIntervalMs = BEACON_INTERVAL_MS;

    // Config and memory
    configManager2* config = nullptr;
    beaconPacket lastSentPacket{};

    // Packet authentication
    void signBeacon(beaconPacket& packet, const uint8_t* key, size_t len);
    bool validateHMAC(const beaconPacket& pkt);

    // Boss-only pairing
    ringBuffer<beaconPacket, DEVICE_MSG_BUFFER_SIZE> beaconBuffer;
    void queueCandidate(const beaconPacket& pkt);
    void processQueue();
};
