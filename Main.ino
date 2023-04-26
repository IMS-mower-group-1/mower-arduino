#include <Arduino.h>
#include <Wire.h>
#include <SoftwareSerial.h>
#include <MeAuriga.h>
#include <MeGyro.h>

MeGyro gyro_0(0, 0x69);
MeRGBLed rgbled_0(0, 12);
MeEncoderOnBoard Encoder_1(SLOT1);
MeEncoderOnBoard Encoder_2(SLOT2);
MeUltrasonicSensor ultrasonic_10(10);
MeLineFollower linefollower_9(9);
MeLightSensor lightsensor_12(12);

float accumulatedX = 0.0;
float accumulatedY = 0.0;
const float wheel_radius = 0.045;

unsigned long lastInputTime;
const unsigned long inputTimeout = 500; //input timeout

static unsigned long last_print = 0;

String state = "AUTO";

enum AutonomousState {
  MOVE_FORWARD,
  TURN_AWAY_FROM_LINE,
  BACK_OFF_AND_TURN
};

AutonomousState autonomousState = MOVE_FORWARD;
unsigned long autonomousStateStartTime = 0;
int savedLineFollowerStatus = -1;

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

// Update position using dead reckoning
void update_position() {
  float x_angle = gyro_0.getAngle(3);
  if (x_angle < 0) {
    x_angle = 360 + x_angle;
  }
  x_angle = (x_angle * M_PI / 180);

  float rpm = (Encoder_1.getCurrentSpeed() - Encoder_2.getCurrentSpeed()) / 2.0;
  float angular_velocity = rpm * 2 * M_PI / 60;
  float velocity = angular_velocity * wheel_radius * 100 * (-1);
  float distance = velocity * 0.1; // Assuming 100 ms update interval

  float xDistance = distance * cos(x_angle);
  float yDistance = distance * sin(x_angle);

  accumulatedX += xDistance;
  accumulatedY += yDistance;
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
  while(1){
    if(state == "AUTO"){
      handleAutonomousBehaviour();
    } else if (state == "MANUAL"){
      handleManualBehaviour();
    } else {
      // TODO: turn off
    }
    
    update_position();
    if (millis() - last_print > 500) { // Print every 500 ms
      Serial.print("X: ");
      Serial.print(accumulatedX/100);
      Serial.print(", Y: ");
      Serial.println(accumulatedY/100);
      last_print = millis();
    }
  }
}

void clearSerialBuffer() {
  while (Serial.available() > 0) {
    Serial.read();
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
void handleManualBehaviour(){
  if (Serial.available()) {
    lastInputTime = millis();

    // Read angle and magnitude
    float angle = Serial.parseFloat();
    float magnitude = Serial.parseFloat();

    // Clear the serial buffer to ensure we're always reading the latest data
    clearSerialBuffer();

    // Normalize the magnitude within the range of 0 to 255
    int speed = (int)(magnitude * 255);

    // Calculate and set wheel speeds based on the given angle and speed
    set_wheel_speeds(angle, speed);
  }

  if (millis() - lastInputTime > inputTimeout) {
    Encoder_1.setTarPWM(0);
    Encoder_2.setTarPWM(0);
  }

  gyro_0.update();
  Encoder_1.loop();
  Encoder_2.loop();
}

void handleAutonomousBehaviour() {
  int lineStatus = linefollower_9.readSensors();
  int distance = ultrasonic_10.distanceCm();
  int speed = random(120, 131);

  unsigned long currentTime = millis();
  unsigned long elapsedTime = currentTime - autonomousStateStartTime;

  switch (autonomousState) {
    case MOVE_FORWARD:
      if (lineStatus == 1 || lineStatus == 2 || lineStatus == 0) {
        autonomousState = TURN_AWAY_FROM_LINE;
        autonomousStateStartTime = currentTime;
        savedLineFollowerStatus = lineStatus;
      } else if (distance < 10) {
        autonomousState = BACK_OFF_AND_TURN;
        autonomousStateStartTime = currentTime;
      } else {
        set_wheel_speeds(3.14, random(130, 170));
      }
      break;

    case TURN_AWAY_FROM_LINE:
      if (elapsedTime < 700) {
        set_wheel_speeds(0.0, speed);
      } else if (elapsedTime < 1700) {
        if (savedLineFollowerStatus == 1) {
          set_wheel_speeds(-1.57, speed);
        } else if (savedLineFollowerStatus == 2) {
          set_wheel_speeds(1.57, speed);
        } else {
          set_wheel_speeds(random(0, 2) == 0 ? -1.57 : 1.57, speed);
        }
      } else {
        autonomousState = MOVE_FORWARD;
      }
      break;

    case BACK_OFF_AND_TURN:
      if (elapsedTime < 1000) {
        set_wheel_speeds(0.0, speed);
      } else if (elapsedTime < 2000) {
        set_wheel_speeds(random(0, 2) == 0 ? -1.57 : 1.57, speed);
      } else {
        autonomousState = MOVE_FORWARD;
      }
      break;
  }

  gyro_0.update();
  Encoder_1.loop();
  Encoder_2.loop();
}



void _loop(){
  Encoder_1.loop();
  Encoder_2.loop();
}
void loop() {
  _loop();
}