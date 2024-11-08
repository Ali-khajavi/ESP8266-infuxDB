#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>

// Define WiFi Connections
#define WIFI_SSID "Mr.Green"
#define WIFI_PASSWORD "alialiali"
#define WIFI_SSID1 "Pillipp-Mobil"
#define WIFI_PASSWORD1 "P!ll!pp#m0b1l"
#define WIFI_SSID0 "Monitoring"
#define WIFI_PASSWORD0 "Haustechnik24"

#define INFLUXDB_URL "http://172.105.78.144:8086"
#define INFLUXDB_TOKEN "QDp-QDPqKbNbVPjESvLcj7ons1LzjD9AUiEkYs_s5Nc8yRtTiiysH8eITv4hHqC87o7W7pT2_VW-B28MLb-QXQ=="
#define INFLUXDB_ORG "ec44f19893e990b6"
#define INFLUXDB_BUCKET "Sensors"
#define DEVICE "2024" 

InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);
Point sensor("Jürgen Pillipp");
ESP8266WiFiMulti wifi_multi;

const uint16_t CONNECT_TIMEOUT = 5000;
volatile unsigned int pulseCount = 0;

const float CONVERSION_FACTOR = 0.45;
const float ACCURACY_FACTOR = 0.03;

void IRAM_ATTR pulseCounter() {
    pulseCount++;
}

float calculateFlowRate(unsigned long frequency) {
    float maxFlow = (frequency * (1 + ACCURACY_FACTOR)) / CONVERSION_FACTOR;
    float minFlow = (frequency * (1 - ACCURACY_FACTOR)) / CONVERSION_FACTOR;
    return (maxFlow + minFlow) / 2.0;
}

void setup() {
    Serial.begin(115200);
    pinMode(10, OUTPUT); 
    digitalWrite(10, HIGH); // Power Up The Sensor

    pinMode(D1, INPUT);  // Read sensor channel 1 data
    attachInterrupt(digitalPinToInterrupt(D1), pulseCounter, RISING); // Set intrupt Registery active to be able to count digital puls.

    pulseCount = 0;  // Reset pulse count
    unsigned long startMillis = millis();

    // Count pulses for exactly 2 seconds
    while (millis() - startMillis < 2000) {
        // Waiting 2 seconds while counting pulses in the interrupt
    }

    // Calculate frequency from pulse count over 2-second interval
    float frequency = pulseCount / 2.0;
    float flowRate = calculateFlowRate(frequency);

    Serial.println("Connecting to WiFi...");
    wifi_multi.addAP(WIFI_SSID0, WIFI_PASSWORD0);
    wifi_multi.addAP(WIFI_SSID, WIFI_PASSWORD);
    wifi_multi.addAP(WIFI_SSID1, WIFI_PASSWORD1);

    while (wifi_multi.run(CONNECT_TIMEOUT) != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.print("Connected to WiFi: ");
        Serial.println(WiFi.SSID());

        // Send flow rate to InfluxDB
        sensor.addTag("DEVICE", DEVICE);
        sensor.addField("flow", flowRate);
        client.writePoint(sensor);
        
        Serial.print("Flow rate: ");
        Serial.println(flowRate);
        if (client.writePoint(sensor)) {
          Serial.println("Data successfully sent to InfluxDB");
        }
        else {
        Serial.print("InfluxDB write failed: ");
        Serial.println(client.getLastErrorMessage());
        }
      } 
      else {
        Serial.println("WiFi connection failed!");
      }

    // Power down sensor, disable WiFi, and put ESP to sleep
    digitalWrite(10, LOW);
    WiFi.mode(WIFI_OFF);
    ESP.deepSleep(30e6);
}

void loop() {
    // Empty loop since ESP will deep sleep after setup
}
