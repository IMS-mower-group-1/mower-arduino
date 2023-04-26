#include <Arduino.h>
#include <Wire.h>
#include <SoftwareSerial.h>
#include <MeAuriga.h>
#include <MeGyro.h>

MeGyro gyro_0(0, 0x69);
MeRGBLed rgbled_0(0, 12);
MeEncoderOnBoard Encoder_1(SLOT1);
MeEncoderOnBoard Encoder_2(SLOT2);
MeLightSensor lightsensor_12(12);

float accumulatedX = 0.0;
float accumulatedY = 0.0;
const float wheel_radius = 0.045;

unsigned long lastInputTime;
const unsigned long inputTimeout = 500; //input timeout

static unsigned long last_print = 0;

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


  if (millis() - last_print > 1000) { // Print every 1000 ms (1 second)
    Serial.print("X: ");
    Serial.print(accumulatedX/100);
    Serial.print(", Y: ");
    Serial.println(accumulatedY/100);
    last_print = millis();
  }
    /*
    Serial.print(angle);
    Serial.print(",");
    Serial.println(magnitude);
    */
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

  // Update position using dead reckoning
  update_position();
}