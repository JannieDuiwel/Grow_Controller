#ifndef CONSOLE_H
#define CONSOLE_H

#include "VegRoom.h"
#include "FlowerRoom.h"
#include "OTAUpdate.h"
#include <Preferences.h>

extern VegRoom vegRoom;
extern FlowerRoom flowerRoom;
extern Preferences prefs;

void showHelp() {
  Serial.println("========== GREENHOUSE CONSOLE ==========");
  Serial.println("help                       - Show this menu");
  Serial.println("status                     - Print sensor + relay data");
  Serial.println("set <room> <param> <value> - Change config value");
  Serial.println("   room: veg | flower | mother");
  Serial.println("   param: temp | humidity | soil");
  Serial.println("save                       - Save current configs");
  Serial.println("update                     - Perform OTA update from GitHub");
  Serial.println("reboot                     - Restart ESP32");
  Serial.println("----------------------------------------");
}

void showStatus(float temp, float hum, float motherSoil) {
  Serial.printf("Temp: %.1f°C  Humidity: %.1f%%  MotherSoil: %.0f\n",
                temp, hum, motherSoil);
  Serial.printf("Veg -> Temp %.1f Hum %.1f Soil %d\n",
                vegRoom.cfg.idealTemp, vegRoom.cfg.idealHumidity, vegRoom.cfg.idealSoil);
  Serial.printf("Flower -> Temp %.1f Hum %.1f Soil %d\n",
                flowerRoom.cfg.idealTemp, flowerRoom.cfg.idealHumidity, flowerRoom.cfg.idealSoil);
}

void saveAllConfigs() {
  saveConfig(vegRoom.cfg, "veg");
  saveConfig(flowerRoom.cfg, "flower");
  Serial.println("✅ Configs saved to NVS.");
}

void handleSerial(float temp, float hum, float motherSoil) {
  if (!Serial.available()) return;
  String line = Serial.readStringUntil('\n');
  line.trim();
  if (line.length() == 0) return;

  // --- core commands ---
  if (line.equalsIgnoreCase("help")) { showHelp(); return; }
  if (line.equalsIgnoreCase("status")) { showStatus(temp, hum, motherSoil); return; }
  if (line.equalsIgnoreCase("save")) { saveAllConfigs(); return; }

  if (line.equalsIgnoreCase("update")) {
    Serial.println("Fetching latest firmware from GitHub...");
    performOTA();
    return;
  }

  if (line.equalsIgnoreCase("reboot")) {
    Serial.println("Rebooting...");
    delay(1000);
    ESP.restart();
  }

  // --- handle 'set' command ---
  if (line.startsWith("set ")) {
    line.remove(0, 4);
    int space1 = line.indexOf(' ');
    if (space1 == -1) { Serial.println("Format: set <room> <param> <value>"); return; }

    String room = line.substring(0, space1);
    line.remove(0, space1 + 1);

    int space2 = line.indexOf(' ');
    if (space2 == -1) { Serial.println("Missing parameter/value"); return; }

    String param = line.substring(0, space2);
    float value = line.substring(space2 + 1).toFloat();

    RoomConfig *cfg = nullptr;
    if (room.equalsIgnoreCase("veg") || room.equalsIgnoreCase("mother")) cfg = &vegRoom.cfg;
    else if (room.equalsIgnoreCase("flower")) cfg = &flowerRoom.cfg;
    else { Serial.println("Room must be veg, flower or mother"); return; }

    if (param.equalsIgnoreCase("temp")) cfg->idealTemp = value;
    else if (param.equalsIgnoreCase("humidity")) cfg->idealHumidity = value;
    else if (param.equalsIgnoreCase("soil")) cfg->idealSoil = (int)value;
    else { Serial.println("Param must be temp, humidity, or soil"); return; }

    Serial.printf("✅ Set %s %s = %.2f\n", room.c_str(), param.c_str(), value);
    return;
  }

  Serial.println("Unknown command. Type 'help' for list.");
}

#endif
