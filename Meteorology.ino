#include <MiP_Power_Up_-_D1_mini.h>
#include <ArduinoOTA.h>
#include "Config.h"
#include "WeatherService.h"
#include "WebServerManager.h"

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

  mip.wifi.begin(SSID, PASSWORD, HOSTNAME);
  ArduinoOTA.begin();
  randomSeed(analogRead(A0));

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

  if (mip.position.isOnBackWithKickstand()) {
    while (mip.clap.availableEvents() > 0) {
      uint8_t clapCount = mip.clap.readEvent();
      if (clapCount > 0) {
        extinguished = !extinguished;
        if (extinguished) {
          mip.headLEDs.write(MIP_HEAD_LED_OFF, MIP_HEAD_LED_OFF, MIP_HEAD_LED_OFF, MIP_HEAD_LED_OFF);
          mip.chestLED.write(0, 0, 0);
        }
      }
    }

    if (!extinguished) {
      // Update hardware chest LED if needed
      if (!chestValuesWritten) {
        mip.chestLED.write(weatherService.red, weatherService.green, weatherService.blue);
        chestValuesWritten = true;
      }
    }
  }

  webServer.handleClient();
}
