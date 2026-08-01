#include "WebServerManager.h"

extern bool extinguished; // Defined in Meteorology.ino

WebServerManager::WebServerManager(WeatherService& weatherSvc, MiP& mipRef)
  : server(80), weather(weatherSvc), mip(mipRef), weatherUpdatePending(false), searchError(false) {}

void WebServerManager::begin() {
  server.on("/", std::bind(&WebServerManager::handleRoot, this));
  server.onNotFound(std::bind(&WebServerManager::handleNotFound, this));
  server.begin();
}

void WebServerManager::handleClient() {
  server.handleClient();
}

void WebServerManager::handleRoot() {
  if (server.args() > 0) {
    for (uint8_t i = 0; i < server.args(); i++) {
      if (server.argName(i) == "city") {
        String requested = server.arg(i);
        requested.trim();
        if (requested.length() == 0) continue;

        String lower = requested;
        lower.toLowerCase();

        for (size_t a = 0; a < NUM_ALIASES; a++) {
          if (lower == LOCATION_ALIASES[a].informal) {
            requested = LOCATION_ALIASES[a].canonical;
            Serial1.println(F("Alias matched -> using \"") + requested + "\"");
            break;
          }
        }

        bool looksLikeId = true;
        for (unsigned int c = 0; c < requested.length(); c++) {
          if (!isDigit(requested.charAt(c))) {
            looksLikeId = false;
            break;
          }
        }

        pendingLocation = requested;
        pendingWasById = looksLikeId;
        weatherUpdatePending = true;
        searchError = false;

        Serial1.println("Queued weather update for \"" + requested + "\" (treated as " + (looksLikeId ? "ID" : "name") + ")");
      }
    }
  }

  server.send(200, "text/html", completePage());
}

void WebServerManager::handleNotFound() {
  char errorMessage[150];
  snprintf(errorMessage, sizeof(errorMessage),
           "404: Not found.\n\nPlease use \"http://%s\" or \"http://%s.local\" to see the weather.\n         Thank you.\n                  -MiP",
           WiFi.localIP().toString().c_str(), HOSTNAME);

  server.send(404, "text/plain", errorMessage);
}

void WebServerManager::processPendingWeatherUpdate(bool& chestWritten, bool& solidUpdated, bool extinguished) {
  if (!weatherUpdatePending) return;
  weatherUpdatePending = false;

  bool success = pendingWasById 
    ? weather.updateWeatherById(pendingLocation) 
    : weather.updateWeatherByName(pendingLocation);

  if (success) {
    weather.saveLocation(pendingLocation);
    weather.activeLocation = pendingLocation;
    searchError = false;
    chestWritten = false;
    solidUpdated = false;
    Serial1.println("Weather updated for " + weather.data.cityName);
  } else {
    searchError = true;
    Serial1.println(F("Pending weather update failed."));
  }
}

String WebServerManager::completePage() {
  String htmlOutput = "<!DOCTYPE html>\n<html>\n";
  htmlOutput += htmlHead();
  htmlOutput += htmlBody();
  htmlOutput += "</html>\n";
  return htmlOutput;
}

String WebServerManager::htmlHead() {
  String head = "<head>\n";
  head += "<link rel=\"icon\" href=\"http://openweathermap.org/img/w/" + weather.data.icon + ".png\">\n";

  if (weatherUpdatePending) {
    head += "<meta http-equiv=\"refresh\" content=\"3\">\n";
  } else {
    head += "<meta http-equiv=\"refresh\" content=\"900\">\n";
  }
  head += " <meta charset=\"UTF-8\">\n";
  head += " <meta name=\"viewport\" content=\"user-scalable=no,width=device-width\" />\n";

  head += "<style>\n";
  head += "  body {background-color: #";
  
  if (weather.data.icon.indexOf("01") >= 0) {
    (weather.data.icon.indexOf('d') >= 0) ? head += "065ce5" : head += "04398e";
  } else if (weather.data.icon.indexOf("02") >= 0) {
    (weather.data.icon.indexOf('d') >= 0) ? head += "2b64bf" : head += "17376b";
  } else if (weather.data.icon.indexOf("03") >= 0) {
    (weather.data.icon.indexOf('d') >= 0) ? head += "3c6dbc" : head += "213e6d";
  } else if (weather.data.icon.indexOf("04") >= 0) {
    (weather.data.icon.indexOf('d') >= 0) ? head += "4c71ad" : head += "2f466d";
  } else if (weather.data.icon.indexOf("09") >= 0) {
    (weather.data.icon.indexOf('d') >= 0) ? head += "4c6284" : head += "384860";
  } else if (weather.data.icon.indexOf("10") >= 0) {
    (weather.data.icon.indexOf('d') >= 0) ? head += "43536b" : head += "313d4f";
  } else if (weather.data.icon.indexOf("11") >= 0) {
    (weather.data.icon.indexOf('d') >= 0) ? head += "485260" : head += "333a44";
  } else if (weather.data.icon.indexOf("13") >= 0) {
    (weather.data.icon.indexOf('d') >= 0) ? head += "f9fafc" : head += "666768";
  } else if (weather.data.icon.indexOf("50") >= 0) {
    (weather.data.icon.indexOf('d') >= 0) ? head += "bbbdc1" : head += "38393a";
  }
  head += ";}\n";

  head += "  .navbar {overflow: hidden; background-color: #333; position: fixed; top: 0; left: 0; width: 100%; height: 40px; margin: auto; }\n";
  head += "  .search-box,.close-icon,.search-wrapper {position: relative; padding: 5px 5px;}\n";
  head += "  .search-wrapper {width: 30%; float: left;}\n";
  head += "  .search-box {width: 240px; border: 1px solid #ccc; outline: 0; border-radius: 15px;}\n";
  head += "  .search-box:focus {box-shadow: 0 0 2px 2px #b0e0ee; border: 1px solid #bebede;}\n";
  head += "  .close-icon {border:1px solid transparent; background-color: transparent; display: inline-block; vertical-align: middle; outline: 0; cursor: pointer;}\n";
  head += "  .close-icon:after {content: \"X\"; display: block; width: 15px; height: 15px; position: absolute; background-color: #FA9595; z-index:1; right: 35px; top: 0; bottom: 0; margin: auto; padding: 2px; border-radius: 50%; text-align: center; color: white; font-weight: normal; font-size: 10px; box-shadow: 0 0 2px #E50F0F; cursor: pointer;}\n";
  head += "  .search-box:not(:valid) ~ .close-icon { display: none;}\n";
  if (searchError) {
    head += "  ::placeholder {color: red; opacity: 1;}\n";
  }

  head += "  .menuclock {font-family: Arial, Helvetica, sans-serif; color: #ffffff; float: right; padding: 10px 5px; margin-left: 30%;}\n";
  head += "  h1 {color: white; font-family: Arial, Helvetica, sans-serif; font-size: 200%; text-align: center; line-height: 5px;}\n";
  head += "  h2 {color: white; font-family: Arial, Helvetica, sans-serif; font-size: 300%; text-align: center; line-height: 5px;}\n";
  head += "  h3 {color: white; font-family: Arial, Helvetica, sans-serif; font-size: 110%; text-align: center; line-height: 5px;}\n";
  head += "  h4 {color: white; font-family: Arial, Helvetica, sans-serif; font-size: 100%; text-align: center; line-height: 5px;}\n";
  head += "  hr {border-top: 1px solid white;}\n";
  head += "  p {color: white; font-family: Arial, Helvetica, sans-serif;}\n";
  head += "  .weather {margin: 0 auto; max-width: 350px; margin-top: 50px; border-radius: 20px; background: rgba(0, 0, 0, .5); padding: 10px;}\n";
  head += "  canvas {padding-left: 0; padding-right: 0; margin-left: auto; margin-right: auto; display: block;}\n";
  head += "  footer {color: #d26c22; text-align: center;}\n";
  head += " </style>\n";

  head += "<title>" + weather.data.cityName + " Weather Conditions</title>\n";
  head += " </head>\n";

  return head;
}

String WebServerManager::htmlBody() {
  String body = "<body>\n";
  body += "<p/>\n";
  body += htmlMenuBar();

  if (weatherUpdatePending) {
    body += "<p style=\"color:#ffcc00;text-align:center;font-family:Arial,Helvetica,sans-serif;\">"
            "Updating weather for \""
            + pendingLocation + "\"&hellip; page will refresh automatically.</p>\n";
  } else if (searchError) {
    body += "<p style=\"color:#ff6666;text-align:center;font-family:Arial,Helvetica,sans-serif;\">"
            "Could not find that city. Please try another name or OpenWeatherMap city ID.</p>\n";
  }

  body += "<div class=\"weather\">\n";
  body += htmlHeader();
  body += htmlWeatherData();
  body += htmlFooter();
  body += "</div>\n";
  body += "<p/>\n";
  body += "</body>\n";

  return body;
}

String WebServerManager::htmlMenuBar() {
  String htmlMenuBar = "<div class=\"navbar\">\n";

  htmlMenuBar += "<div class=\"search-wrapper\">\n";
  htmlMenuBar += "  <form action=\"/\" method=\"post\">\n";
  htmlMenuBar += "    <input type=\"text\" name=\"city\" required class=\"search-box\" "
                 "placeholder=\"City,CC or City ID\" "
                 "title=\"Examples:&#10;• Montreal,CA&#10;• London,GB&#10;• 6077243 (city ID)&#10;• Naples\" "
                 "autocomplete=\"off\" "
                 "spellcheck=\"false\" />\n";
  htmlMenuBar += "    <button class=\"close-icon\" type=\"reset\"></button>\n";
  htmlMenuBar += "  </form>\n";
  htmlMenuBar += "</div>\n";

  htmlMenuBar += "<div id=\"clockbox\" class=\"menuclock\"></div>\n";
  htmlMenuBar += "<script type=\"text/javascript\">\n";
  htmlMenuBar += "var tday=[\"Sunday\",\"Monday\",\"Tuesday\",\"Wednesday\",\"Thursday\",\"Friday\",\"Saturday\"];\n";
  htmlMenuBar += "var tmonth=[\"January\",\"February\",\"March\",\"April\",\"May\",\"June\",\"July\",\"August\",\"September\",\"October\",\"November\",\"December\"];\n";

  htmlMenuBar += "function GetClock(){\n";
  htmlMenuBar += "var d=new Date();\n";
  htmlMenuBar += "var nday=d.getDay(),nmonth=d.getMonth(),ndate=d.getDate(),nyear=d.getFullYear();\n";
  htmlMenuBar += "var nhour=d.getHours(),nmin=d.getMinutes();\n";
  htmlMenuBar += "if(nmin<=9) nmin=\"0\"+nmin\n";

  htmlMenuBar += "var clocktext=\"\"+tday[nday]+\", \"+ndate+\" \"+tmonth[nmonth]+\", \"+nyear+\" \"+nhour+\":\"+nmin+\"\";\n";
  htmlMenuBar += "document.getElementById('clockbox').innerHTML=clocktext;\n";
  htmlMenuBar += "}\n";

  htmlMenuBar += "GetClock();\n";
  htmlMenuBar += "setInterval(GetClock,1000);\n";
  htmlMenuBar += "</script>\n";
  htmlMenuBar += "</div>\n";

  return htmlMenuBar;
}

String WebServerManager::htmlHeader() {
  String header = "<header>\n";
  header += "  <h1>" + weather.data.cityName + "</h1> \n";
  header += "<h4>" + weather.data.main + "</h4>\n";
  header += "  <h2>" + String(round(weather.data.temp)) + "&#176;</h2> \n";
  header += "</header>\n";

  return header;
}

String WebServerManager::htmlFooter() {
  String footer = "<footer>\n";
  footer += "  <a title=\"OpenWeatherMap\" href=\"https://openweathermap.org\"><img src=\"https://openweathermap.org/themes/openweathermap/assets/vendor/owm/img/logo_OpenWeatherMap_orange.svg\" alt=\"OpenWeatherMap logo\" height=\"20\"></a>\n";
  footer += "</footer>\n";

  return footer;
}

String WebServerManager::htmlWeatherData() {
  String htmlOutput = "<hr />";
  htmlOutput += "<h4>Weather for " + weather.data.cityName + ", " + weather.data.country + "</h4>\n";
  htmlOutput += "<p>\n";
  time_t time = weather.data.observationTime;
  htmlOutput += "Observation time: " + String(ctime(&time)) + "<br>\n";
  htmlOutput += "Description: " + weather.data.description + "<br>\n";
  htmlOutput += "IconMeteoCon: " + weather.data.iconMeteoCon + "<br>\n";
  htmlOutput += "Temperature: " + String(round(weather.data.temp)) + "&#176;<br>\n";
  htmlOutput += "Pressure: " + String(weather.data.pressure) + " hPa<br>\n";
  htmlOutput += "Humidity: " + String(weather.data.humidity) + "&#37;<br>\n";
  htmlOutput += "Temperature minimum: " + String(round(weather.data.tempMin)) + "&#176;<br>\n";
  htmlOutput += "Temperature maximum: " + String(round(weather.data.tempMax)) + "&#176;<br>\n";
  htmlOutput += "Wind speed: " + String(weather.data.windSpeed) + " mph<br>\n";
  htmlOutput += "Wind degrees: " + String(weather.data.windDeg) + "<br>\n";
  htmlOutput += "Clouds: " + String(weather.data.clouds) + "&#37;<br>\n";
  time = weather.data.sunrise;
  htmlOutput += "Sunrise: " + String(ctime(&time)) + "<br>\n";
  time = weather.data.sunset;
  htmlOutput += "Sunset: " + String(ctime(&time)) + "<br>\n";
  htmlOutput += "</p>\n";
  htmlOutput += "<hr />";

  htmlOutput += "<h3>MiP";
  if (mip.position.isUpright()) {
    htmlOutput += " is roaming</h3>\n";
    htmlOutput += chestHTML(0xB6, 0x00, 0xFF);
  } else if (extinguished) {
    htmlOutput += " is muted</h3>\n";
  } else if (!extinguished) {
    htmlOutput += "</h3>\n";
    htmlOutput += chestHTML(weather.red, weather.green, weather.blue);
  }
  htmlOutput += "<hr />";

  return htmlOutput;
}

String WebServerManager::chestHTML(const uint8_t redHTML, const uint8_t greenHTML, const uint8_t blueHTML) {
  String chestHTML = "<canvas id=\"imageView\" width=\"64\" height=\"64\"></canvas>\n";

  chestHTML += "<script type=\"text/javascript\">\n";
  chestHTML += "var canvas, context, canvaso, contexto;\n";
  chestHTML += "canvaso = document.getElementById('imageView');\n";
  chestHTML += "context = canvaso.getContext('2d');\n";

  chestHTML += "context.rect(0, 0, 64, 64);\n";
  chestHTML += "context.fillStyle=\"white\";\n";
  chestHTML += "context.fill();\n";

  chestHTML += "context.strokeStyle = '#a1a2a3';\n";
  chestHTML += "context.save();\n";
  chestHTML += "context.translate(32, 32);\n";
  chestHTML += "context.scale(0.6363636363636364, 1);\n";
  chestHTML += "context.beginPath();\n";
  chestHTML += "context.arc(0, 0, 37, 0, 6.283185307179586, false);\n";
  chestHTML += "context.fillStyle = '#";
  char rgbValue[7];
  snprintf(rgbValue, sizeof(rgbValue), "%02X%02X%02X", redHTML, greenHTML, blueHTML);
  chestHTML += rgbValue;
  chestHTML += "';\n";
  chestHTML += "context.fill();\n";
  chestHTML += "context.stroke();\n";
  chestHTML += "context.closePath();\n";
  chestHTML += "context.restore();\n";

  chestHTML += "context.strokeStyle = '#000000';\n";
  chestHTML += "context.beginPath();\n";
  chestHTML += "context.moveTo(0, 32);\n";
  chestHTML += "context.lineTo(9, 32);\n";
  chestHTML += "context.lineWidth=5;\n";
  chestHTML += "context.stroke();\n";
  chestHTML += "context.closePath();\n";

  chestHTML += "context.strokeStyle = '#000000';\n";
  chestHTML += "context.beginPath();\n";
  chestHTML += "context.moveTo(55, 32);\n";
  chestHTML += "context.lineTo(64, 32);\n";
  chestHTML += "context.stroke();\n";
  chestHTML += "context.closePath();\n";
  chestHTML += "</script>\n";

  return chestHTML;
}
