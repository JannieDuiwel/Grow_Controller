#ifndef OTAUPDATE_H
#define OTAUPDATE_H

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <Update.h>

// === OTA CONFIG ===
// Use your raw GitHub URL here:
const char* OTA_URL =
  "https://github.com/JannieDuiwel/Grow_Controller/raw/refs/heads/main/Grow_Controller.bin";

void performOTA() {
  Serial.println("🔁 Starting OTA update from GitHub RAW...");

  WiFiClientSecure client;
  client.setInsecure(); // bypass SSL cert verification for GitHub

  HTTPClient https;
  if (!https.begin(client, OTA_URL)) {
    Serial.println("❌ HTTPS init failed");
    return;
  }

  int httpCode = https.GET();
  if (httpCode != HTTP_CODE_OK) {
    Serial.printf("❌ HTTP failed, code: %d\n", httpCode);
    https.end();
    return;
  }

  int contentLength = https.getSize();
  WiFiClient *stream = (WiFiClient *)https.getStreamPtr();

  if (!Update.begin(contentLength)) {
    Serial.println("❌ Not enough space for OTA");
    https.end();
    return;
  }

  Serial.printf("⬇️  Downloading %d bytes...\n", contentLength);
  size_t written = Update.writeStream(*stream);

  if (written == contentLength)
    Serial.println("✅ Download complete");
  else
    Serial.printf("⚠️ Written only %d/%d bytes\n", written, contentLength);

  if (Update.end()) {
    if (Update.isFinished()) {
      Serial.println("🎉 OTA successful, rebooting...");
      delay(1000);
      ESP.restart();
    } else {
      Serial.println("⚠️ OTA not finished");
    }
  } else {
    Serial.printf("❌ OTA error #%u: %s\n", Update.getError(), Update.errorString());
  }

  https.end();
}

#endif
