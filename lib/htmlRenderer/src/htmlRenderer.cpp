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

#include "htmlRenderer.h"

htmlRenderer::htmlRenderer(const std::map<String, std::map<String, String>> *cfg) : config(cfg) {};

htmlRenderer::~htmlRenderer() {}

String htmlRenderer::generateHomePage() const
{
    String html = String("<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Home</title>") + embeddedStyle + "</head><body>";
    html += "<h1>Main Dashboard</h1>";
    html += "<ul>";
    html += "<h2>\t\tℹ️\t\t<a href='/info'>Device Info</a></h2>";
    html += "<hr>";
    html += "<h2>\t\t🛠\t\t<a href='/configs'>Configuration Form</a></h2>";
    html += "<h2>\t\t📃\t\t<a href='/all'>All Sections</a></h2>";
    html += "<hr>";
    html += "<h2>\t\t🗂\t\t<a href='/files'>View SPIFFS</a></h2>";
    html += "<h2>\t\t📥\t\t<a href='/downloadPage'>Download from SPIFFS</a></h2>";
    html += "<h2>\t\t📤\t\t<a href='/upload'>Upload to SPIFFS</a></h2>";
    html += "<hr>";
    html += "<h2>\t\t📤\t\t<a href='/firmware'>Update Firmware</a></h2>";
    html += "<hr>";
    html += "<h2>\t\t🔄\t\t<a href='/restart'>Restart Device</a></h2>";
    html += "</ul>";
    html += "</body></html>";
    return html;
}

String htmlRenderer::generateEspNowQrPage(const String& mac, const String& lmkHex) const {
    String payload = "MAC=" + mac + ";LMK=" + lmkHex;

    String html = String(R"rawlite(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <title>ESP-NOW QR Code</title>
  <style>
    body { font-family: sans-serif; background: #f6fff6; color: #2c3e50; text-align: center; padding: 2em; }
    #qrcode { margin: 2em auto; }
    .info { font-size: 0.9rem; color: #555; }
  </style>
</head>
<body>
  <h1>Scan to Pair</h1>
  <div id="qrcode"></div>
  <p class="info">MAC: )rawlite") + mac + R"rawlite(<br>LMK: )rawlite" + lmkHex + R"rawlite(</p>

  <script src="https://cdn.jsdelivr.net/npm/qrcodejs@1.0.0/qrcode.min.js"></script>
  <script>
    const payload = ")rawlite" + payload + R"rawlite(";
    new QRCode(document.getElementById("qrcode"), {
      text: payload,
      width: 256,
      height: 256,
      colorDark: "#000000",
      colorLight: "#ffffff",
      correctLevel: QRCode.CorrectLevel.H
    });
  </script>
</body>
</html>
)rawlite";

    return html;
}

void htmlRenderer::printSectionData(const String &sectionName, const std::map<String, String> &sectionData) const
{
    Serial.println("🔍 Dumping config for section: " + sectionName);

    if (sectionData.empty())
    {
        Serial.println("  (empty)");
        return;
    }

    for (const auto &kv : sectionData)
    {
        Serial.printf("  • %s = %s\n", kv.first.c_str(), kv.second.c_str());
    }
}

String htmlRenderer::generateConfigFormPage(const std::map<String, std::map<String, String>>& config, bool rowFormat) const {

    String html = String("<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Edit Config</title>") + embeddedStyle + "</head><body>";
    html += "<h1>🛠 Edit Configuration</h1><form method='POST' action='/submit-section'>";
//    html += "<table class='config-table'>\n";

//    const auto& configMap = config.getConfig();
    const auto& configMap = config;

    for (const auto& section : configMap) {
        const String& sectionName = section.first;
        const auto& sectionData = section.second;

        // Skip if this is a format definition section
        if (sectionName.endsWith(".format")) continue;

        // Look up format specifier
        String formatID = sectionData.count("format.use") ? sectionData.at("format.use") : "";
        const auto& formatMap = configMap.count(formatID)
                                ? configMap.at(formatID)
                                : std::map<String, String>();

        html += renderConfigSection(sectionName, sectionData, formatMap, rowFormat);
    }

    html += "</table>\n";
    html += "<input type='submit' value='Save Configuration'>\n";
    html += "</form>\n";
    html += "<div style='text-align:center; margin:1.5em 0;'>"
        "<a href=\"/espnow-manual.html\" target=\"_blank\" "
        "style=\"background:#003366; color:#fff; padding:0.6em 1.2em; border-radius:4px; "
        "text-decoration:none; font-weight:bold;\">"
        "📘 Open ESP-NOW Manual</a></div>";

    return html;
}

/*!SECTION
String htmlRenderer::generateConfigFormPage(const std::map<String, std::map<String, String>>* configMap, bool verbose) const {
    if (verbose) {
        Serial.println("🔧 Starting config form generation...");
    }

    String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Edit Config</title>" + embeddedStyle + "</head><body>";
    html += "<h1>🛠 Edit Configuration</h1><form method='POST' action='/submit-section'>";

    for (const auto& section : *configMap) {
        const String& sectionName = section.first;
        const auto& sectionData = section.second;

        Serial.println("📂 Checking section: " + sectionName);
        printSectionData(sectionName, sectionData);

        if (sectionName.endsWith(".format")) {
            Serial.println("⏭ Skipped format section");
            continue;
        }

        if (!sectionData.count("format.use")) {
            Serial.println("❌ Skipped: no format.use defined");
            continue;
        }

        html += renderConfigSection(sectionName, *configMap, sectionData, true, verbose);
    }

    html += "<input type='submit' value='Save'>";
    html += "</form><br><a href='/home'>Back to Home</a></body></html>";
    return html;
}
*/

String htmlRenderer::renderInputField(const String& fullKey,
                                      const String& value,
                                      const String& type,
                                      const std::map<String, String>& formatMap) const {
    String baseType = type;
    bool isReadOnly = false;
    bool isPassword = false;

    int dotPos = type.indexOf('.');
    if (dotPos != -1) {
        baseType = type.substring(0, dotPos);
        String suffix = type.substring(dotPos + 1);
        isReadOnly = (suffix == "readonly");
        isPassword = (suffix == "password");
    }

    // Extract base key (strip section.)
    String baseKey = fullKey;
    int dot = fullKey.indexOf('.');
    if (dot != -1) baseKey = fullKey.substring(dot + 1);

    // Tooltip support
    String tooltip = "";
    auto tooltipKey = baseKey + ".tooltip";
    if (formatMap.count(tooltipKey)) {
        tooltip = " title='" + formatMap.at(tooltipKey) + "'";
    }

    // Handle checkbox
    if (baseType == "checkbox") {
        bool checked = (value == "true" || value == "1");
        String html = "<input type='checkbox' name='" + fullKey + "'" + (checked ? " checked" : "");
        if (isReadOnly) html += " disabled";
        html += tooltip + ">";
        if (isReadOnly) html += "<input type='hidden' name='" + fullKey + "' value='" + value + "'>";
        return html;
    }

    // Text or password input
    String html = "<input type='";
    html += isPassword ? "password" : "text";
    html += "' name='" + fullKey + "' value='" + value + "'";
    if (isReadOnly) html += " readonly style='background:#f5f5f5; color:#555'";
    html += tooltip + ">";
    return html;
}

String htmlRenderer::renderConfigSubmitSummaryPage(const std::map<String, String>& updatedFields, bool verbose) const {
    Serial.printf("📬 Rendering submission summary (%zu fields updated)\n", updatedFields.size());

    String html = String("<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Config Updated</title>") + embeddedStyle + "</head><body>";
    html += "<h1>✅ Configuration Updated</h1>";
    html += "<table class='config-table'><tr><th>Field</th><th>New Value</th></tr>";

    for (const auto& kv : updatedFields) {
        html += "<tr><td>" + kv.first + "</td><td>" + kv.second + "</td></tr>";
    }

    html += "</table><br><a href='/configs'>Edit More</a> | <a href='/home'>Home</a></body></html>";
    return html;
}

/*!SECTION
String htmlRenderer::generateConfigFormPage(const std::map<String, std::map<String, String>> *config) const
{
    Serial.println("🔧 Starting config form generation...");
    String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Edit Config</title>" + embeddedStyle + "</head><body>";
    html += "<h1>🛠 Edit Configuration</h1><form method='POST' action='/submit-section'>";

    for (const auto &section : *config)
    {
        for (const auto &section : *config)
        {
            printSectionData("config", section.second);
            const String &sectionName = section.first;
            const auto &sectionData = section.second;

            Serial.println("📂 Checking section: " + sectionName);

            if (sectionName.endsWith(".format"))
            {
                Serial.println(sectionName);
                Serial.println("⏭ Skipped format section");
                continue;
            }

            if (!sectionData.count("_use"))
            {
                Serial.println("sectionData.count:_use = " + String(sectionData.count("_use")));
                Serial.println("❌ Skipped: no _use defined");
                continue;
            }

            const String formatName = sectionData.at("_use");
            Serial.println("🔗 Using format: " + formatName);

            if (!config->count(formatName))
            {
                Serial.println("🚫 Format missing: " + formatName);
                continue;
            }

            const auto &formatSpec = config->at(formatName);
            Serial.printf("✅ Rendering section '%s' with %zu fields\n", sectionName.c_str(), formatSpec.size());

            html += renderConfigSection(sectionName, sectionData, formatSpec);
        }

        html += "<input type='submit' value='Save'>";
        html += "</form><br><a href='/home'>Back to Home</a></body></html>";
        return html;
    }
}
*/

String htmlRenderer::renderConfigSection(const String& sectionName,
                                         const std::map<String, String>& fields,
                                         const std::map<String, String>& formatMap,
                                         bool rowFormat, bool verbose) const {
    String html;

    if (rowFormat) {
        html += "<tr><th>" + sectionName + "</th><td>";

        for (const auto& kv : fields) {
            const String& key = kv.first;
            const String& value = kv.second;
            if (key.startsWith("format.")) continue;

            String type = formatMap.count(key) ? formatMap.at(key) : "string";
            String fullKey = sectionName + "." + key;

            html += "<label style='margin-right:1.5em'>" + key + ": ";
            html += renderInputField(fullKey, value, type, formatMap);
            html += "</label>";
        }

        html += "</td></tr>\n";
    } else {
        html += "<tr><th colspan='2'>" + sectionName + "</th></tr>\n";

        for (const auto& kv : fields) {
            const String& key = kv.first;
            const String& value = kv.second;
            if (key.startsWith("format.")) continue;

            String type = formatMap.count(key) ? formatMap.at(key) : "string";
            String fullKey = sectionName + "." + key;

            html += "<tr><td>" + key + "</td><td>";
            html += renderInputField(fullKey, value, type, formatMap);
            html += "</td></tr>\n";
        }
    }

    return html;
}

/*!SECTION
String htmlRenderer::renderConfigSection(const String& sectionName,
                                         const std::map<String, std::map<String, String>>& configMap, bool verbose) const {
    if (!configMap.count(sectionName)) {
        if (verbose) Serial.println("❌ Section '" + sectionName + "' not found");
        return "<p>Section '" + sectionName + "' not found.</p>";
    }

    const auto& sectionData = configMap.at(sectionName);

    // Require format.use key
    if (!sectionData.count("format.use")) {
        if (verbose) Serial.println("⚠️ Skipping section '" + sectionName + "' (no format.use)");
        return "<p><strong>" + sectionName + "</strong> has no <code>format.use</code> field.</p>";
    }

    const String formatName = sectionData.at("format.use");
    if (!configMap.count(formatName)) {
        if (verbose) Serial.println("⚠️ Format section missing: " + formatName);
        return "<p><strong>" + sectionName + "</strong> references missing format: <code>" + formatName + "</code></p>";
    }

    const auto& fieldMap = configMap.at(formatName);
    String html = "<h2>" + sectionName + "</h2><table class='config-table'>";
    html += "<tr><th>Key</th><th>Value</th></tr>";

    for (const auto& field : fieldMap) {
        const String& key = field.first;
        const String& type = field.second;

        const String fullKey = sectionName + "." + key;
        const String value = sectionData.count(key) ? sectionData.at(key) : "";

        html += "<tr><td>" + key + "</td><td>" + renderInputField(fullKey, value, type) + "</td></tr>";
    }

    html += "</table><br>";
    return html;
}

String htmlRenderer::renderConfigSection(const String& sectionName,
                                         const std::map<String, String>& fields,
                                         const std::map<String, String>& formatMap,) const {
    String html;
    bool rowFormat=true;

    if (rowFormat) {
        html += "<table class='config-table'>\n";
    html += "<tr><th colspan='2'>" + sectionName + "</th></tr>\n";

    for (const auto& kv : fields) {
        const String& key = kv.first;
        const String& value = kv.second;
        const String fullKey = sectionName + "." + key;

        // Skip metadata fields
        if (key.startsWith("format.")) continue;

        String inputType = "string";
        if (formatMap.count(key)) inputType = formatMap.at(key);

        html += "<tr><td>" + key + "</td><td>" + renderInputField(fullKey, value, inputType, formatMap) + "</td></tr>\n";
    }
    } else {
        html += "<table class='config-table-compact'>\n";
    }

    return html;
}
*/

String htmlRenderer::generateSPIFFSFileListPage() const
{
    String html = String("<!DOCTYPE html><html><head><meta charset='UTF-8'><title>SPIFFS Files</title>") + embeddedStyle + "</head><body>";
    html += "<h1>🗂 SPIFFS File Browser</h1>";
    html += "<table class='config-table'><th>Filename</th><th>Size (bytes)</th><th>Actions</th></tr>";

    File root = SPIFFS.open("/");
    File file = root.openNextFile();

    while (file)
    {
        String name = file.name();
        size_t size = file.size();

        html += "<tr><td>" + name + "</td><td>" + String(size) + "</td>";
        html += "<td><a href='/files/view?name=" + name + "'>View</a></td></tr>";

        file = root.openNextFile();
    }

    html += "</table><br><a href='/home'>Back to Home</a></body></html>";
    return html;
}
String htmlRenderer::generateFileContentPage(const String& filename, const String& content, bool fileFound) const {
    Serial.printf("🧾 Rendering file viewer for: %s (found: %s)\n", filename.c_str(), fileFound ? "yes" : "no");

    String html = String("<!DOCTYPE html><html><head><meta charset='UTF-8'><title>View File</title>") + embeddedStyle + "</head><body>";
    html += "<h1>📂 Viewing: " + filename + "</h1>";

    if (fileFound) {
        html += "<pre style='border:1px solid #ccc; padding:1em; background:#fdfdfd'>" + content + "</pre>";
    } else {
        html += "<p style='color:red;'>❌ File not found: <code>" + filename + "</code></p>";
    }

    html += "<br><a href='/files'>Back to Files</a> | <a href='/home'>Home</a></body></html>";
    return html;
}

/*!SECTION
String htmlRenderer::generateFileContentPage(const String &filename, const String &content, bool fileFound) const
{
    String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>View File</title>" + embeddedStyle + "</head><body>";
    html += "<h1>📄 Viewing: " + filename + "</h1>";

    if (fileFound)
    {
        html += "<pre>" + content + "</pre>";
    }
    else
    {
        html += "<p style='color:red;'>⚠️ File not found: " + filename + "</p>";
    }

    html += "<br><a href='/files'>Back to Files</a> | <a href='/home'>Home</a></body></html>";
    return html;
}
*/

String htmlRenderer::generateInfoPage() const
{
    String html = String("<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Device Info</title>") + embeddedStyle + "</head><body>";
    html += "<h1>📟 Device Information</h1>";
    html += "<table class='config-table'>";
    html += "<tr><th>Field</th><th>Value</th></tr>";

    html += "<tr><td>Chip Model</td><td>" + String(ESP.getChipModel()) + "</td></tr>";
    html += "<tr><td>Chip Revision</td><td>" + String(ESP.getChipRevision()) + "</td></tr>";
    html += "<tr><td>CPU Frequency</td><td>" + String(ESP.getCpuFreqMHz()) + " MHz</td></tr>";
    html += "<tr><td>Flash Size</td><td>" + String(ESP.getFlashChipSize() / (1024 * 1024)) + " MB</td></tr>";
    html += "<tr><td>Sketch Size</td><td>" + String(ESP.getSketchSize()) + " bytes</td></tr>";
    html += "<tr><td>Free Heap</td><td>" + String(ESP.getFreeHeap()) + " bytes</td></tr>";
    html += "<tr><td>MAC Address</td><td>" + WiFi.macAddress() + "</td></tr>";
    html += "<tr><td>Local IP</td><td>" + WiFi.localIP().toString() + "</td></tr>";
    html += "<tr><td>Channel</td><td>" + String(WiFi.channel()) + "</td></tr>";
    html += "<tr><td>SSID</td><td>" + WiFi.SSID() + "</td></tr>";
    html += "<tr><td>Signal Strength</td><td>" + String(WiFi.RSSI()) + " dBm</td></tr>";

    html += "</table>";
    html += "<br><a href='/home'>Back to Home</a></body></html>";
    return html;
}

String htmlRenderer::generateAllSectionsPage() const
{
    if (!config)
        return "<html><body><h1>No Config Loaded</h1></body></html>";

    String html = String("<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Edit All</title>") + embeddedStyle + "</head><body>";
    html += "<h1>Edit All Configuration</h1>";
    html += "<form method='POST' action='/submit-section'>";

    for (const auto &section : *config)
    {
        html += "<h2>[" + section.first + "]</h2>";
        html += "<input type='hidden' name='section' value='" + section.first + "'>";
        html += "<table class == 'config-table'><tr><th>Key</th><th>Value</th></tr>";

        for (const auto &kv : section.second)
        {
            html += "<tr><td>" + kv.first + "</td><td>";
            html += "<input type='text' name='" + kv.first + "' value='" + kv.second + "'></td></tr>";
        }

        html += "</table><br>";
    }

    html += "<input type='submit' value='Save All Changes'>";
    html += "</form><br><a href='/home'>Back to Home</a>";
    return html;
}

String htmlRenderer::generateUploadPage() const
{
    String html = String("<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Upload to SPIFFS</title>") + embeddedStyle + "</head><body>";
    html += "<h1>Upload File to SPIFFS</h1>";
    html += "<form method='POST' action='/upload' enctype='multipart/form-data'>";
    html += "<input type='file' name='upload'><br><br><input type='submit' value='Upload'>";
    html += "</form><br><a href='/home'>Back to Home</a></body></html>";
    return html;
}

String htmlRenderer::generateFirmwareUpdatePage() const
{
    String html = String("<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Firmware Update</title>") + embeddedStyle + "</head><body>";
    html += "<h1>🧪 Firmware Update</h1>";

    html += "<p><strong>Current Firmware Build:</strong><br>";
    html += "Compiled on <code>" + String(__DATE__) + " at " + String(__TIME__) + "</code></p>";

    html += "<form method='POST' action='/update' enctype='multipart/form-data'>";
    html += "<input type='file' name='firmware'><br><br>";
    html += "<input type='submit' value='Upload Firmware'>";
    html += "</form>";

    html += "<br><a href='/home'>Back to Home</a></body></html>";
    return html;
}

String htmlRenderer::generateDownloadPage() const
{
    String html = String("<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Download SPIFFS File</title>") + embeddedStyle + "</head><body>";
    html += "<h1>📥 Download a File from SPIFFS</h1>";

    html += "<form method='GET' action='/download'>";
    html += "<label for='filename'>Select file:</label> ";
    html += "<select name='filename'>";

    File root = SPIFFS.open("/");
    File file = root.openNextFile();

    while (file)
    {
        String name = file.name();
        html += "<option value='" + name + "'>" + name + "</option>";
        file = root.openNextFile();
    }

    html += "</select> <input type='submit' value='Download'>";
    html += "</form><br><a href='/home'>Back to Home</a></body></html>";
    return html;
}

String htmlRenderer::generateRestartPage() const
{
    return String("<html><head><meta charset='UTF-8'><title>Restart</title>") + embeddedStyle +
           "</head><body><h1>Device Restarting...</h1>"
           "<p>Please wait a few seconds before reconnecting.</p></body></html>";
}

String htmlRenderer::generateSectionEditForm(const String& sectionName,
                                             const std::map<String, String>& section,
                                             const std::map<String, std::map<String, String>>& configMap) const {
    String html;

    html += "<form method='post' action='/submit-section'>\n";
    html += "<table class='config-table'>\n";
    html += "<tr><th colspan='2'>" + sectionName + "</th></tr>\n";

    // Fetch format map if available
    std::map<String, String> formatMap;
    if (section.count("format.use")) {
        const String& formatID = section.at("format.use");
        if (configMap.count(formatID)) {
            formatMap = configMap.at(formatID);
        }
    }

    for (const auto& kv : section) {
        const String& key = kv.first;
        const String& value = kv.second;

        if (key.startsWith("format.")) continue;

        String inputType = formatMap.count(key) ? formatMap.at(key) : "string";
        String fullKey = sectionName + "." + key;

        html += "<tr><td>" + key + "</td><td>";
        html += renderInputField(fullKey, value, inputType, formatMap);
        html += "</td></tr>\n";
    }

    html += "</table>\n";
    html += "<input type='submit' value='Save Section'>\n";
    html += "</form>\n";

    return html;
}

String htmlRenderer::generateConfigErrorPage(const String& errorMessage) const {
    Serial.println("🚨 Rendering configuration error page");

    String html = String("<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Config Error</title>") + embeddedStyle + "</head><body>";
    html += "<h1 style='color:red;'>Configuration Error</h1>";
    html += "<p>" + errorMessage + "</p>";
    html += "<br><a href='/configs'>Back to Config Editor</a> | <a href='/home'>Home</a></body></html>";
    return html;
}

String htmlRenderer::generateConfigSubmitSummaryPage(const std::map<String, String>& updatedFields) const {
    Serial.printf("📬 Rendering submission summary (%zu fields updated)\n", updatedFields.size());

    String html = String("<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Config Updated</title>") + embeddedStyle + "</head><body>";
    html += "<h1>✅ Configuration Updated</h1>";
    html += "<table class='config-table'><tr><th>Field</th><th>New Value</th></tr>";

    for (const auto& kv : updatedFields) {
        html += "<tr><td>" + kv.first + "</td><td>" + kv.second + "</td></tr>";
    }

    html += "</table><br><a href='/configs'>Edit More</a> | <a href='/home'>Home</a></body></html>";
    return html;
}

String htmlRenderer::getInputTypeForField(const String& key, const std::map<String, String>& formatMap) const {
    auto it = formatMap.find(key + ".format");
    if (it != formatMap.end()) {
        const String& fmt = it->second;
        if (fmt == "password") return "password";
        if (fmt == "integer")  return "number";
        if (fmt == "checkbox") return "checkbox";
        if (fmt == "macaddress") return "text";     // optional custom validator
        if (fmt == "ipaddress")  return "text";     // same
    }

    if (key.endsWith("password") || key == "password") return "password";
    if (key.startsWith("use") || key.endsWith("active")) return "checkbox";
    return "text";
}

/*!SECTION
const String htmlRenderer::embeddedStyle =
    "<style>"
    "body { font-family: sans-serif; margin: 20px; background-color: #f6fff6; color: #2c3e50; }"
    "h1, h2 { color: #3c763d; }"
    "table { border-collapse: collapse; width: 100%; background-color: #ffffff; }"
    "th, td { border: 1px solid #b2d8b2; padding: 8px; }"
    "th { background-color: #dff0d8; text-align: left; }"
    "td:first-child { text-align: right; font-weight: bold; white-space: nowrap; width: 20%; }"
    "td:last-child { text-align: left; }"
    "input[type='text'], input[type='password'] { width: 100%; box-sizing: border-box; background-color: #fafff7; border: 1px solid #c2e0c6; padding: 6px; font-size: 1rem; }"
    "input[type='checkbox'] { transform: scale(1.1); margin-right: 0.5em; }"
    "input[readonly] { background: #f3f3f3; color: #666; border: 1px solid #ccc; }"
    "input[type='submit'], input[type='button'] { background-color: #4CAF50; color: white; border: none; padding: 10px 20px; cursor: pointer; }"
    "input[type='submit']:hover, input[type='button']:hover { background-color: #45a049; }"
    ".readonly-field { display: inline-block; padding: 6px 8px; background: #f3f3f3; color: #444; font-family: monospace; }"
    "table.config-table { width: 100%; border-collapse: collapse; table-layout: auto; margin-bottom: 1.5em; }"
    "table.config-table th, table.config-table td { border: 1px solid #b2d8b2; padding: 8px; }"
    "table.config-table input[type='text'], table.config-table input[type='password'] { width: 100%; }"
    "table.config-table label { margin-right: 1.5em; display: inline-block; }"
    "pre { background: #f0f0f0; padding: 1em; overflow-x: auto; }"
    "a { color: #3c763d; }"
    "</style>";
*/