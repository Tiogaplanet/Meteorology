#ifndef WEB_SERVER_MANAGER_H
#define WEB_SERVER_MANAGER_H

#include <ESP8266WebServer.h>
#include <MiP_Power_Up_-_D1_mini.h>
#include "WeatherService.h"
#include "Config.h"

class WebServerManager {
public:
  WebServerManager(WeatherService& weatherSvc, MiP& mipRef);

  void begin();
  void handleClient();
  void processPendingWeatherUpdate(bool& chestWritten, bool& solidUpdated, bool extinguished);

  bool weatherUpdatePending;
  bool searchError;

private:
  ESP8266WebServer server;
  WeatherService& weather;
  MiP& mip;

  String pendingLocation;
  bool pendingWasById;

  void handleRoot();
  void handleNotFound();

  String completePage();
  String htmlHead();
  String htmlBody();
  String htmlMenuBar();
  String htmlHeader();
  String htmlFooter();
  String htmlWeatherData();
  String chestHTML(uint8_t redHTML, uint8_t greenHTML, uint8_t blueHTML);
};

#endif  // WEB_SERVER_MANAGER_H
