#include <MiP_Power_Up_-_D1_mini.h>
#include <ArduinoOTA.h>
#include "Config.h"
#include "WeatherService.h"
#include "WebServerManager.h"
#include "secrets.h"  // Includes the SSID and password.

// Hardware and Service instances
MiP mip;
WeatherService weatherService;
WebServerManager webServer(weatherService, mip);

// MiP State variables
bool chestValuesWritten = false;
bool lastUpdatedToSolid = false;
bool extinguished = false;

unsigned long previousWeatherMillis = 0;
unsigned long previousEyesMillis = 0;
MiPPosition lastPosition = (MiPPosition)-1;

void setup() {
  if (!mip.begin()) {
    Serial1.println(F("Failed connecting to MiP."));
    return;
  }

  mip.wifi.begin(SECRET_SSID, SECRET_PASSWORD, HOSTNAME);
  ArduinoOTA.begin();
  randomSeed(secureRandom(0, USHRT_MAX));

  webServer.begin();
  weatherService.begin();

  mip.clap.enableEvents();
  mip.clap.writeDelay(1000);
}

void loop() {
  ArduinoOTA.handle();

  webServer.processPendingWeatherUpdate(chestValuesWritten, lastUpdatedToSolid, extinguished);

  unsigned long currentMillis = millis();
  if (currentMillis - previousWeatherMillis >= WEATHER_UPDATE_INTERVAL) {
    weatherService.begin();
    chestValuesWritten = false;
    previousWeatherMillis = currentMillis;
  }

  MiPPosition currentPosition = mip.position.read();
  if (currentPosition != lastPosition) {
    if (mip.position.isOnBackWithKickstand()) {
      mip.motion.stop();
      mip.radar.disable();
      mip.clap.enableEvents();
      mip.chestLED.write(weatherService.red, weatherService.green, weatherService.blue);
    }
    lastPosition = currentPosition;
  }

  // MiP is on his kickstand - start reporting the weather.
  if (mip.position.isOnBackWithKickstand()) {
    // Listen for claps first
    while (mip.clap.availableEvents() > 0) {
      uint8_t clapCount = mip.clap.readEvent();
      if (clapCount > 0) {
        extinguished = !extinguished;
        if (extinguished) {
          mip.headLEDs.write(MIP_HEAD_LED_OFF, MIP_HEAD_LED_OFF, MIP_HEAD_LED_OFF, MIP_HEAD_LED_OFF);
          mip.chestLED.write(0, 0, 0);
          lastUpdatedToSolid = false;
          chestValuesWritten = false;
        }
      }
    }

    if (!extinguished) {
      // Animate MiP's eyes based on weather conditions
      if (weatherService.data.description.indexOf("rain") >= 0) {
        unsigned long eyesMillis = millis();
        if (eyesMillis - previousEyesMillis >= EYES_RAIN_INTERVAL) {
          mip.headLEDs.unverifiedWrite(
            (MiPHeadLED)random(0, 2),
            (MiPHeadLED)random(0, 2),
            (MiPHeadLED)random(0, 2),
            (MiPHeadLED)random(0, 2));
          previousEyesMillis = eyesMillis;
        }
        lastUpdatedToSolid = false;
      } else if (weatherService.data.description.indexOf("drizzle") >= 0) {
        unsigned long eyesMillis = millis();
        if (eyesMillis - previousEyesMillis >= EYES_DRIZZLE_INTERVAL) {
          mip.headLEDs.unverifiedWrite(
            (MiPHeadLED)random(0, 2),
            (MiPHeadLED)random(0, 2),
            (MiPHeadLED)random(0, 2),
            (MiPHeadLED)random(0, 2));
          previousEyesMillis = eyesMillis;
        }
        lastUpdatedToSolid = false;
      } else if (weatherService.data.description.indexOf("mist") >= 0) {
        unsigned long eyesMillis = millis();
        if (eyesMillis - previousEyesMillis >= EYES_MIST_INTERVAL) {
          mip.headLEDs.unverifiedWrite(
            (MiPHeadLED)random(0, 2),
            (MiPHeadLED)random(0, 2),
            (MiPHeadLED)random(0, 2),
            (MiPHeadLED)random(0, 2));
          previousEyesMillis = eyesMillis;
        }
        lastUpdatedToSolid = false;
      } else if (!lastUpdatedToSolid) {
        // If there is no rain/drizzle/mist, keep the eyes solid ON
        mip.headLEDs.write(MIP_HEAD_LED_ON, MIP_HEAD_LED_ON, MIP_HEAD_LED_ON, MIP_HEAD_LED_ON);
        lastUpdatedToSolid = true;
      }

      // Update hardware chest LED if needed
      if (!chestValuesWritten) {
        mip.chestLED.write(weatherService.red, weatherService.green, weatherService.blue);
        chestValuesWritten = true;
      }
    }
  }

  webServer.handleClient();
}
