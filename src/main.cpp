#include <Arduino.h>
#include <Wire.h>
#include <SparkFun_VL6180X.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define VL6180X_ADDRESS 0x29
#define TOPIC_MASK "Pedro/Varg/Vl180x/%s/S2B"

VL6180xIdentification identification;
VL6180x sensor(VL6180X_ADDRESS);

WiFiClient espClient;
PubSubClient client(espClient);
const char* ssid = "cabo canaveral";
const char* password = "05151924";
const char* mqttServer = "mqtt.eclipse.org";
char topic[40];

/**
 * @brief Envia o valor da distância para o mqtt
 * 
 * @param distance 
 */
void sendMessage(uint8_t distance){
  char message[20];

  sprintf(
    message,
    "{"
        "\"d\":%d"
    "}",
    distance
  );

  client.publish(topic, message);
  Serial.println(message);

  delay(500);
}

/**
 * @brief Inicializa o sensor
 * 
 */
void sensorBegin(){
  Wire.begin(2,0);

  delay(100);

  sensor.getIdentification(&identification);

  if(sensor.VL6180xInit() != 0){
    Serial.println("FAILED TO INITALIZE");
  }; 

  sensor.VL6180xDefautSettings(); 
  
  delay(1000);
}

/**
 * @brief Inicializa o WiFi
 * 
 */
void wifiBegin(){
  WiFi.mode(WIFI_STA);
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
  uint8_t macChar[6];
  char macStr[18] = { 0 };
  wifi_get_macaddr(STATION_IF, macChar);
  printf(macStr, "%02x%02x%02x%02x%02x%02x", macChar[0], macChar[1], macChar[2], macChar[3], macChar[4], macChar[5]);
  String mac = String(macStr);

  randomSeed(micros());
  sprintf(
    topic,
    TOPIC_MASK,
    mac
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

  sendMessage(
    sensor.getDistance()
  );
}