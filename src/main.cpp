/** Arduino Libs **/
#include <Arduino.h>

#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif
/** Arduino Libs **/

/** Custom Libs **/
#include "Sensor_Update.h"
#include "UNS_WiFi_Setup.h"

void setup () {
  Serial.begin(115200);
  delay(1000);

  dht.setup(32, DHTesp::DHT22);
  Serial.println("[INFO] DHT22 setup done!");

  if (!SPIFFS.begin(true)) {
    Serial.println("[ERROR] An Error has occurred while mounting SPIFFS");
    return;
  } else {
    Serial.println("[INFO] SPIFFS mounted successfully");
  }

  uns_alternate_connection();

  server_start();

  xTaskCreatePinnedToCore(dht_task_handler, "dht0", 2048, NULL, 1, NULL, app_cpu);
  xTaskCreatePinnedToCore(dht_get_request_http, "dht_thingspeak0", 2048, NULL, 1, NULL, app_cpu);
}

void loop () { server.handleClient(); }
