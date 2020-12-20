#include <Wire.h>
#include "mpu6500.h"

#define FRAME_SIZE    6000
#define CONNECT       0x01
#define DISCONNECT    0x00
const uint8_t ST_DATAFRAME[8] = {0x11, 0x11, 0x11, 0x11};
const uint8_t EN_DATAFRAME[8] = {0x00, 0x00, 0x00, 0x00};

PROGMEM uint8_t acc_buffer[FRAME_SIZE] = {0};
uint8_t device_stat = DISCONNECT;
unsigned long pre_timestamp = 0;
unsigned long cur_timestamp = pre_timestamp;

void setup() {
  // put your setup code here, to run once:
  Serial1.begin(750000);
  while (!Serial1);
  Serial.begin(115200);
  while(!Serial);
  Serial.println("Serial, Serial1 start");
  Wire.begin();
  Serial.println("Wire start");
  Mpu6500Config();
  Serial.println("MPU6500 start");
}

void loop() {
  // put your main code here, to run repeatedly:
  uint32_t avail_bytes = Serial1.available();
  //uint32_t rate = 0;
  if(avail_bytes > 0) {
    uint8_t buffer[avail_bytes];
    Serial1.readBytes(buffer, avail_bytes);
    if(avail_bytes = 1 && buffer[0] == 0x01) {
      device_stat = CONNECT;
      Serial.println("connect");
    }
    if(avail_bytes = 1 && buffer[0] == 0x00) {
      device_stat = DISCONNECT;
      Serial.println("disconnect");
    }
  }
  if(device_stat == CONNECT) {
    Mpu6500GetData(acc_buffer, FRAME_SIZE);
    cur_timestamp = millis();
    Serial.println(1000.0 / (cur_timestamp - pre_timestamp) * FRAME_SIZE/6);
    pre_timestamp = cur_timestamp;
    Serial1.write(ST_DATAFRAME, 4);    
    Serial1.write(acc_buffer, FRAME_SIZE);
    Serial1.write(EN_DATAFRAME, 4);
  }
}
