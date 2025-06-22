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
#include <Arduino.h>
#if defined ESP32
#include <WiFi.h> // or <ESP8266WiFi.h> if on ESP8266
#elif defined ESP8266
#include <WiFi.h>
#endif

#include <deviceDataPacket.h>
#include <configManager2.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <webUI.h>
#include <ringBuffer.hpp>
#include <wifiHelper.hpp>
// #include "espnowCallBacks.h"

#include <espNowCallbacks.hpp>
#include <espNowCoPilot.hpp>

const String filename = "/config.json";
String jsonString;

configManager2 config;
AsyncWebServer server(80); // ✅ Web server instance
webUI ui(&config);

ringBuffer<deviceDataPacket, 10> rxRing;
wifiHelper myWiFi; //
espNowHelper<deviceDataPacket> myEspNow(myRxCallback, myTxCallback, &rxRing);

deviceDataPacket incomingData; // ✅ Buffer for received data


void setup()
{
    Serial.begin(115200);

    unsigned long startMillis = millis(); // ✅ Non-blocking delay alternative
    while (millis() - startMillis < 10000)
    {
        delay(100); // Small increments instead of full 10s block
    }

//    Serial.println("🔧 Loading configuration...");

    if (config.loadConfigString(filename.c_str(), &jsonString, false))
    {
        Serial.println("✅ Config file loaded.");
        Serial.println(jsonString);

        if (config.jsonStringToConfig(jsonString))
        {
            config.printConfigToSerial();
        }
        else
        {
            Serial.println("⚠️ Failed to parse JSON. Loading defaults...");
            config.loadDefaults();
            config.printConfigToSerial();
        }

        // ✅ Connect WiFi and start the web server
        //        myEspNow.printMAC()
        int channel;
        channel = myWiFi.connectWifi(config.getValue("wifiSTA", "channel").toInt(), WIFI_MODE_STA, 5000,
                                     config.getValue("wifiSTA", "ssid"), config.getValue("wifiSTA", "password"), true);
        if (channel < 0)
        {
            Serial.println("❌ Failed to connect to WiFi. Check your credentials.");
            channel = myWiFi.connectWifi(config.getValue("wifiAP", "channel").toInt(), WIFI_MODE_AP, 5000,
                                         config.getValue("wifiAP", "ssid"), config.getValue("wifiAP", "password"), true);
            config.setValue("wifiSTA", "active", String("false"));
            if (channel < 0)
            {
            config.setValue("wifiSTA", "active", String("false"));
            config.setValue("wifiAP", "active", String("false"));
                Serial.println("❌ Failed to connect to AP mode. Check your credentials.");
                return; // Exit if both connections fail
            }
            else
            {
                Serial.printf("✅ Connected to AP on channel %d\n", channel);
                Serial.println("SSID: " + WiFi.SSID());
                Serial.println("BSSID: " + WiFi.BSSIDstr());
                Serial.println("RSSI: " + String(WiFi.RSSI()));
                config.setValue("wifiSTA", "channel", String(channel));
            config.setValue("wifiSTA", "active", String("false"));
            config.setValue("wifiAP", "active", String("true"));
                ui.begin();
            }
        }
        else
        {
            Serial.printf("✅ Connected to WiFi on channel %d\n", channel);
            Serial.println("SSID: " + WiFi.SSID());
            Serial.println("BSSID: " + WiFi.BSSIDstr());
            Serial.println("RSSI: " + String(WiFi.RSSI()));
            config.setValue("wifiSTA", "channel", String(channel));
            config.setValue("wifiSTA", "active", String("true"));
            config.setValue("wifiAP", "active", String("false"));
            ui.begin();
        }

        // ✅ Start ESP-NOW Safely
        esp_wifi_set_promiscuous(true);
        myEspNow.connectEspnow(channel, WIFI_MODE_STA, true);
        config.saveToJson(filename.c_str(), config.getConfig());
    }
    else
    {
        Serial.println("❌ Failed to initialize SPIFFS or load configuration.");
        return;
    }
    esp_log_level_set("wifi", ESP_LOG_WARN);
}

void loop()
{
    static unsigned long startMillis = millis(); // Non-blocking delay alternative
    static deviceDataPacket dataPacket = {0, 0, 0, 0, 0}; // Initialize data packet

    while (millis() - startMillis < 10000)
    {
        delay(100); // Small increments instead of full 10s block
    }
    startMillis = millis(); // Reset for next loop
    dataPacket.id = startMillis%256; // Reset ID for new packet
    dataPacket.command = (dataPacket.command+3)%256;
    dataPacket.value1 = (dataPacket.value1+5)%256;
    dataPacket.value2 = (dataPacket.value2-5)%256;
    dataPacket.value3 = (dataPacket.value3+1)%256;

    myEspNow.sendEspNow(broadcastMAC, dataPacket);
    Serial.printf("📤 Sent data packet: ID=%d, Command=%d, Value1=%d, Value2=%d, Value3=%d\n",
                  dataPacket.id, dataPacket.command, dataPacket.value1, dataPacket.value2, dataPacket.value3);
}