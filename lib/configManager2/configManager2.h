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
#include <map>
#include <vector>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <WString.h> // Ensure String class is available

class configManager2
{
private:
    std::map<String, std::map<String, String>> _config; // section → field → value

public:
    configManager2();
    ~configManager2();

    bool begin( String filename, bool = true);
    bool loadConfigString(const char *filename, String *jsonString, bool verbose = true);
    std::map<String, std::map<String, String>> jsonStringToMap(const String &jsonString, bool = false);
    bool jsonStringToConfig(const String &jsonString, bool = false);
    String mapToJsonString(const std::map<String, std::map<String, String>> &configMap);
    bool saveToJson(const String &path, const std::map<String, std::map<String, String>> &configMap);
    bool saveConfigFile(const char *filename);
    String loadDefaults();

    String getValue(const String &section, const String &key) const;
    void setValue(const String &section, const String &key, const String &value);

    const std::map<String, std::map<String, String>> &getConfig() const;

    // Optional auth helpers
    String getUser() const;
    String getPassword() const;

    void printConfigToSerial() const;
    std::map<String, String> &getSection(const String &sectionName);
    bool parseHexStringToBytes(const String &hexInput, uint8_t *out, size_t outLen) ;
};