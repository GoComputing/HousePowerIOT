#if defined(ESP32)
  #include <WiFiMulti.h>
  WiFiMulti wifiMulti;
#elif defined(ESP8266)
  #include <ESP8266WiFiMulti.h>
  ESP8266WiFiMulti wifiMulti;
#endif


#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>
#include "config.h"


#define V_SENSOR_PIN 35
#define C_SENSOR_PIN 34


InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN);
Point house_power_sensor("house_power");



void read_power(float &voltage, float &current) {
  uint16_t raw_voltage = 0;
  uint16_t smoothed_raw_voltage = analogRead(V_SENSOR_PIN);
  uint16_t min_smoothed_raw_voltage = 0xFFFF;
  uint16_t max_smoothed_raw_voltage = 0;
  float min_smoothed_voltage;
  float max_smoothed_voltage;

  uint16_t raw_current = 0;
  float smoothed_raw_current = analogRead(C_SENSOR_PIN);
  uint16_t min_smoothed_raw_current = 0xFFFF;
  uint16_t max_smoothed_raw_current = 0;
  float min_smoothed_current;
  float max_smoothed_current;

  float voltage_alpha = 0.5;
  float current_alpha = 0.99;
  uint32_t curr_millis = millis();
  uint32_t prev_millis = millis();

  while(curr_millis - prev_millis <= 5000) {

    // Read raw values
    raw_voltage = analogRead(V_SENSOR_PIN);
    raw_current = analogRead(C_SENSOR_PIN);

    // Smooth values
    smoothed_raw_voltage = (1-voltage_alpha)*raw_voltage + voltage_alpha*smoothed_raw_voltage;
    smoothed_raw_current = (1-current_alpha)*raw_current + current_alpha*smoothed_raw_current;

    if(curr_millis - prev_millis >= 500) {
      // Compute minimum/maximum
      if(smoothed_raw_voltage < min_smoothed_raw_voltage)
        min_smoothed_raw_voltage = smoothed_raw_voltage;
      if(smoothed_raw_voltage > max_smoothed_raw_voltage)
        max_smoothed_raw_voltage = smoothed_raw_voltage;
  
      if(smoothed_raw_current > max_smoothed_raw_current)
        max_smoothed_raw_current = smoothed_raw_current;
      if(smoothed_raw_current < min_smoothed_raw_current)
        min_smoothed_raw_current = smoothed_raw_current;
    }

    curr_millis = millis();
  }
  min_smoothed_voltage = (min_smoothed_raw_voltage * 3.3f) / 4096;
  max_smoothed_voltage = (max_smoothed_raw_voltage * 3.3f) / 4096;
  min_smoothed_current = (min_smoothed_raw_current * 3.3f) / 4096;
  max_smoothed_current = (max_smoothed_raw_current * 3.3f) / 4096;

  voltage = (max_smoothed_voltage - min_smoothed_voltage) * 114.623;
  current = (max_smoothed_current - min_smoothed_current) * 74.985 * 1.1339245735 - 0.71554617189;
  if(current < 0)
    current = 0;
}



void setup() {
  Serial.begin(115200);

  // Setup wifi
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);
  
  Serial.print("Connecting to wifi");
  while (wifiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();

  // Setup sensors
  house_power_sensor.addTag("location", SENSOR_LOCATION);

  // Time configuration
  timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");

  // Check InfluxDB connection status
  if (client.validateConnection()) {
    Serial.print("Connected to InfluxDB: ");
    Serial.println(client.getServerUrl());
  } else {
    Serial.print("InfluxDB connection failed: ");
    Serial.println(client.getLastErrorMessage());
  }
}

void loop() {
  house_power_sensor.clearFields();

  // Read sensor values
  float voltage;
  float intensity;
  read_power(voltage, intensity);
  float power = voltage * intensity;

  // Add values to InfluxDB point
  house_power_sensor.addField("voltage_V", voltage);
  house_power_sensor.addField("intensity_A", intensity);
  house_power_sensor.addField("power_W", power);

  // Log the read point
  Serial.println(client.pointToLineProtocol(house_power_sensor));

  // Push point to InfluxDB
  if (!client.writePoint(house_power_sensor)) {
    Serial.print("InfluxDB write failed: ");
    Serial.println(client.getLastErrorMessage());
  }

  delay(5000);
}
