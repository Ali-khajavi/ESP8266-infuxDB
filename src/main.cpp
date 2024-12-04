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
#define DEVICE "2023"

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
    // Reading The Channel 1 J1
    // Power up the sensor
    pinMode(10, OUTPUT);
    digitalWrite(10, HIGH);
    // Configure flow sensor pin and interrupt
    pinMode(D1, INPUT);
    attachInterrupt(digitalPinToInterrupt(D1), pulseCounter, RISING);

    // Variables for flow rate calculation
    unsigned int totalPulseCount = 0;
    const unsigned int measurementDuration = 1000; // 1 seconds
    const unsigned int intervals = 3; // Break into smaller intervals for averaging
    Serial.println("Measuring flow rate...");
    for (unsigned int i = 0; i < intervals; i++) {
        pulseCount = 0;
        delay(measurementDuration / intervals);
        totalPulseCount += pulseCount; // Total pulse count in 1 seconds
    }
    // Calculate the average frequency (Hz) and flow rate (L/min)
    float averageFrequency_1 = totalPulseCount / (measurementDuration / 1000.0); // Pulses per second
    float flowRate_1 = averageFrequency_1 / CONVERSION_FACTOR; // Flow rate in L/min

    // Reading The Channel 2 J2
    // Configure flow sensor pin and interrupt
    pinMode(D2, INPUT);
    attachInterrupt(digitalPinToInterrupt(D2), pulseCounter, RISING);
    // Variables for flow rate calculation
    totalPulseCount = 0;
    Serial.println("Measuring flow rate...");
    for (unsigned int i = 0; i < intervals; i++) {
        pulseCount = 0;
        delay(measurementDuration / intervals);
        totalPulseCount += pulseCount; // Total pulse count in 1 seconds
    }

    float averageFrequency_2 = totalPulseCount / (measurementDuration / 1000.0); // Pulses per second
    float flowRate_2 = averageFrequency_2 / CONVERSION_FACTOR; // Flow rate in L/min

    Serial.print("Average Frequency Channel 1 (Hz): ");
    Serial.println(averageFrequency_1);
    Serial.print("Flow Rate (L/min): ");
    Serial.println(flowRate_1);

    Serial.print("Average Frequency Channel 2 (Hz): ");
    Serial.println(averageFrequency_2);
    Serial.print("Flow Rate (L/min): ");
    Serial.println(flowRate_2);

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

    // Send Channel 1 data to InfluxDB
    sensor.addTag("DEVICE", DEVICE);
    sensor.addField("flow_channel_1", flowRate_1);
    if (client.writePoint(sensor)) {
        Serial.println("Data successfully sent to InfluxDB.");
    } else {
        Serial.print("InfluxDB write failed: ");
        Serial.println(client.getLastErrorMessage());
    }

    // Send Channel 2 data to InfluxDB
    //sensor.addTag("DEVICE", DEVICE);
    sensor.addField("flow_channel_2", flowRate_2);
    if (client.writePoint(sensor)) {
        Serial.println("Data successfully sent to InfluxDB.");
    } else {
        Serial.print("InfluxDB write failed: ");
        Serial.println(client.getLastErrorMessage());
    }

    // Power down the sensor, disable WiFi, and enter deep sleep
    digitalWrite(10, LOW);
    WiFi.mode(WIFI_OFF);
    ESP.deepSleep(1800e6); // Deep sleep for 30 min (30e6 µs)
}

void loop() {
    // Empty loop since ESP will deep sleep after setup
}
