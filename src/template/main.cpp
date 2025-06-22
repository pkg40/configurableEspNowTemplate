/* main.cpp
 *------------------------------------------------------------------------------
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
#include <Arduino.h>

#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#else
  #error "Unsupported platform: This library only supports ESP32 and ESP8266."

#endif
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>

#include <beaconHandler.hpp>
#include <configManager2.h>
#include <webUI.h>
#include <ringBuffer.hpp>
#include <wifiHelper.hpp>
#include <espNowCoPilot.hpp>
#include <pairingManager.hpp>
#include <messageHandler.hpp>
#include <radioInterface.hpp>
#include <deviceDataPacket.h>

const String configFile = "/config.json";
configManager2 config;
AsyncWebServer server(80);
webUI ui(&config);

// Queues (shared across modules)
static ringBuffer<deviceDataPacket, DEVICE_MSG_BUFFER_SIZE> txQueue;
static ringBuffer<deviceDataPacket, DEVICE_MSG_BUFFER_SIZE> rxQueue;
static ringBuffer<deviceDataPacket, DEVICE_MSG_BUFFER_SIZE> handlerQueue;

// Pointers for dynamic setup
wifiHelper* wifi = nullptr;
espNowCoPilot<deviceDataPacket>* radio = nullptr;
pairingManager* pairing = nullptr;
messageHandler* messenger = nullptr;

beaconHandler beacon;

#include "espNowCallbacks.cpp"

void setup() {
    Serial.begin(115200);
    while (!Serial) delay(10);

    // Debug boot pause
    unsigned long start = millis();
    while (millis() - start < 10000) {
        Serial.print(".");
        delay(100);
    }

    // Load config
    config.begin(configFile, true);
    if (!heap_caps_check_integrity_all(true)) {
        Serial.println("🚨 Heap corruption detected");
    }

    // Start WiFi + UI
    wifi = new wifiHelper();
    if (wifi->begin(&config, true)) {
        ui.begin();
    }
    config.saveToJson(configFile.c_str(), config.getConfig());

    // Instantiate message handler
    static messageHandler messengerInstance(&txQueue, &rxQueue, &handlerQueue);
    messenger = &messengerInstance;

    // Temporarily initialize pairing with null radio
    static pairingManager pairingInstance(nullptr, messenger);
    pairing = &pairingInstance;

    // Create radio and inject pairing pointer
    static espNowCoPilot<deviceDataPacket> radioInstance(pairing, &rxQueue, &txQueue);
    radio = &radioInstance;

    // Inject back into pairing manager
    pairing->setRadio(radio);  // Add this setter to pairingManager

    // Start ESP-NOW
    int channel = config.getValue("espnow", "channel").toInt();
    radio->begin(channel, WIFI_MODE_STA, true);

#ifdef I_AM_A_BOSS
    Serial.println("[ROLE] Boss mode enabled");
    beacon.beginPairingWindow(&config, BEACON_TIMEOUT);
#else
    Serial.println("[ROLE] Worker mode enabled");
    beacon.begin(&config);
#endif
}

void loop() {
    if (messenger) messenger->loop();
    if (pairing) pairing->loop();
    if (radio) radio->loop();

#ifdef I_AM_A_BOSS
    if (!beacon.isPaired()) {
        beacon.processQueue();
    }
#else
    beacon.loop(millis());
#endif
}