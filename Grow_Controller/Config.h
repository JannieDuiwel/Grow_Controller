#ifndef CONFIG_H
#define CONFIG_H

#include <Wire.h>
#include <Adafruit_AHTX0.h>
#include <Preferences.h>

extern Adafruit_AHTX0 aht;
extern Preferences prefs;

// Shared constants ----------------------------------------------------
#define DEFAULT_SOIL_THRESHOLD     200
#define DEFAULT_TEMP_THRESHOLD     1.0
#define DEFAULT_HUMIDITY_THRESHOLD 5.0
#define DEFAULT_MIN_RUN_TIME_MS    10000UL   // 10 s minimum relay run

// Shared post-water delay (minutes)
#define POST_WATER_DELAY_MIN       60

// Function declarations
void loadConfig(struct RoomConfig &cfg, const char* ns);
void saveConfig(const struct RoomConfig &cfg, const char* ns);

#endif
