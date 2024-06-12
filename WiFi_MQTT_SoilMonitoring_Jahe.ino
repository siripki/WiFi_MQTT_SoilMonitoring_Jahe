//Wifi & Mqtt Parameter
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
WiFiClient espClient;
PubSubClient mqtt(espClient);

const char *wifiSSID = "Nasgor";
const char *wifiPass = "ntahlahh";
const char *mqttTopic = "v2.0/pubs/APP6630af915dc3717900/DEV6630afa282d5e39604";
const char *mqttID = "DEV6630afa282d5e39604";
const char *mqttUser = "18f2e2e32e5542a2";
const char *mqttPass = "18f2e2e32e5d2efd";
const char *mqttBroker = "mqtt.telkomiot.id";
uint16_t mqttPort = 1883;
uint8_t uplinkInterval = 10; //interval dalam detik

//Konfigurasi pin
byte pinLed = 2;
byte pinPompa = 5; //D1
bool pompaState = false;

//DallasTemp Parameter
#include <OneWire.h>
#include <DallasTemperature.h>
#define dallasPin 4
OneWire oneWire(dallasPin);
DallasTemperature dallas(&oneWire);

//Data Variabel
int soilMoist;
float soilTemp;

void setup() {
  Serial.begin(115200);

  pinMode(pinLed, OUTPUT);
  //Pompa Setup
  pinMode(pinPompa, OUTPUT);
  //DallasTemp Setup
  dallas.begin();
  //wifi setup
  wifiSetup();
  //mqtt Setup
  mqttSetup();
}

void loop() {
  soilMoist = getMoisture();
  soilTemp = getTemp();
  if(soilMoist < 20 or soilTemp > 30) pompaState = 1;
  else pompaState = 0;
  digitalWrite(pinPompa, pompaState);

  static unsigned long prevPrint = 0;
  if (millis() - prevPrint > 500) {
    
    Serial.println("Soil Moisture    :" + String(soilMoist) + "%");
    Serial.println("Soil Temperature :" + String(soilTemp) + "°C");
    Serial.println();
    prevPrint = millis();
  }

  static unsigned long prevSend = 0;
  if (millis() - prevSend > uplinkInterval*1000) {
    String data = "{\"SoilMoist\":" + String(soilMoist) + ",\"SoilTemp\":" + String(soilTemp, 2) +"}";
    sendData_Mqtt(data);
    prevSend = millis();
  }
}

void wifiSetup() {
  //connecting to a WiFi network
  bool ledState = false;
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifiSSID);
  WiFi.begin(wifiSSID, wifiPass);
  while (WiFi.status() != WL_CONNECTED) {
    ledState = !ledState;
    digitalWrite(pinLed, ledState);
    Serial.print(".");
    delay(500);
  }
  digitalWrite(pinLed, 0);
  Serial.println("");
  Serial.print("WiFi connected - ESP IP address: ");
  Serial.println(WiFi.localIP());
}

//MQTT
void mqttSetup() {
  mqtt.setServer(mqttBroker, mqttPort);
  Serial.println("Status  : MQTT SetServer Finish");
}

void mqttConnect() {
  if (!mqtt.connected()) {
    Serial.println("Status  : Reconnecting to MQTT");
    mqtt.connect(mqttID, mqttUser, mqttPass); //menghubungkan ulang
    if (!mqtt.connected()) {
      mqtt.connect(mqttID, mqttUser, mqttPass); //menghubungkan ulang jika gagal
    }
  }
  Serial.println("Status  : Reconnected to MQTT");
}
void sendData_Mqtt(String msg) {
  mqttConnect();
  char finalDataChar[msg.length() + 1];
  msg.toCharArray(finalDataChar, msg.length() + 1);
  mqtt.publish(mqttTopic, finalDataChar);
  Serial.println("Send Data : " + String(finalDataChar));  //debug
}

uint8_t getMoisture() {
  uint16_t maxMoistADC = 293;
  uint16_t minMoistADC = 680;
  uint16_t adc = analogRead(A0);
  adc = constrain(adc, maxMoistADC, minMoistADC);
  uint8_t moisture = map(adc, minMoistADC, maxMoistADC, 0, 100);
  return moisture;
}

float getTemp() {
  float temp;
  dallas.setResolution(10);
  dallas.requestTemperatures();
  temp = dallas.getTempCByIndex(0);
  return temp;
}
