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

#include "webUI.h"

webUI::webUI(configManager2 *cfg)
    : server(80), configManager(cfg)
{
    renderer = new htmlRenderer(
        &configManager->getConfig());
}

webUI::~webUI()
{
    delete renderer;
}

configManager2 *webUI::getConfigManager()
{
    return configManager;
}

void webUI::begin(bool verbose)
{
    if (verbose)
        Serial.println("🌐 Starting Web UI on port 80...");

    server.on("/home", HTTP_GET, [this](AsyncWebServerRequest *request)
              { request->send(200, "text/html", renderer->generateHomePage()); });

    server.on("/files/view", HTTP_GET, [this](AsyncWebServerRequest *request)
              {
    Serial.println("🛠️ /files/view handler triggered");

    if (!request->hasParam("name")) {
        Serial.println("⚠️ Missing 'name' parameter on /files/view");
        request->send(400, "text/plain", "Missing 'name' parameter");
        return;
    }

    String filename = request->getParam("name")->value();
    filename = "/"+filename; // Ensure leading slash for SPIFFS
    filename = filename.substring(0, 32); // Limit filename length
    filename.trim(); 
    if (filename.length() > 32) {
        Serial.println("⚠️ Filename too long: " + filename);
        request->send(400, "text/plain", "Filename too long");
        return;
    }
    Serial.println("📂 File view requested: " + filename);

    bool found = SPIFFS.exists(filename);
    String content = "";

    if (found) {
        File file = SPIFFS.open(filename, "r");
        if (file) {
            content = file.readString();
            file.close();
            Serial.printf("📄 Opened %s (%u bytes)\n", filename.c_str(), content.length());
        } else {
            Serial.println("❌ Failed to open file after exists() check passed");
        }
    } else {
        Serial.println("❌ File not found in SPIFFS: " + filename);
    }

    request->send(200, "text/html", renderer->generateFileContentPage(filename, content, found)); });

    server.on("/files", HTTP_GET, [this](AsyncWebServerRequest *request)
              { request->send(200, "text/html", renderer->generateSPIFFSFileListPage()); });

    server.on("/files/view", HTTP_GET, [this](AsyncWebServerRequest *request)
              {
    if (!request->hasParam("name")) {
        Serial.println("⚠️ Missing 'name' parameter on /files/view");
        request->send(400, "text/plain", "Missing 'name' parameter");
        return;
    }

    String filename = request->getParam("name")->value();
    Serial.println("📂 File view requested: " + filename);

    bool found = SPIFFS.exists("/" + filename);
    String content = found ? SPIFFS.open(filename, "r").readString() : "";

    if (!found) {
        Serial.println("❌ File not found: " + filename);
    } else {
        Serial.printf("📄 Opened %s (%u bytes)\n", filename.c_str(), content.length());
    }

    request->send(200, "text/html", renderer->generateFileContentPage(filename, content, found)); });

    server.on("/info", HTTP_GET, [this](AsyncWebServerRequest *request)
              { request->send(200, "text/html", renderer->generateInfoPage()); });

    server.on("/all", HTTP_GET, [this](AsyncWebServerRequest *request)
              { request->send(200, "text/html", renderer->generateAllSectionsPage()); });

    server.on("/configs", HTTP_GET, [this](AsyncWebServerRequest *request)
              {
                const std::map<String, std::map<String, String>> config = configManager->getConfig();  // or however you load it
                request->send(200, "text/html", renderer->generateConfigFormPage(config, true)); });

    server.on("/upload", HTTP_GET, [this](AsyncWebServerRequest *request)
              { request->send(200, "text/html", renderer->generateUploadPage()); });

    server.on("/firmware", HTTP_GET, [this](AsyncWebServerRequest *request)
              { request->send(200, "text/html", renderer->generateFirmwareUpdatePage()); });

    server.on("/downloadPage", HTTP_GET, [this](AsyncWebServerRequest *request)
              { request->send(200, "text/html", renderer->generateDownloadPage()); });

    server.on("/download", HTTP_GET, [](AsyncWebServerRequest *request)
              {
    if (!request->hasParam("filename")) {
        request->send(400, "text/plain", "Missing 'filename' parameter");
        return;
    }

    String filename = "/"+request->getParam("filename")->value();
    if (!SPIFFS.exists(filename)) {
        request->send(404, "text/plain", "File not found: " + filename);
        return;
    }

    File file = SPIFFS.open(filename, "r");
    request->send(file, filename, "application/octet-stream"); });

    server.on("/restart", HTTP_GET, [this](AsyncWebServerRequest *request)
              {
                  request->send(200, "text/html", renderer->generateRestartPage());
                  delay(500);    // Let response flush
                  ESP.restart(); // Soft reboot (optional)
              });

    server.on("/submit-section", HTTP_POST, [this](AsyncWebServerRequest *request)
              {
    if (!configManager) {
        request->send(500, "text/plain", "❌ Configuration manager unavailable.");
        return;
    }

    Serial.println("📩 Handling /submit-section post...");

    std::map<String, std::map<String, String>> updates;

    for (size_t i = 0; i < request->params(); ++i) {
        AsyncWebParameter* p = request->getParam(i);
        const String& name = p->name();   // Expected: section.key
        const String& value = p->value();

        int split = name.indexOf('.');
        if (split == -1) {
            Serial.println("⚠️ Skipping invalid field name: " + name);
            continue;
        }

        String section = name.substring(0, split);
        String key = name.substring(split + 1);
        String finalValue = (value == "on") ? "true" : value;

        updates[section][key] = finalValue;
        Serial.printf("🔧 Updating [%s][%s] = %s\n", section.c_str(), key.c_str(), finalValue.c_str());
    }

    // Apply to configManager2
    for (const auto& s : updates) {
        for (const auto& kv : s.second) {
            configManager->setValue(s.first, kv.first, kv.second);
        }
    }

    configManager->saveToJson("/config.json", configManager->getConfig());

    // Flattened view for summary page
    std::map<String, String> flat;
    for (const auto& s : updates) {
        for (const auto& kv : s.second) {
            flat[s.first + "." + kv.first] = kv.second;
        }
    }

    request->send(200, "text/html", renderer->renderConfigSubmitSummaryPage(flat)); });

    /*!SECTION

        server.on("/submit-section", HTTP_POST, [this](AsyncWebServerRequest *request)
                  { handleFullConfigFormSubmission(request); });
    */

    server.on("/upload", HTTP_POST, [](AsyncWebServerRequest *request)
              { request->send(200, "text/html",
                              "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Upload Complete</title></head><body>"
                              "<h2>✅ File uploaded to SPIFFS</h2><a href='/home'>Back to Home</a></body></html>"); }, [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
              {
            static File uploadFile;

            if (index == 0)
            {
                Serial.printf("📁 Starting upload: %s\n", filename.c_str());
                if (SPIFFS.exists("/" + filename))
                    SPIFFS.remove("/" + filename);
                uploadFile = SPIFFS.open("/" + filename, FILE_WRITE);
            }

            if (uploadFile)
                uploadFile.write(data, len);

            if (final)
            {
                Serial.printf("✅ Completed upload: %s (%u bytes)\n", filename.c_str(), index + len);
                uploadFile.close();
            } });

    server.on(
        "/update", HTTP_POST,
        [](AsyncWebServerRequest *request)
        {
            request->send(200, "text/html",
                          "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Firmware Updated</title></head><body>"
                          "<h2>✅ Firmware uploaded successfully. Rebooting...</h2>"
                          "<a href='/home'>Back to Home</a></body></html>");
            delay(1000);
            ESP.restart();
        },
        [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
        {
            static size_t totalReceived = 0;
            static size_t contentLen = 0;

            if (index == 0)
            {
                contentLen = request->contentLength();
                totalReceived = 0;
                Serial.printf("⚙️ OTA Begin: %s (%u bytes expected)\n", filename.c_str(), contentLen);
                if (!Update.begin())
                {
                    Update.printError(Serial);
                }
            }

            totalReceived += len;
            if (Update.write(data, len) != len)
            {
                Update.printError(Serial);
            }

            // Log progress every 10%
            static int lastProgress = 0;
            int progress = (100 * totalReceived) / contentLen;
            if (progress >= lastProgress + 10 || final)
            {
                Serial.printf("📶 Progress: %d%% (%u/%u bytes)\n", progress, totalReceived, contentLen);
                lastProgress = progress;
            }

            if (final)
            {
                if (Update.end(true))
                {
                    Serial.printf("✅ OTA Complete. Total: %u bytes\n", totalReceived);
                }
                else
                {
                    Update.printError(Serial);
                }
            }
        });
    server.serveStatic("/schema_manual.html", SPIFFS, "/schema_manual.html.gz")
        .setCacheControl("max-age=3600");

    server.begin();
    Serial.println("✅ WebUI routes registered and server is live.");
}

void webUI::handleFirmwareUploadPage(AsyncWebServerRequest *request)
{
    String html = String("<!DOCTYPE html><html><head><meta charset='UTF-8'><title>OTA Update</title>") + renderer->getStyle() + "</head><body>";
    html += "<h1>📦 Firmware Update</h1>";
    html += "<form method='POST' action='/update' enctype='multipart/form-data'>";
    html += "<input type='file' name='firmware'>";
    html += "<input type='submit' value='Upload & Install'>";
    html += "</form><br>";
    html += "<a href='/home'>Back to Home</a></body></html>";

    request->send(200, "text/html", html);
}

void webUI::handleFullConfigFormSubmission(AsyncWebServerRequest *request, bool verbose)
{
    if (!configManager)
    {
        request->send(500, "text/plain", "❌ Configuration manager not available.");
        return;
    }

    if (verbose)
        Serial.println("📨 Received full config form submission...");

    std::map<String, std::map<String, String>> updated;

    for (size_t i = 0; i < request->params(); ++i)
    {
        AsyncWebParameter *param = request->getParam(i);
        const String &name = param->name(); // Expected: section.key
        const String &value = param->value();

        int dotIndex = name.indexOf('.');
        if (dotIndex == -1)
        {
            if (verbose)
                Serial.println("⚠️ Skipping malformed field name: " + name);
            continue;
        }

        String section = name.substring(0, dotIndex);
        String key = name.substring(dotIndex + 1);
        String finalValue = (value == "on") ? "true" : value;

        updated[section][key] = finalValue;
        if (verbose)
            Serial.printf("💾 Parsed: [%s][%s] = %s\n", section.c_str(), key.c_str(), finalValue.c_str());
    }

    // Apply updates to config manager
    for (const auto &sectionPair : updated)
    {
        const String &section = sectionPair.first;
        for (const auto &kv : sectionPair.second)
        {
            configManager->setValue(section, kv.first, kv.second);
        }
    }

    // Save config to disk
    configManager->saveToJson("/config.json", configManager->getConfig());

    // Optionally: show confirmation page
    std::map<String, String> flatUpdates;
    for (const auto &s : updated)
    {
        for (const auto &kv : s.second)
        {
            flatUpdates[s.first + "." + kv.first] = kv.second;
        }
    }

    request->send(200, "text/html", renderer->renderConfigSubmitSummaryPage(flatUpdates));
}
