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

#include <wifiHelper.hpp>
#include <configManager2.h>

wifiHelper::wifiHelper() {};
wifiHelper::~wifiHelper() {};

void wifiHelper::networkScan()
{
    Serial.print("scanning ...., ");
    int n = WiFi.scanNetworks();
    Serial.println("....... scan complete, ");
    Serial.print(n);
    if (n == 0)
    {
        Serial.println("no network found");
    }
    else
    {
        Serial.print("  :  ");
        Serial.println("networks:");
        for (int i = 0; i < n; i++)
        {
            Serial.print(i + 1);
            Serial.print(": ");
            Serial.print(WiFi.SSID(i));
            Serial.print(" (");
            Serial.print(WiFi.RSSI(i));
            Serial.print(")");
            Serial.println(WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? " " : "");
        }
        Serial.println("");
    }
    delay(1000); // block scans for 1 second
}

int wifiHelper::connectWifi(uint8_t wifi_channel, wifi_mode_t wifi_mode, uint32_t breakTimeout, String ssid, String pass, bool verbose)
{
    if (verbose)
        Serial.println("🚀 Connecting to WiFi...SSID " + ssid + ", Password: " + pass + ", Mode: " + (wifi_mode == WIFI_MODE_STA ? "STA" : "AP") + ", Channel: " + String(wifi_channel));

    if (wifi_mode == WIFI_MODE_STA)
    {
        WiFi.persistent(false);
        WiFi.mode(WIFI_MODE_STA);

        if (ssid.isEmpty() || pass.isEmpty())
        {
            Serial.println("❌ Error: SSID and password cannot be empty!");
            return -2;
        }
        Serial.println("Connecting to SSID: " + ssid);

        WiFi.begin(ssid.c_str(), pass.c_str());
    }
    else
    {
        WiFi.mode(WIFI_MODE_AP);
        WiFi.softAP(ssid.c_str(), pass.c_str(), wifi_channel);
    }

    uint32_t timeout = 0;
    while (WiFi.status() != WL_CONNECTED && timeout++ < breakTimeout)
    {
        delay(500);
        if (verbose)
            Serial.printf("... %d seconds - status %d\n", timeout, WiFi.status());
    }

    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("❌ Timeout: Could not connect to WiFi.");
        return -1;
    }

    if (verbose)
    {
        Serial.println("✅ Connected to WiFi");
        Serial.printf("IP Address: %s\n", WiFi.localIP().toString().c_str());
    }

    return WiFi.channel();
};

bool wifiHelper::begin(configManager2* config, bool verbose)
{
    const bool staEnabled = config->getValue("wifiSTA", "use") == "true";
    const bool apEnabled  = config->getValue("wifiAP", "use") == "true";

    if (staEnabled) {
        String ssid     = config->getValue("wifiSTA", "ssid");
        String password = config->getValue("wifiSTA", "password");

        if (ssid.isEmpty() || password.isEmpty()) {
            if (verbose) Serial.println("❌ STA config missing SSID or password. Skipping STA mode.");
        } else {
            if (verbose) Serial.printf("🔌 Connecting to SSID: %s\n", ssid.c_str());

            WiFi.mode(WIFI_STA);
            WiFi.begin(ssid.c_str(), password.c_str());

            unsigned long startTime = millis();
            while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000) {
                delay(250);
                if (verbose) Serial.print(".");
            }
            Serial.println();

            if (WiFi.status() == WL_CONNECTED) {
                int channel = WiFi.channel();
                if (verbose) {
                    Serial.printf("✅ Connected to Wi-Fi (channel %d)\n", channel);
                    Serial.println("SSID  : " + WiFi.SSID());
                    Serial.println("BSSID : " + WiFi.BSSIDstr());
                    Serial.println("RSSI  : " + String(WiFi.RSSI()));
                }

                config->setValue("wifiSTA", "channel", String(channel));
                config->setValue("wifiSTA", "active", "true");
                config->setValue("wifiAP", "active", "false");
                return true;
            }

            if (verbose) Serial.println("⏱️ STA connection failed. Will attempt AP fallback.");
        }
    }

    if (apEnabled) {
        String ssid     = config->getValue("wifiAP", "ssid");
        String password = config->getValue("wifiAP", "password");
        int channel     = config->getValue("wifiAP", "channel").toInt();

        WiFi.mode(WIFI_AP);
        bool result = WiFi.softAP(ssid.c_str(), password.c_str(), channel);

        if (result) {
            IPAddress apIP = WiFi.softAPIP();
            if (verbose) {
                Serial.printf("📶 Started AP \"%s\" on channel %d\n", ssid.c_str(), channel);
                Serial.print("IP Address: ");
                Serial.println(apIP.toString());
            }
            config->setValue("wifiAP", "active", "true");
            config->setValue("wifiSTA", "active", "false");
            return true;
        } else {
            Serial.println("❌ Failed to start Access Point");
            return false;
        }
    }

    Serial.println("❌ No network mode is enabled. Set either wifiSTA.use or wifiAP.use to true.");
    return false;
}