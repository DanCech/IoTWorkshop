#include <Arduino.h>
#include <WiFi.h>
#include <NTPClient.h>
#include <DHT.h>
#include <HTTPClient.h>

#include "config.h"

// NTP Client
WiFiUDP ntpUDP;
NTPClient ntpClient(ntpUDP);

// DHT Sensor
DHT dht(DHTPIN, DHTTYPE);

// Hosted Metrics Graphite Client
HTTPClient httpGraphite;

// Carbon UDP Client
WiFiUDP carbonUdp;

/*
  Function to set up the connection to the WiFi AP
*/
void setupWiFi() {
  Serial.print("Connecting to '");
  Serial.print(WIFI_SSID);
  Serial.print("' ...");

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("connected");

  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  randomSeed(micros());
}

/*
  Function to format a unix timestamp as a human-readable time
*/
String formatTime(unsigned long rawTime) {
  unsigned long localTime = rawTime + (TZ_OFFSET * 3600);

  unsigned long hours = (localTime % 86400L) / 3600;
  String hoursStr = hours < 10 ? "0" + String(hours) : String(hours);

  unsigned long minutes = (localTime % 3600) / 60;
  String minuteStr = minutes < 10 ? "0" + String(minutes) : String(minutes);

  unsigned long seconds = localTime % 60;
  String secondStr = seconds < 10 ? "0" + String(seconds) : String(seconds);

  return hoursStr + ":" + minuteStr + ":" + secondStr;
}

/*
 * Function to submit metrics to hosted Graphite
 */
void submitHostedGraphite(unsigned long ts, float c, float f, float h, float hic, float hif) {
  // build hosted metrics json payload
  String body = String("[") +
    "{\"name\":\"sensor." + ID + ".temp_c\",\"interval\":" + INTERVAL + ",\"value\":" + c + ",\"mtype\":\"gauge\",\"time\":" + ts + "}," +
    "{\"name\":\"sensor." + ID + ".temp_f\",\"interval\":" + INTERVAL + ",\"value\":" + f + ",\"mtype\":\"gauge\",\"time\":" + ts + "}," +
    "{\"name\":\"sensor." + ID + ".humidity\",\"interval\":" + INTERVAL + ",\"value\":" + h + ",\"mtype\":\"gauge\",\"time\":" + ts + "}," +
    "{\"name\":\"sensor." + ID + ".heat_index_c\",\"interval\":" + INTERVAL + ",\"value\":" + hic + ",\"mtype\":\"gauge\",\"time\":" + ts + "}," +
    "{\"name\":\"sensor." + ID + ".heat_index_f\",\"interval\":" + INTERVAL + ",\"value\":" + hif + ",\"mtype\":\"gauge\",\"time\":" + ts + "}]";

  // Serial.println(body);

  // submit POST request via HTTP
  httpGraphite.begin(String("https://") + HM_GRAPHITE_HOST + "/metrics", HM_ROOT_CA);
  httpGraphite.setAuthorization(HM_GRAPHITE_INSTANCE, HM_API_KEY);
  httpGraphite.addHeader("Content-Type", "application/json");

  int httpCode = httpGraphite.POST(body);
  if (httpCode > 0) {
    Serial.printf("Graphite [HTTPS] POST...  Code: %d  Response: ", httpCode);
    httpGraphite.writeToStream(&Serial);
    Serial.println();
  } else {
    Serial.printf("Graphite [HTTPS] POST... Error: %s\n", httpGraphite.errorToString(httpCode).c_str());
  }

  httpGraphite.end();
}

/*
 * Function to submit metrics via carbon protocol
 */
void submitCarbon(unsigned long ts, float c, float f, float h, float hic, float hif) {
  // build hosted metrics json payload
  String body = String("sensor.") + ID + ".temp_c " + c + " " + ts + "\n" +
    "sensor." + ID + ".temp_f " + f + " " + ts + "\n" +
    "sensor." + ID + ".humidity " + h + " " + ts + "\n" +
    "sensor." + ID + ".heat_index_c " + hic + " " + ts + "\n" +
    "sensor." + ID + ".heat_index_f " + hif + " " + ts + "\n";

  Serial.println("Sending to carbon");
  // Serial.println(body);

  carbonUdp.beginPacket(CARBON_HOST, CARBON_PORT);
  carbonUdp.print(body);
  carbonUdp.endPacket();
}

/*
  Function called at boot to initialize the system
*/
void setup() {
  // start the serial output at 115,200 baud
  Serial.begin(115200);

  // connect to WiFi
  setupWiFi();

  // Initialize a NTPClient to get time
  ntpClient.begin();

  // start the DHT sensor
  dht.begin();
}

/*
  Function called in a loop to read temp/humidity and submit them to hosted metrics
*/
void loop() {
  // reconnect to WiFi if required
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.disconnect();
    yield();
    setupWiFi();
  }

  // update time via NTP if required
  while(!ntpClient.update()) {
    yield();
    ntpClient.forceUpdate();
  }

  // get current timestamp
  unsigned long ts = ntpClient.getEpochTime();

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)

  // Read humidity
  float h = dht.readHumidity();
  yield();

  // Read temperature as Celsius (the default)
  float c = dht.readTemperature();
  yield();

  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);
  yield();

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(c) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(c, h, false);

  // output readings on Serial connection
  Serial.println(
    formatTime(ts) +
    "  Humidity: " + h + "%" +
    "  Temperature: " + c + "°C " + f + "°F" +
    "  Heat index: " + hic + "°C " + hif + "°F"
  );

  if (HM_GRAPHITE_INSTANCE != "<instance id>") {
    yield();
    submitHostedGraphite(ts, c, f, h, hic, hif);
  }

  submitCarbon(ts, c, f, h, hic, hif);

  // wait 30s, then do it again
  delay(30 * 1000);
}
