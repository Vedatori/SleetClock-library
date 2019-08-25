#include "json-streaming-parser/JsonStreamingParser.h"
#include "json-streaming-parser/JsonListener.h"
#include <time.h>
#include "SleetClock.h"
#include "DarkskyParser.h"

void DarkskyParser::whitespace(char c) {
}

void DarkskyParser::startDocument() {
}

void DarkskyParser::endDocument() {
  for (int i = 0; i<DS_NUMBER_OF_HOURLY_DATA; i++)
    weatherData[i].weather = darkskyWeatherToIcon(weatherData[i].weather, weatherData[i].precipIntensity);
}

void DarkskyParser::key(String key) {
  switch (status) {
    case S0:
      if (key == "currently") {
        status = CURRENTLY;
        subStatus = NONE;
      }
      break;
    case CURRENTLY:
      setSubStatus(key);
      if (key == "hourly") {
        status = HOURLY;
        subStatus = NONE;
      }
    case HOURLY:
      if (key == "data") {
        status = HOURLYDATA;
        subStatus = NONE;
      }
      break;
    case HOURLYDATA:
      setSubStatus(key);
      break;
    default:
      break;
  }
}

void DarkskyParser::value(String value) {
  switch (status) {
    case CURRENTLY:
    case HOURLYDATA:
      setData(value);
      break;
    default:
      break;
  }
}

void DarkskyParser::startArray() {
}

void DarkskyParser::endArray() {
}

void DarkskyParser::startObject() {
}

void DarkskyParser::endObject() {
  switch (status) {
    case CURRENTLY:
    case HOURLYDATA:
      counter++;
      break;
    default:
      break;
  }
}

void DarkskyParser::begin(const char *apiKey, const char *latitude, const char *longitude) {
  this->apiKey = strdup(apiKey);
  this->latitude = strdup(latitude);
  this->longitude = strdup(longitude);
  parser.setListener(this);
}

void DarkskyParser::reset() {
  status = S0;
  subStatus = NONE;
  counter = 0;
}

void DarkskyParser::getData() {
  int c;

  parser.reset();
  reset();

  client.setCACert(rootCA);
  if (client.connect(server, 443) == 1) {
    client.print("GET ");
    client.print(url);
    client.print(apiKey);
    client.print("/");
    client.print(latitude);
    client.print(",");
    client.print(longitude);
    client.print(options);
    client.println(" HTTP/1.1");
    client.print("Host: ");
    client.println(server);
    client.println("Connection: close");
    client.println();
    client.flush();

    skipHeaders();
    while ((c = getChar()) != EOF) {
      parser.parse(c);
    }
    client.stop();
  }
}

void DarkskyParser::skipHeaders() {
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      break;
    }
  }
}

int DarkskyParser::getChar() {
  if (client.connected()) {
    if (client.available()) {
      return client.read();
    } else {
      return 0;
    }
  } else {
    return EOF;
  }
}

void DarkskyParser::setSubStatus(String key) {
  if (key == "time") {
    subStatus = TIME;
  } else if (key == "icon") {
    subStatus = ICON;
  } else if (key == "precipIntensity") {
    subStatus = PRECIPINTENSITY;
  } else if (key == "precipProbability") {
    subStatus = PRECIPPROBABILITY;
  } else if (key == "temperature") {
    subStatus = TEMPERATURE;
  } else if (key == "humidity") {
    subStatus = HUMIDITY;
  } else if (key == "pressure") {
    subStatus = PRESSURE;
  } else {
    subStatus = NONE;
  }
}

void DarkskyParser::setData(String value) {
  struct tm *currentTime;

  switch (status) {
    case CURRENTLY:
    case HOURLYDATA:
      if (counter < DS_NUMBER_OF_HOURLY_DATA + 1) {
        switch (subStatus) {
          case TIME:
            if (status == CURRENTLY) {
              lastUpdate = value.toInt();
              currentTime = localtime(&lastUpdate);
              currentHour = currentTime->tm_hour % 12;
            };
            break;
          case ICON:
            weatherData[counter].weather = 10;
            for (int i = 0; i < 10; i++) {
              if (value == weatherName[i]) {
                weatherData[counter].weather = i;
              }
            }
            break;
          case PRECIPINTENSITY:
            weatherData[counter].precipIntensity = value.toFloat();
            break;
          case PRECIPPROBABILITY:
            weatherData[counter].precipProbability = value.toFloat() * 100;
            break;
          case TEMPERATURE:
            weatherData[counter].temperature = value.toFloat();
            break;
          case HUMIDITY:
            weatherData[counter].humidity = value.toFloat() * 100;
            break;
          case PRESSURE:
            weatherData[counter].pressure = value.toFloat();
            break;
          default:
            break;
        }
      }
  }
}

/* mapping from darksky weather to this clock weather */
/* rain is split into usual rain and heavy rain */
int DarkskyParser::darkskyWeatherToIcon(int weather, int precipIntensity) {
  switch (weather) {
    case 0: /* clear-day */
      return (int)CLEAR_DAY;
      break;
    case 1: /* clear-night */
      return (int)CLEAR_NIGHT;
      break;
    case 2: /* rain */
      if (precipIntensity < 5) {
        return (int)RAIN;
      } else {
        return (int)HEAVY_RAIN;
      }
      break;
    case 3: /* snow */
      return (int)SNOW;
      break;
    case 4: /* sleet */
      return (int)SLEET;
      break;
    case 5:
      return (int)WIND;
      break;
    case 6:
      return (int)FOG;
      break;
    case 7: /* cloudy */
      return (int)CLOUDY;
      break;
    case 8: /* partly-cloudy-day */
      return (int)PARTLY_CLOUDY_DAY;
      break;
    case 9: /* partly-cloudy-night */
      return (int)PARTLY_CLOUDY_NIGHT;
      break;
    default:
      return (int)OTHER;
      break;
  }
}