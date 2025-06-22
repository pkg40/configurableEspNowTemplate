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
#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
class htmlRenderer
{
private:
    const std::map<String, std::map<String, String>> *config;
    const std::map<String, std::map<String, String>> *templates;
    const std::map<String, std::vector<String>> *formatGroups;

    const char *embeddedStyle =
        R"rawlite(
<style>
  body {
    font-family: sans-serif;
    margin: 20px;
    background-color: #f6fff6;
    color: #2c3e50;
  }

  h1, h2 {
    color: #3c763d;
  }

  table {
    border-collapse: collapse;
    width: 100%;
    background-color: #ffffff;
  }

  th, td {
    border: 1px solid #b2d8b2;
    padding: 8px;
  }

  th {
    background-color: #dff0d8;
    text-align: left;
  }

  td:first-child {
    text-align: right;
    font-weight: bold;
    white-space: nowrap;
    width: 20%;
  }

  td:last-child {
    text-align: left;
  }

  input[type='text'],
  input[type='password'] {
    width: 100%;
    box-sizing: border-box;
    background-color: #fafff7;
    border: 1px solid #c2e0c6;
    padding: 6px;
    font-size: 1rem;
  }

  input[type='checkbox'] {
    transform: scale(1.1);
    margin-right: 0.5em;
  }

  input[readonly] {
    background: #f3f3f3;
    color: #666;
    border: 1px solid #ccc;
  }

  input[type='submit'],
  input[type='button'] {
    background-color: #4CAF50;
    color: white;
    border: none;
    padding: 10px 20px;
    cursor: pointer;
  }

  input[type='submit']:hover,
  input[type='button']:hover {
    background-color: #45a049;
  }

  .readonly-field {
    display: inline-block;
    padding: 6px 8px;
    background: #f3f3f3;
    color: #444;
    font-family: monospace;
  }

  table.config-table {
    width: 100%;
    border-collapse: collapse;
    table-layout: auto;
    margin-bottom: 1.5em;
  }

  table.config-table th,
  table.config-table td {
    border: 1px solid #b2d8b2;
    padding: 8px;
  }

  table.config-table input[type='text'],
  table.config-table input[type='password'] {
    width: 100%;
  }

  table.config-table label {
    margin-right: 1.5em;
    display: inline-block;
  }

  pre {
    background: #f0f0f0;
    padding: 1em;
    overflow-x: auto;
  }

  a {
    color: #3c763d;
  }
</style>
)rawlite";

public:
    htmlRenderer(const std::map<String, std::map<String, String>> *cfg);

    ~htmlRenderer();

    const char *getStyle() const
    {
        return embeddedStyle;
    }
    String generateHomePage() const;
    String generateEspNowQrPage(const String &mac, const String &lmkHex) const;
    String generateSPIFFSFileListPage() const;
    String generateFileContentPage(const String &filename, const String &content, bool fileFound) const;
    String generateInfoPage() const;
    String generateAllSectionsPage() const;
    String generateConfigFormPage(const std::map<String, std::map<String, String>> &config, bool = false) const;
    String renderInputField(const String &fullKey,
                            const String &value,
                            const String &type,
                            const std::map<String, String> &formatMap) const;
    String renderConfigSubmitSummaryPage(const std::map<String, String> &updatedFields, bool = true) const;
    String generateUploadPage() const;
    String generateFirmwareUpdatePage() const;
    String generateDownloadPage() const;
    String generateRestartPage() const;
    String generateSectionEditForm(const String &sectionName,
                                   const std::map<String, String> &sectionData,
                                   const std::map<String, std::map<String, String>> &allConfig) const;
    String generateConfigErrorPage(const String &errorMessage) const;
    String generateConfigSubmitSummaryPage(const std::map<String, String> &updatedFields) const;
    String renderConfigSection(const String &sectionName,
                               const std::map<String, String> &fields,
                               const std::map<String, String> &formatMap,
                               bool rowFormat = true, bool verbose = false) const;
    void printSectionData(const String &sectionName, const std::map<String, String> &sectionData) const;
    String getInputTypeForField(const String& key, const std::map<String, String>& formatMap) const;

    //    static const String embeddedStyle;
};