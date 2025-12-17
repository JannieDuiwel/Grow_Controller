#ifndef VEGROOM_H
#define VEGROOM_H

#include "RoomBase.h"

// === Watering Configuration ===
#define VEG_WATER_INTERVAL_MIN     120   // minutes between Veg floods
#define VEG_WATER_DURATION_SEC      45   // seconds pump ON
#define MOTHER_WATER_INTERVAL_MIN  240   // minutes between Mother floods
#define MOTHER_WATER_DURATION_SEC   30   // seconds Mother solenoid ON
// Shared POST_WATER_DELAY_MIN from Config.h

class VegRoom {
public:
  RoomConfig cfg;
  int soilPins[4] = {34, 35, 32, 33};  // Veg sensors
  int motherSoilPin = 27;              // Mother soil sensor
  RelayController relays = RelayController(5, 18, 19, 21, 22);

  // state tracking
  bool lightState = false;

  // Veg flood state
  bool vegWatering = false;
  unsigned long vegLastWaterTime = 0;
  unsigned long vegFloodStart = 0;

  // Mother flood state
  bool motherWatering = false;
  unsigned long motherLastWaterTime = 0;
  unsigned long motherFloodStart = 0;

  float motherSoilReading = 0;

  VegRoom() {
    cfg = {
      "Veg Room", 26.0, 60.0, 2000,
      DEFAULT_TEMP_THRESHOLD, DEFAULT_HUMIDITY_THRESHOLD, DEFAULT_SOIL_THRESHOLD,
      18UL * 3600000UL, 6UL * 3600000UL
    };
  }

  void begin() {
    relays.begin();
    pinMode(motherSoilPin, INPUT);
  }

  float getMotherSoil() { return motherSoilReading; }

  void update(float temp, float hum) {
    float soilAvg = readSoilAverage(soilPins, 4);
    motherSoilReading = analogRead(motherSoilPin);

    controlEnvironment(temp, hum, soilAvg);
    handleLighting();
    manageVegWatering(soilAvg);
    manageMotherWatering(motherSoilReading);

    printStatus(temp, hum, soilAvg, motherSoilReading);
  }

private:
  void controlEnvironment(float temp, float hum, float soil) {
    static bool heaterOn = false, exhaustOn = false;

    // ---- Temperature hysteresis ----
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

  // ---- Veg flood logic ----
  void manageVegWatering(float soilAvg) {
    unsigned long now = millis();
    unsigned long intervalMs = VEG_WATER_INTERVAL_MIN * 60000UL;
    unsigned long durationMs = VEG_WATER_DURATION_SEC * 1000UL;
    unsigned long postDelayMs = POST_WATER_DELAY_MIN * 60000UL;

    if (!vegWatering) {
      bool intervalOK = (now - vegLastWaterTime) >= intervalMs;
      if (intervalOK && soilAvg < cfg.idealSoil - cfg.soilThreshold) {
        relays.water(true);
        vegWatering = true;
        vegFloodStart = now;
        Serial.println("[VEG] 🌊 Flood started");
      }
    } else {
      if (now - vegFloodStart >= durationMs) {
        relays.water(false);
        vegWatering = false;
        vegLastWaterTime = now + postDelayMs;
        Serial.println("[VEG] ✅ Flood ended, rest period active");
      }
    }
  }

  // ---- Mother flood logic ----
  void manageMotherWatering(float motherSoil) {
    unsigned long now = millis();
    unsigned long intervalMs = MOTHER_WATER_INTERVAL_MIN * 60000UL;
    unsigned long durationMs = MOTHER_WATER_DURATION_SEC * 1000UL;
    unsigned long postDelayMs = POST_WATER_DELAY_MIN * 60000UL;

    if (!motherWatering) {
      bool intervalOK = (now - motherLastWaterTime) >= intervalMs;
      if (intervalOK && motherSoil < 2100 - cfg.soilThreshold) {
        relays.intake(true); // Intake = Mother solenoid
        motherWatering = true;
        motherFloodStart = now;
        Serial.println("[MOTHER] 🌊 Flood started");
      }
    } else {
      if (now - motherFloodStart >= durationMs) {
        relays.intake(false);
        motherWatering = false;
        motherLastWaterTime = now + postDelayMs;
        Serial.println("[MOTHER] ✅ Flood ended, rest period active");
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

  void printStatus(float t, float h, float s, float m) {
    Serial.println("---- VEG ROOM ----");
    Serial.printf("Temp: %.1f°C  Hum: %.1f%%  SoilAvg: %.0f  MotherSoil: %.0f\n", t, h, s, m);
    Serial.printf("Light: %s | VegPump: %s | MotherValve: %s\n",
      lightState ? "ON" : "OFF",
      relays.getState("water") ? "ON" : "OFF",
      relays.getState("intake") ? "ON" : "OFF");
    Serial.println("------------------");
  }
};

#endif
