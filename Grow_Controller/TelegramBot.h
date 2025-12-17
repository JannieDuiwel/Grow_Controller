#ifndef TELEGRAMBOT_H
#define TELEGRAMBOT_H

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include "Console.h"
#include "OTAUpdate.h"

// --- credentials ---
const char* WIFI_SSID = "YourSSID";
const char* WIFI_PASS = "YourPassword";
const char* BOT_TOKEN = "123456789:ABCDEF";  // from @BotFather
const String CHAT_ID  = "123456789";         // your own chat id

WiFiClientSecure secureClient;
UniversalTelegramBot bot(BOT_TOKEN, secureClient);

unsigned long lastBotCheck = 0;
const unsigned long BOT_INTERVAL = 5000;

// ------------------------------------------------------------------
void initWiFi() {
  Serial.printf("Connecting to %s ...\n", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi connected");
  secureClient.setCACert(TELEGRAM_CERTIFICATE_ROOT);
}

// ------------------------------------------------------------------
void handleTelegram(float temp, float hum, float motherSoil) {
  if (millis() - lastBotCheck < BOT_INTERVAL) return;
  lastBotCheck = millis();

  int newMsgs = bot.getUpdates(bot.last_message_received + 1);
  while (newMsgs) {
    for (int i=0; i<newMsgs; i++) {
      String chat_id = String(bot.messages[i].chat_id);
      String text = bot.messages[i].text;

      if (chat_id != CHAT_ID) continue;

      if (text == "/start") {
        bot.sendMessage(CHAT_ID,
          "🌿 Greenhouse Bot ready.\n"
          "Commands:\n"
          "/status – show readings\n"
          "/set <room> <param> <value>\n"
          "/update – pull latest firmware\n"
          "/reboot – restart controller\n"
          "/help – show this list", "");
      }
      else if (text == "/status") {
        char buf[200];
        sprintf(buf, "Temp: %.1f°C\nHum: %.1f%%\nMother soil: %.0f",
                temp, hum, motherSoil);
        bot.sendMessage(CHAT_ID, buf, "");
      }
      else if (text.startsWith("/set ")) {
        Serial.println(text.substring(1)); // feed to serial handler
        handleSerial(temp, hum, motherSoil);
        bot.sendMessage(CHAT_ID, "✅ Command executed", "");
      }
      else if (text == "/update") {
        bot.sendMessage(CHAT_ID, "⬇️ Fetching latest firmware...", "");
        performOTA();
      }
      else if (text == "/reboot") {
        bot.sendMessage(CHAT_ID, "♻️ Rebooting...", "");
        delay(1000);
        ESP.restart();
      }
      else if (text == "/help") {
        bot.sendMessage(CHAT_ID,
          "Usage:\n/status\n/set <room> <param> <value>\n/update\n/reboot", "");
      }
      else {
        bot.sendMessage(CHAT_ID, "Unknown command. Use /help", "");
      }
    }
    newMsgs = bot.getUpdates(bot.last_message_received + 1);
  }
}
#endif
