#ifndef WEATHER_SERVICE_H
#define WEATHER_SERVICE_H

#include <Arduino.h>
#include <OpenWeatherMapCurrent.h>
#include <LittleFS.h>
#include "Config.h"

class WeatherService {
public:
  WeatherService();
  
  bool begin();
  bool updateWeatherById(const String& locationId);
  bool updateWeatherByName(const String& cityName);
  
  void saveLocation(const String& location);
  String readLocation();
  String urlEncode(const String& str);
  void updateChestColor(); // Calculates 256-color RGB from data.temp

  OpenWeatherMapCurrentData data;
  String activeLocation;

  // Chest LED RGB values based on temperature
  uint8_t red = 0;
  uint8_t green = 0;
  uint8_t blue = 0;

private:
  OpenWeatherMapCurrent client;
};

#endif // WEATHER_SERVICE_H
