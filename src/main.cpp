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

volatile unsigned int pulseCount = 0; // Count of pulses from the sensor
const float CONVERSION_FACTOR = 0.45; // Flow sensor conversion factor

void IRAM_ATTR pulseCounter() {
    pulseCount++;
}

void setup() {
    Serial.begin(115200);

    // Power up the sensor
    pinMode(10, OUTPUT);
    digitalWrite(10, HIGH);

    // Configure flow sensor pin and interrupt
    pinMode(D1, INPUT);
    attachInterrupt(digitalPinToInterrupt(D1), pulseCounter, RISING);

    // Variables for flow rate calculation
    unsigned int totalPulseCount = 0;
    const unsigned int measurementDuration = 3000; // 3 seconds
    const unsigned int intervals = 3; // Break into smaller intervals for averaging

    Serial.println("Measuring flow rate...");
    for (unsigned int i = 0; i < intervals; i++) {
        pulseCount = 0;
        delay(measurementDuration / intervals);
        totalPulseCount += pulseCount;
    }

    // Calculate the average frequency (Hz) and flow rate (L/min)
    float averageFrequency = totalPulseCount / (measurementDuration / 1000.0); // Pulses per second
    float flowRate = averageFrequency / CONVERSION_FACTOR; // Flow rate in L/min

    Serial.print("Average Frequency (Hz): ");
    Serial.println(averageFrequency);
    Serial.print("Flow Rate (L/min): ");
    Serial.println(flowRate);

    // Connect to WiFi
    Serial.println("Connecting to WiFi...");
    wifi_multi.addAP(WIFI_SSID0, WIFI_PASSWORD0);
    wifi_multi.addAP(WIFI_SSID, WIFI_PASSWORD);
    wifi_multi.addAP(WIFI_SSID1, WIFI_PASSWORD1);

    while (wifi_multi.run() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
    }

    Serial.println("\nConnected to WiFi!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    // Send data to InfluxDB
    sensor.addTag("DEVICE", DEVICE);
    sensor.addField("flow", flowRate);

    if (client.writePoint(sensor)) {
        Serial.println("Data successfully sent to InfluxDB.");
    } else {
        Serial.print("InfluxDB write failed: ");
        Serial.println(client.getLastErrorMessage());
    }

    // Power down the sensor, disable WiFi, and enter deep sleep
    digitalWrite(10, LOW);
    WiFi.mode(WIFI_OFF);
    ESP.deepSleep(30e6); // Deep sleep for 30 seconds (30e6 µs)
}

void loop() {
    // Empty loop since ESP will deep sleep after setup
}
