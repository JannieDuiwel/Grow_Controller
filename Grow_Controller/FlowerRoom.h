#ifndef FLOWERROOM_H
#define FLOWERROOM_H

#include "RoomBase.h"

// === Watering Configuration (Flower) ===
#define FLOWER_WATER_INTERVAL_MIN  180   // minutes between floods
#define FLOWER_WATER_DURATION_SEC   60   // pump ON duration
// Uses POST_WATER_DELAY_MIN from Config.h

class FlowerRoom {
public:
  RoomConfig cfg;
  int soilPins[4] = {36, 39, 25, 26};
  RelayController relays = RelayController(23, 25, 26, 27, 14);

  bool lightState = false;
  bool wateringActive = false;
  unsigned long lastWaterTime = 0;
  unsigned long floodStart = 0;

  FlowerRoom() {
    cfg = {
      "Flower Room", 24.0, 55.0, 2200,
      DEFAULT_TEMP_THRESHOLD, DEFAULT_HUMIDITY_THRESHOLD, DEFAULT_SOIL_THRESHOLD,
      12UL * 3600000UL, 12UL * 3600000UL
    };
  }

  void begin() { relays.begin(); }

  void update(float temp, float hum) {
    float soilAvg = readSoilAverage(soilPins, 4);
    controlEnvironment(temp, hum, soilAvg);
    handleLighting();
    manageWatering(soilAvg);
    printStatus(temp, hum, soilAvg);
  }

private:
  void controlEnvironment(float temp, float hum, float soil) {
    static bool heaterOn = false, exhaustOn = false;

    if (!heaterOn && temp < cfg.idealTemp - cfg.tempThreshold) {
      relays.heater(true); heaterOn = true;
    } else if (heaterOn && temp > cfg.idealTemp + cfg.tempThreshold) {
      relays.heater(false); heaterOn = false;
    }

    if (!exhaustOn && temp > cfg.idealTemp + cfg.tempThreshold) {
      relays.exhaust(true); exhaustOn = true;
    } else if (exhaustOn && temp < cfg.idealTemp - cfg.tempThreshold) {
      relays.exhaust(false); exhaustOn = false;
    }
  }

  void manageWatering(float soilAvg) {
    unsigned long now = millis();
    unsigned long intervalMs = FLOWER_WATER_INTERVAL_MIN * 60000UL;
    unsigned long durationMs = FLOWER_WATER_DURATION_SEC * 1000UL;
    unsigned long postDelayMs = POST_WATER_DELAY_MIN * 60000UL;

    if (!wateringActive) {
      bool intervalOK = (now - lastWaterTime) >= intervalMs;
      if (intervalOK && soilAvg < cfg.idealSoil - cfg.soilThreshold) {
        relays.water(true);
        wateringActive = true;
        floodStart = now;
        Serial.println("[FLOWER] 🌊 Flood started");
      }
    } else {
      if (now - floodStart >= durationMs) {
        relays.water(false);
        wateringActive = false;
        lastWaterTime = now + postDelayMs;
        Serial.println("[FLOWER] ✅ Flood ended, rest period active");
      }
    }
  }

  void handleLighting() {
    unsigned long now = millis();
    unsigned long cycle = cfg.lightOnDuration + cfg.lightOffDuration;
    bool shouldBeOn = (now % cycle) < cfg.lightOnDuration;
    if (shouldBeOn != lightState) {
      relays.light(shouldBeOn);
      lightState = shouldBeOn;
    }
  }

  void printStatus(float t, float h, float s) {
    Serial.println("---- FLOWER ROOM ----");
    Serial.printf("Temp: %.1f°C  Hum: %.1f%%  SoilAvg: %.0f\n", t, h, s);
    Serial.printf("Light: %s | Pump: %s\n",
      lightState ? "ON" : "OFF",
      relays.getState("water") ? "ON" : "OFF");
    Serial.println("---------------------");
  }
};

#endif
