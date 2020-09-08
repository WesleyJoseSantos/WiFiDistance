#include <Arduino.h>
#include <Wire.h>
#include "VL6180X.h"

#define INTERVAL_DATA 10
#define PIN_FREQ 4
#define DIST_MIN 0
#define DIST_MAX 200
#define FREQ_MIN 33
#define FREQ_MAX 1023

VL6180X sensor;

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

    Serial.println(message);
    uint16_t freq = map(distance, 0, 255, 33, 1023);
    tone(PIN_FREQ, freq);
  }
}

/**
 * @brief Inicializa o sensor
 * 
 */
void sensorBegin(){
  Wire.begin();
  
  sensor.init();
  sensor.configureDefault();
  sensor.setTimeout(500);
}

/**
 * @brief Realiza o setup da placa
 * 
 */
void setup() {
  Serial.begin(115200);
  sensorBegin();
  pinMode(PIN_FREQ, OUTPUT);
}

/**
 * @brief Executa as rotinas
 * 
 */
void loop() {
  sendDistance(
    sensor.readRangeSingleMillimeters(),
    INTERVAL_DATA
  );

  if(sensor.timeoutOccurred()){
    Serial.println("timeout");
  }
}