#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>


// Define Wifi Connections
#define WIFI_SSID "Mr.Green"
#define WIFI_PASSWORD "alialiali"
#define WIFI_SSID1 "Pillipp-Mobil"
#define WIFI_PASSWORD1 "P!ll!pp#m0b1l"
#define WIFI_SSID0 "Monitoring"
#define WIFI_PASSWORD0 "Haustechnik24"

#define INFLUXDB_URL "http://172.104.132.172:8086"
#define INFLUXDB_TOKEN "as-RsgomyvthozUA1YzzUji5uwjRBcRZgB5xLkYmQmTTZ8mdSDiziVfzxqjTUqKPND27P1jCrnwvjGTm6CsXlg=="
#define INFLUXDB_ORG "9658415ef121eada"
#define INFLUXDB_BUCKET "Sensors"

#define DEVICE "ESP8266" 

//#define TZ_INFO "WET0WEST,M3.5.0/1,M10.5.0"

InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);

Point sensor("Flow_Rate");

ESP8266WiFiMulti wifi_multi;

uint16_t connectTimeOutPerAP=5000;



volatile unsigned int pulseCount = 0;


float flow_Rate = 0;
const float conversionFactor = 0.45;
const float accuracyFactor = 0.03;


bool start_flag = 0;


long currentMillis = 0;
long previousMillis = 0;
int interval = 1000;
float calibrationFactor = 0.45;
byte pulse1Sec = 0;
float flowRate;
unsigned long flowMilliLitres;
unsigned int totalMilliLitres;
float flowLitres;
float totalLitres;


float calculationFlow(unsigned long frequency){

  float maxFlow = (frequency * (1 + accuracyFactor)) / conversionFactor;
  float minFlow = (frequency * (1 - accuracyFactor)) / conversionFactor;
  Serial.println("");
  Serial.print("This is the pulse : ");
  Serial.println(frequency);
  return (maxFlow + minFlow) / 2.0;
}

void IRAM_ATTR  pulseCounter(){
  pulseCount++;
}

void setup() {
  // power sensor on
  pinMode(D6, OUTPUT);
  digitalWrite(D6, HIGH);

  // read sensor data
  pinMode(D5, INPUT);
  attachInterrupt(digitalPinToInterrupt(D5), pulseCounter , RISING);
  delay(80);

  // calculate pulse in 1 second
  previousMillis = millis();
  while(currentMillis <  previousMillis + 3000 ){
    currentMillis = millis();
    pulse1Sec = pulseCount;
  }

  Serial.begin(115200);

  //flow_Rate = calculationFlow(pulseCount);
  
  wifi_multi.addAP(WIFI_SSID0,WIFI_PASSWORD0);
  wifi_multi.addAP(WIFI_SSID,WIFI_PASSWORD);
  wifi_multi.addAP(WIFI_SSID1,WIFI_PASSWORD1);
  
  
  Serial.println("");
  Serial.print("number of pulse is :");
  Serial.println(pulseCount);
  Serial.println("Connecting to WiFi....");

  int i = 0;
  while(wifi_multi.run(connectTimeOutPerAP)!=WL_CONNECTED)
  {
    delay(500);
    Serial.println(i);
    i++;
  }

  delay(1000);

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.print("Wifi Connected to :");
    Serial.println(WiFi.SSID());
  }else{
    Serial.println("Wifi connection problem!!!!!!!!");
  }

  flowRate = ((1000.0 / (currentMillis - previousMillis)) * pulse1Sec) / calibrationFactor;


  Serial.print("flow : ");
  Serial.println(flowRate);

  sensor.addTag("device", DEVICE);
  //timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");
  sensor.addField("flow", flowRate);
  client.writePoint(sensor);
  sensor.clearFields();
  digitalWrite(D6, LOW);
  WiFi.mode(WIFI_OFF);
  Serial.println("Good Night for 30 Seconds");
  ESP.deepSleep(30e6);
}

void loop() {
  
}
