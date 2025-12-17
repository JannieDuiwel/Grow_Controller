#include "Config.h"
#include "RoomBase.h"
#include "VegRoom.h"
#include "FlowerRoom.h"
#include "Console.h"
#include <Preferences.h>

// ------------------------------------------------------------------
// Optional Telegram interface
// ------------------------------------------------------------------
#define USE_TELEGRAM  true   // set to false to disable all WiFi/Telegram features
#if USE_TELEGRAM
  #include "TelegramBot.h"
#endif
// ------------------------------------------------------------------

Adafruit_AHTX0 aht;
Preferences prefs;

VegRoom vegRoom;
FlowerRoom flowerRoom;

void safeStartup() {
  int allPins[] = {5,18,19,21,22,23,25,26,27,14};
  for (int i = 0; i < 10; i++) {
    pinMode(allPins[i], OUTPUT);
    digitalWrite(allPins[i], LOW);
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  safeStartup();
  Serial.println("🌿 ESP32 Greenhouse Controller Booting...");

  Wire.begin();
  if (!aht.begin()) {
    Serial.println("⚠️ Could not find DHT20 sensor! Check wiring.");
    while (1) delay(10);
  }

  vegRoom.begin();
  flowerRoom.begin();

  loadConfig(vegRoom.cfg, "veg");
  loadConfig(flowerRoom.cfg, "flower");

  showHelp();

#if USE_TELEGRAM
  initWiFi();
#endif
}

void loop() {
  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);

  if (isnan(temp.temperature) || isnan(humidity.relative_humidity)) {
    Serial.println("⚠️ Sensor read failed, skipping cycle");
    delay(2000);
    return;
  }

  vegRoom.update(temp.temperature, humidity.relative_humidity);
  flowerRoom.update(temp.temperature, humidity.relative_humidity);

  handleSerial(temp.temperature, humidity.relative_humidity, vegRoom.getMotherSoil());

#if USE_TELEGRAM
  handleTelegram(temp.temperature, humidity.relative_humidity, vegRoom.getMotherSoil());
#endif

  delay(5000);
}
