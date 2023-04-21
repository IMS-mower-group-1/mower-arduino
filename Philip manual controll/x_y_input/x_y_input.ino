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
}

void loop() {
  // Check if data is available from the mobile app
  int leftMotorSpeed = 0;
  int rightMotorSpeed = 0;
  if (Serial.available() > 0) {
    
    // Read angle and magnitude
    float angle = Serial.parseFloat();
    float magnitude = Serial.parseFloat();

    // Calculate motor speeds based on angle and magnitude
    float leftSpeed = magnitude * (cos(angle) + sin(angle));
    float rightSpeed = magnitude * (cos(angle) - sin(angle));

    // Scale motor speeds to the desired PWM range (e.g., -255 to 255)
    leftMotorSpeed = leftSpeed * 255;
    rightMotorSpeed = -rightSpeed * 255;
    Serial.print("\nLeft motor speed:");
    Serial.print(leftMotorSpeed);
    Serial.print("\nRight motor speed:");
    Serial.print(rightMotorSpeed);
    // Set motor speeds
    
  }
  Encoder_1.setTarPWM(leftMotorSpeed);
  Encoder_2.setTarPWM(rightMotorSpeed);

  // Update gyroscope and encoders
  gyro_0.update();
  Encoder_1.loop();
  Encoder_2.loop();
}