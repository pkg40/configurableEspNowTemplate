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
#include <esp_wifi.h>
#if defined(ESP8266)
extern "C"
{
#include <user_interface.h> // give access to raw SDK API
}
#include <ESP8266WiFi.h>
#include <espnow.h>
#elif defined(ESP32)
#include <Update.h>
#include <WiFi.h>
#include <esp_now.h>
#else
#error "Only support  ESP32 and ESP8266"
#endif

#include <ringBuffer.hpp>
#include <configManager2.h>
#include <espNowCoPilot.hpp>
#include <deviceDataPacket.h>

// Sample data structure you'd send/receive
struct MyData
{
    int id;
    float value;
};

#include <esp_wifi.h>

extern configManager2 config;
extern espNowCoPilot<deviceDataPacket> myEspNow;

// example promiscuous callback function
//  This function will be called for every received packet in promiscuous mode
//  You can filter packets based on type, channel, etc.
void IRAM_ATTR onPromiscuousRx(void *buf, wifi_promiscuous_pkt_type_t type);

void IRAM_ATTR onPromiscuousRx2(void *buf, wifi_promiscuous_pkt_type_t type);

// Example receive callback (Rx)
void myRxCallback(const uint8_t *mac, const uint8_t *data, size_t len, void *cbarg);

// Example transmit callback (Tx)
void myTxCallback(const uint8_t *mac, const uint8_t status, void *cbarg);
