#include "WeatherService.h"

WeatherService::WeatherService() {}

bool WeatherService::begin() {
  if (LittleFS.begin()) {
    activeLocation = readLocation();
  }
  if (activeLocation.length() == 0) {
    activeLocation = OPEN_WEATHER_MAP_LOCATION_ID;
  }

  bool looksLikeId = true;
  for (unsigned int c = 0; c < activeLocation.length(); c++) {
    if (!isDigit(activeLocation.charAt(c))) {
      looksLikeId = false;
      break;
    }
  }

  return looksLikeId ? updateWeatherById(activeLocation) : updateWeatherByName(activeLocation);
}

String WeatherService::urlEncode(const String& str) {
  String encoded = "";
  for (unsigned int i = 0; i < str.length(); i++) {
    char c = str.charAt(i);
    if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
      encoded += c;
    } else if (c == ' ') {
      encoded += "%20";
    } else {
      char code[4];
      snprintf(code, sizeof(code), "%%%02X", (unsigned char)c);
      encoded += code;
    }
  }
  return encoded;
}

// Map temperature to 256-color RGB representation for MiP's chest LED
void WeatherService::updateChestColor() {
  if (data.temp != 0) {
    // Range of blue.
    if (data.temp <= 32) {
      blue = 255;
    } else if (data.temp > 32 && data.temp <= 72) {
      blue = map(data.temp, 32, 72, 255, 0);
    } else if (data.temp > 72) {
      blue = 0;
    }

    // Range of green.
    if (data.temp <= 32) {
      green = 0;
    } else if (data.temp > 32 && data.temp <= 60) {
      green = map(data.temp, 32, 60, 0, 255);
    } else if (data.temp > 60 && data.temp <= 80) {
      green = 255;
    } else if (data.temp > 80) {
      green = map(data.temp, 80, 110, 255, 0);
    }

    // Range of red.
    if (data.temp < 72) {
      red = 0;
    } else if (data.temp >= 72 && data.temp <= 80) {
      red = map(data.temp, 72, 80, 1, 255);
    } else if (data.temp > 80) {
      red = 255;
    }
  }
}

bool WeatherService::updateWeatherById(const String& locationId) {
  client.setLanguage(OPEN_WEATHER_MAP_LANGUAGE);
  client.setMetric(IS_METRIC);

  for (uint8_t i = 0; i < HTTP_RETRIES; i++) {
    OpenWeatherMapCurrentData tempData;
    client.updateCurrentById(&tempData, OPEN_WEATHER_MAP_APP_ID, locationId);
    if (tempData.cityName.length() > 0) {
      data = tempData;
      updateChestColor();  // Automatically update RGB colors for chest LED
      Serial1.println("Found data for " + data.cityName + ", " + data.country + ".");
      return true;
    }
  }
  Serial1.println(F("Failed updating weather by ID."));
  return false;
}

bool WeatherService::updateWeatherByName(const String& cityName) {
  client.setLanguage(OPEN_WEATHER_MAP_LANGUAGE);
  client.setMetric(IS_METRIC);

  String encodedQuery = urlEncode(cityName);

  for (uint8_t i = 0; i < HTTP_RETRIES; i++) {
    OpenWeatherMapCurrentData tempData;
    client.updateCurrent(&tempData, OPEN_WEATHER_MAP_APP_ID, encodedQuery);
    if (tempData.cityName.length() > 0) {
      data = tempData;
      updateChestColor();  // Automatically update RGB colors for chest LED
      Serial1.println("Found data for " + data.cityName + ".");
      return true;
    }
  }
  Serial1.println(F("Failed updating weather by name."));
  return false;
}

void WeatherService::saveLocation(const String& location) {
  LittleFS.remove("/location.txt");
  File saveFile = LittleFS.open("/location.txt", "w");
  if (saveFile) {
    saveFile.println(location);
    saveFile.close();
  }
}

String WeatherService::readLocation() {
  File saveFile = LittleFS.open("/location.txt", "r");
  if (!saveFile) return "";
  String savedLocation = saveFile.readStringUntil('\n');
  saveFile.close();
  savedLocation.trim();
  return savedLocation;
}
