#if defined(ESP32)
  #include <WiFiMulti.h>
  WiFiMulti wifiMulti;
#elif defined(ESP8266)
  #include <ESP8266WiFiMulti.h>
  ESP8266WiFiMulti wifiMulti;
#endif


#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>
#include "EmonLib.h"   //https://github.com/openenergymonitor/EmonLib
#include "config.h"


InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN);
Point house_power_sensor("house_power");

EnergyMonitor emon;
float kWh = 0;
unsigned long lastmillis = millis();


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

  // Sensor configuration
  emon.voltage(35, vCalibration, 1.7); // Voltage: input pin, calibration, phase_shift
  emon.current(34, currCalibration); // Current: input pin, calibration.
}

void loop() {
  house_power_sensor.clearFields();

  // Read sensor values
  emon.calcVI(20, 2000);
  float voltage = emon.Vrms;
  float intensity = emon.Irms;
  float power = emon.apparentPower;

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
