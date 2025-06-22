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
#include <Arduino.h>

#if defined(ESP32)
#include <WiFi.h>
#include <esp_wifi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#else
#error "Unsupported platform"
#endif

#include <ringBuffer.hpp>

#define USE_BROADCAST 1

#if USE_BROADCAST != 1
static uint8_t receiver[] = {0x12, 0x34, 0x56, 0x78, 0x90, 0x12}; // Unicast MAC address
#define DEST_ADDR receiver
#else
#define DEST_ADDR ESPNOW_BROADCAST_ADDRESS
#endif

static const uint32_t TIMEOUT = 25;

// WiFi Settings
constexpr bool ENABLE_WIFI = false;
constexpr wifi_mode_t WIFI_MODE = WIFI_MODE_AP;
constexpr wifi_interface_t WIFI_IF = (WIFI_MODE == WIFI_MODE_STA) ? WIFI_IF_STA : WIFI_IF_AP;
static uint8_t CHANNEL = 1;

// void dataReceived(uint8_t*, uint8_t*, uint8_t, signed int, bool);
void createDefaultJsonConfig();

class wifiHelper
{
private:
    //    ringBuffer<deviceDataPacket, 10>* rxbuffer;
    //    bool ownsBuffer = false;

public:
    wifiHelper();
    ~wifiHelper();

    void networkScan();
    int connectWifi(uint8_t, wifi_mode_t = WIFI_MODE_STA, uint32_t = TIMEOUT, String = "", String = "", bool = false);
    void printMAC(uint8_t *mac, bool verbose);
    bool begin(class configManager2*, bool = false);
};