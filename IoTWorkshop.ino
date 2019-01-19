#include <Arduino.h>

#include <WiFi.h>

#include <HTTPClient.h>
#include <NTPClient.h>

#include <DHT.h>

#include "config.h"

#define USE_SERIAL Serial

HTTPClient http;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

DHT dht(DHTPIN, DHTTYPE);

///////////////////////////////////////////////////////////////////////////
//   WiFi
///////////////////////////////////////////////////////////////////////////

/*
   Function called to setup the connection to the WiFi AP
*/
void setupWiFi() {
  Serial.print("Connecting to ");
  Serial.print(WIFI_SSID);
  Serial.print("...");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("connected");

  // Print local IP address
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  randomSeed(micros());
}

void setup() {
  USE_SERIAL.begin(115200);

  USE_SERIAL.println();
  USE_SERIAL.println();
  USE_SERIAL.println();

  for(uint8_t t = 4; t > 0; t--) {
    USE_SERIAL.printf("[SETUP] WAIT %d...\n", t);
    USE_SERIAL.flush();
    delay(1000);
  }

  setupWiFi();

  // allow reuse (if server supports it)
  http.setReuse(true);
  
  // Initialize a NTPClient to get time
  timeClient.begin();
  
  dht.begin();
}

void loop() {
  // update time
  while(!timeClient.update()) {
    yield();
    timeClient.forceUpdate();
  }

  yield();
  
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print(F("°C "));
  Serial.print(f);
  Serial.print(F("°F  Heat index: "));
  Serial.print(hic);
  Serial.print(F("°C "));
  Serial.print(hif);
  Serial.println(F("°F"));

  unsigned long ts = timeClient.getEpochTime();
  
  String body = String("[") +
  "{\"name\":\"sensor." + ID + ".temp_c\",\"interval\":" + INTERVAL + ",\"value\":" + t + ",\"mtype\":\"gauge\",\"time\":" + ts + "}," +
  "{\"name\":\"sensor." + ID + ".temp_f\",\"interval\":" + INTERVAL + ",\"value\":" + f + ",\"mtype\":\"gauge\",\"time\":" + ts + "}," +
  "{\"name\":\"sensor." + ID + ".humidity\",\"interval\":" + INTERVAL + ",\"value\":" + h + ",\"mtype\":\"gauge\",\"time\":" + ts + "}," +
  "{\"name\":\"sensor." + ID + ".heat_index_c\",\"interval\":" + INTERVAL + ",\"value\":" + hic + ",\"mtype\":\"gauge\",\"time\":" + ts + "}," +
  "{\"name\":\"sensor." + ID + ".heat_index_f\",\"interval\":" + INTERVAL + ",\"value\":" + hif + ",\"mtype\":\"gauge\",\"time\":" + ts + "}]";
  
  http.begin(HM_HOST, 80, "/metrics");
  http.setAuthorization(HM_INSTANCE, HM_API_KEY);
  http.addHeader("Content-Type", "application/json");

  int httpCode = http.POST(body);
  if(httpCode > 0) {
    USE_SERIAL.printf("[HTTP] POST... code: %d\n", httpCode);
    http.writeToStream(&USE_SERIAL);
    USE_SERIAL.println();
  } else {
    USE_SERIAL.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }
  
  http.end();

  delay(1000);
}



