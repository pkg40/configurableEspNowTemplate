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

#if defined(ESP32)
  #include <WiFi.h>
  #include <esp_wifi.h>
#elif defined(ESP8266)
  extern "C" {
    #include <user_interface.h>
  }
  #include <ESP8266WiFi.h>
#endif

inline void setWiFiChannel(uint8_t channel) {
#if defined(ESP32)
  esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
#elif defined(ESP8266)
  wifi_set_channel(channel);
#endif
}

inline void getDeviceMac(uint8_t* macOut) {
#if defined(ESP32) || defined(ESP8266)
  WiFi.macAddress(macOut);
#endif
}

inline void transmitRawPacket(const uint8_t* data, size_t len) {
#if defined(ESP32)
  esp_wifi_80211_tx(WIFI_IF_STA, data, len, false);
#elif defined(ESP8266)
  // NOTE: Not supported — could use beacon simulation via WiFi.print if needed
  // For now, just ignore or log
#endif
}