#include <Arduino.h>
#include <Wire.h>
#include "VL6180X.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define VL6180X_ADDRESS 0x29
#define INTERVAL_LOG 10000
#define INTERVAL_DATA 50
#define DATA_TOPIC_MASK "Pedro/Vl180x/S2B/%s"
#define LOG_TOPIC_MASK "Pedro/Vl180x/LOG/%s"

VL6180X sensor;

WiFiClient espClient;
PubSubClient client(espClient);
const char* ssid = "cabo canaveral";
const char* password = "05151924";
const char* mqttServer = "mqtt.eclipse.org";
char dataTopic[40];
char logTopic[40];

void sendLog(unsigned long interval){
  static unsigned long timer = 0;
  if(millis() > timer + interval){
    timer += interval;
    char message[20];
    timer += interval;
    
    sprintf(
      message,
      "{"
          "\"p\":%i"
      "}",
      WiFi.RSSI()
    );

    client.publish(logTopic, message);
    Serial.println(message);
  }
}

/**
 * @brief Envia o valor da distância para o mqtt
 * 
 * @param distance 
 */
void sendDistance(uint8_t distance, unsigned long interval){
  static unsigned long timer = 0;
  if(millis() > timer + interval){
    char message[20];
    timer += interval;
    
    sprintf(
      message,
      "{"
          "\"d\":%d"
      "}",
      distance
    );

    client.publish(dataTopic, message);
    Serial.println(message);
  }
}

/**
 * @brief Inicializa o sensor
 * 
 */
void sensorBegin(){
  Wire.begin(2,0);
  
  sensor.init();
  sensor.configureDefault();
  sensor.setTimeout(500);
}

/**
 * @brief Inicializa o WiFi
 * 
 */
void wifiBegin(){
  WiFi.mode(WIFI_STA);
  WiFi.setOutputPower(10);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

/**
 * @brief Inicializa o Mqtt
 * 
 */
void mqttInit(){
  String mac = WiFi.macAddress();

  for (size_t i = 0; i < mac.length(); i++)
    if(mac[i] == ':') mac.remove(i, 1);

  mac.toLowerCase();  
  randomSeed(micros());
  sprintf(
    logTopic,
    LOG_TOPIC_MASK,
    mac.c_str()
  );
  sprintf(
    dataTopic,
    DATA_TOPIC_MASK,
    mac.c_str()
  );
  client.setServer(mqttServer, 1883);
}

/**
 * @brief Realiza reconexão em caso de queda
 * 
 */
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = WiFi.macAddress();
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

/**
 * @brief Realiza o setup da placa
 * 
 */
void setup() {
  Serial.begin(115200);
  sensorBegin();
  wifiBegin();
  mqttInit();
}

/**
 * @brief Executa as rotinas
 * 
 */
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  sendLog(INTERVAL_LOG);

  sendDistance(
    sensor.readRangeSingleMillimeters(),
    INTERVAL_DATA
  );

  if(sensor.timeoutOccurred()){
    client.publish(
      logTopic, 
      "{"
          "\"e\":\"vl6108x timeout\""
      "}"
    );
    Serial.println("timeout");
  }
}