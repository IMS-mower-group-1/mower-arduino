#include <Arduino.h>
#include <Wire.h>
#include <SoftwareSerial.h>
#include <MeAuriga.h>

MeUltrasonicSensor ultrasonic_10(10);
MeEncoderOnBoard Encoder_1(SLOT1);
MeEncoderOnBoard Encoder_2(SLOT2);
MeLineFollower linefollower_9(9);
MeLightSensor lightsensor_12(12);

String state = "AUTO";

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
void set_wheel_speeds(float angle, int speed) {
  float rad_angle = angle + 3.14159; //Flip angle
  float vx = speed * sin(rad_angle);
  float vy = speed * cos(rad_angle);

  // Calculate wheel speeds based on the given angle
  Encoder_1.setTarPWM(-(int)(vy - vx));
  Encoder_2.setTarPWM((int)(vy + vx));
}

void _delay(float seconds) {
  if(seconds < 0.0){
    seconds = 0.0;
  }
  long endTime = millis() + seconds * 1000;
  while(millis() < endTime) _loop();
}

void setup() {
  TCCR1A = _BV(WGM10);
  TCCR1B = _BV(CS11) | _BV(WGM12);
  TCCR2A = _BV(WGM21) | _BV(WGM20);
  TCCR2B = _BV(CS21);
  attachInterrupt(Encoder_1.getIntNum(), isr_process_encoder1, RISING);
  attachInterrupt(Encoder_2.getIntNum(), isr_process_encoder2, RISING);
  
  randomSeed((unsigned long)(lightsensor_12.read() * 123456));

  while(1) {
    if(state == "AUTO"){
      handleAutonomousBehaviour();
    } else if (state == "MANUAL"){
      // TODO: manual control
    } else {
      // TODO: turn off
    }
    _loop();
  }
}

void handleAutonomousBehaviour(){
  int lineStatus = linefollower_9.readSensors();
  int distance = ultrasonic_10.distanceCm();
  int speed;

  // Check if the robot is about to cross the black line
  if(lineStatus == 0.000000 || lineStatus == 2.000000 || lineStatus == 1.000000){
    speed = random(120,131);
    // If near the black line, turn away from it
    if(lineStatus == 1.000000){
      set_wheel_speeds(0.0, speed);
      _delay(0.7);
      set_wheel_speeds(-1.57, speed);
      _delay(1);
    } else if(lineStatus == 2.000000){
      set_wheel_speeds(0.0, speed);
      _delay(0.7);
      set_wheel_speeds(1.57, speed);
      _delay(1);
    } else {
      set_wheel_speeds(0.0, speed);
      _delay(0.8);
      set_wheel_speeds(0.0, 0);
      set_wheel_speeds(random(0, 2) == 0 ? -1.57 : 1.57, speed);
      _delay(1);
    }
  } else if (distance < 10){
    speed = random(120,131);
    // If there is an obstacle within 10 centimeters, back off and turn away from it
    set_wheel_speeds(0.0, speed);
    _delay(1);
    set_wheel_speeds(random(0, 2) == 0 ? -1.57 : 1.57, speed);
    _delay(1);
  } else {
    // Otherwise, move forward
    set_wheel_speeds(3.14, random (130, 170));
  }
  _delay(0.1); // Small delay between direction updates
}

void _loop() {
  Encoder_1.loop();
  Encoder_2.loop();
}

void loop() {
  _loop();
}