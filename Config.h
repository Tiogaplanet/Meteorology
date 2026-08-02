#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

const String OPEN_WEATHER_MAP_LOCATION_ID = "6077243";
const String OPEN_WEATHER_MAP_LANGUAGE = "en";
const bool IS_METRIC = false;

// Hostname & Network Defaults
const char* const HOSTNAME = "MiP-Meteorologist";
#define HTTP_RETRIES 3
const unsigned long WEATHER_UPDATE_INTERVAL = 900000;  // 15 minutes

// Eye animation intervals (in milliseconds)
const unsigned long EYES_RAIN_INTERVAL = 500;
const unsigned long EYES_DRIZZLE_INTERVAL = 1000;
const unsigned long EYES_MIST_INTERVAL = 3000;

// Location Aliases
struct LocationAlias {
  const char* informal;   // User typed input (lower-case)
  const char* canonical;  // OpenWeatherMap query string
};

const LocationAlias LOCATION_ALIASES[] = {
  { "joint base andrews", "Camp Springs,MD,US" },
  { "jb andrews", "Camp Springs,MD,US" },
  { "jb andrews, md", "Camp Springs,MD,US" },
  { "andrews afb", "Camp Springs,MD,US" },
  { "andrews air force base", "Camp Springs,MD,US" },
};

const size_t NUM_ALIASES = sizeof(LOCATION_ALIASES) / sizeof(LOCATION_ALIASES[0]);

#endif  // CONFIG_H
