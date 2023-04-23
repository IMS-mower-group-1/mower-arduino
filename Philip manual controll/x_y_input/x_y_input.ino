#include <Arduino.h>
#include <Wire.h>
#include <SoftwareSerial.h>
#include <MeAuriga.h>
#include "dir.h"
#include <MeGyro.h>

MeGyro gyro_0(0, 0x69);
MeRGBLed rgbled_0(0, 12);
MeEncoderOnBoard Encoder_1(SLOT1);
MeEncoderOnBoard Encoder_2(SLOT2);
MeLightSensor lightsensor_12(12);

unsigned long lastInputTime;
const unsigned long inputTimeout = 500; //input timeout

void isr_process_encoder1(void)
{
  if(digitalRead(Encoder_1.getPortB()) == 0){
    Encoder_1.pulsePosMinus();
  }else{
    Encoder_1.pulsePosPlus();
  }
}
void isr_process_encoder2(void)
{
  if(digitalRead(Encoder_2.getPortB()) == 0){
    Encoder_2.pulsePosMinus();
  }else{
    Encoder_2.pulsePosPlus();
  }
}

void setup() {
  Serial.begin(9600);
  gyro_0.begin();
  rgbled_0.setpin(44);
  rgbled_0.fillPixelsBak(0, 2, 1);
  TCCR1A = _BV(WGM10);
  TCCR1B = _BV(CS11) | _BV(WGM12);
  TCCR2A = _BV(WGM21) | _BV(WGM20);
  TCCR2B = _BV(CS21);
  attachInterrupt(Encoder_1.getIntNum(), isr_process_encoder1, RISING);
  attachInterrupt(Encoder_2.getIntNum(), isr_process_encoder2, RISING);
  randomSeed((unsigned long)(lightsensor_12.read() * 123456));
  lastInputTime = millis();
}

void clearSerialBuffer() {
  while (Serial.available() > 0) {
    Serial.read();
  }
}

void calculate_wheel_speeds(float angle, int speed, int &leftWheelSpeed, int &rightWheelSpeed) {
  float rad_angle = angle + 3.14159; //Flip angle
  float vx = speed * sin(rad_angle);
  float vy = speed * cos(rad_angle);

  // Calculate wheel speeds based on the given angle
  leftWheelSpeed = (int)(vy - vx);
  rightWheelSpeed = (int)(vy + vx);
}


void loop() {
  if (Serial.available()) {
    lastInputTime = millis();

    // Read angle and magnitude
    float angle = Serial.parseFloat();
    float magnitude = Serial.parseFloat();

    // Clear the serial buffer to ensure we're always reading the latest data
    clearSerialBuffer();

    Serial.print(angle);
    Serial.print(",");
    Serial.println(magnitude);
    
    // Normalize the magnitude within the range of 0 to 255
    int speed = (int)(magnitude * 255);

    // Calculate wheel speeds based on the given angle
    int leftWheelSpeed, rightWheelSpeed;

    calculate_wheel_speeds(angle, speed, leftWheelSpeed, rightWheelSpeed);

    // Set wheel speeds
    Encoder_1.setTarPWM(-rightWheelSpeed);
    Encoder_2.setTarPWM(leftWheelSpeed);

  }

  if (millis() - lastInputTime > inputTimeout) {
    Encoder_1.setTarPWM(0);
    Encoder_2.setTarPWM(0);
  }

  // Update gyroscope and encoders
  gyro_0.update();
  Encoder_1.loop();
  Encoder_2.loop();
}