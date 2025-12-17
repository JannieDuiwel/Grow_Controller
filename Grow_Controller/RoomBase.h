#ifndef ROOMBASE_H
#define ROOMBASE_H

#include "Config.h"

// ---------- RoomConfig structure ----------
struct RoomConfig {
  String name;
  float idealTemp;
  float idealHumidity;
  int   idealSoil;
  float tempThreshold;
  float humidityThreshold;
  int   soilThreshold;
  unsigned long lightOnDuration;
  unsigned long lightOffDuration;
};

// ---------- Relay Controller (hysteresis + min-run) ----------
class RelayController {
public:
  struct RelayState {
    bool state = false;
    unsigned long lastChange = 0;
  };

  int exhaustPin, heaterPin, waterPin, lightPin, intakePin;
  unsigned long minRunTime = DEFAULT_MIN_RUN_TIME_MS;

  RelayState exhaustState, heaterState, waterState, lightState, intakeState;

  RelayController(int ex, int ht, int wt, int lt, int in)
    : exhaustPin(ex), heaterPin(ht), waterPin(wt), lightPin(lt), intakePin(in) {}

  void begin() {
    pinMode(exhaustPin, OUTPUT);
    pinMode(heaterPin, OUTPUT);
    pinMode(waterPin, OUTPUT);
    pinMode(lightPin, OUTPUT);
    pinMode(intakePin, OUTPUT);
    allOff();
  }

  void allOff() {
    setRelay(exhaustPin, exhaustState, false);
    setRelay(heaterPin,  heaterState,  false);
    setRelay(waterPin,   waterState,   false);
    setRelay(lightPin,   lightState,   false);
    setRelay(intakePin,  intakeState,  false);
  }

  void exhaust(bool s) { setRelay(exhaustPin, exhaustState, s); }
  void heater (bool s) { setRelay(heaterPin,  heaterState,  s); }
  void water  (bool s) { setRelay(waterPin,   waterState,   s); }
  void light  (bool s) { setRelay(lightPin,   lightState,   s); }
  void intake (bool s) { setRelay(intakePin,  intakeState,  s); }

  bool getState(String name) {
    if (name == "exhaust") return exhaustState.state;
    if (name == "heater")  return heaterState.state;
    if (name == "water")   return waterState.state;
    if (name == "light")   return lightState.state;
    if (name == "intake")  return intakeState.state;
    return false;
  }

private:
  void setRelay(int pin, RelayState &r, bool desiredState) {
    unsigned long now = millis();
    if (desiredState != r.state) {
      if (now - r.lastChange >= minRunTime) {
        r.state = desiredState;
        r.lastChange = now;
        digitalWrite(pin, r.state ? HIGH : LOW);
      }
    }
  }
};

// ---------- Soil average helper ----------
inline float readSoilAverage(int* pins, int count) {
  float total = 0;
  for (int i = 0; i < count; i++) total += analogRead(pins[i]);
  return total / count;
}

// ---------- NVS config I/O ----------
inline void loadConfig(RoomConfig &cfg, const char* ns) {
  prefs.begin(ns, true);
  cfg.idealTemp       = prefs.getFloat("temp", cfg.idealTemp);
  cfg.idealHumidity   = prefs.getFloat("hum",  cfg.idealHumidity);
  cfg.idealSoil       = prefs.getInt  ("soil", cfg.idealSoil);
  cfg.tempThreshold   = prefs.getFloat("tTh",  cfg.tempThreshold);
  cfg.humidityThreshold = prefs.getFloat("hTh", cfg.humidityThreshold);
  cfg.soilThreshold   = prefs.getInt  ("sTh",  cfg.soilThreshold);
  prefs.end();
}

inline void saveConfig(const RoomConfig &cfg, const char* ns) {
  prefs.begin(ns, false);
  prefs.putFloat("temp", cfg.idealTemp);
  prefs.putFloat("hum",  cfg.idealHumidity);
  prefs.putInt  ("soil", cfg.idealSoil);
  prefs.putFloat("tTh",  cfg.tempThreshold);
  prefs.putFloat("hTh",  cfg.humidityThreshold);
  prefs.putInt  ("sTh",  cfg.soilThreshold);
  prefs.end();
}

#endif
