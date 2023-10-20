#ifndef SENSOR_UPDATE_H
#define SENSOR_UPDATE_H

#include <Arduino.h>

#include "DHT.h"
#include <HTTPClient.h>

DHTesp dht;

const char *thingspeak_api_key = "JKLA2H9G3TV3K9NQ";
const char *thingspeak_url = "http://api.thingspeak.com/update";

const char *field_temperature = "&field1=";
const char *field_humidity = "&field2=";

struct dht_data {
  float temperature;
  float humidity;
};

dht_data _global_dht_data;

void dht_task_handler (void *pvParameters) {
  uint8_t total_data = 0;

  float temperature = 0;
  float humidity = 0;

    _global_dht_data.temperature = 0;
    _global_dht_data.humidity = 0;

  while (true) {
    if (total_data < 10) {
        temperature += dht.getTemperature();
        humidity += dht.getHumidity();

        total_data++;
    } else {
        _global_dht_data.temperature = temperature / total_data;
        _global_dht_data.humidity = humidity / total_data;

        // Serial.printf("[INFO] Temperature: %.2f\n", data.temperature);
        // Serial.printf("[INFO] Humidity: %.2f\n", data.humidity);

        temperature = 0;
        humidity = 0;
        total_data = 0;
    }

    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

// send http GET request to thingspeak
void dht_get_request_http (void *pvParameters) {
    while (1) {
        if (WiFi.status() == WL_CONNECTED) {
            if (_global_dht_data.temperature != 0 && _global_dht_data.humidity != 0) {
                HTTPClient http;

                String url = thingspeak_url;
                url += "?api_key=";
                url += thingspeak_api_key;
                url += field_temperature;
                url += String(dht.getTemperature());
                url += field_humidity;
                url += String(dht.getHumidity());

                http.begin(url);
                int httpCode = http.GET();

                if (httpCode > 0) {
                    Serial.printf("[INFO] HTTP GET request to %s\n", url.c_str());
                    Serial.printf("[INFO] HTTP GET response code: %d\n", httpCode);
                } else {
                    Serial.printf("[ERROR] HTTP GET request to %s\n", url.c_str());
                    Serial.printf("[ERROR] HTTP GET response code: %d\n", httpCode);
                }

                http.end();
            } else {
                Serial.println("[ERROR] HTTP GET request failed! Waiting for DHT22 data..");
            }
        }

        vTaskDelay(60000 / portTICK_PERIOD_MS);
    }
}

#endif // SENSOR_UPDATE_H