#include <Arduino.h>
#include <Wire.h>
#include <SoftwareSerial.h>
#include <MeAuriga.h>

MeUltrasonicSensor ultrasonic_10(10);
MeEncoderOnBoard Encoder_1(SLOT1);
MeEncoderOnBoard Encoder_2(SLOT2);
MeLineFollower linefollower_9(9);
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
void move(int direction, int speed)
{
  int leftSpeed = 0;
  int rightSpeed = 0;
  if(direction == 1){
    leftSpeed = -speed;
    rightSpeed = speed;
  }else if(direction == 2){
    leftSpeed = speed;
    rightSpeed = -speed;
  }else if(direction == 3){
    leftSpeed = -speed;
    rightSpeed = -speed;
  }else if(direction == 4){
    leftSpeed = speed;
    rightSpeed = speed;
  }
  Encoder_1.setTarPWM(leftSpeed);
  Encoder_2.setTarPWM(rightSpeed);
}

void _delay(float seconds) {
  if(seconds < 0.0){
    seconds = 0.0;
  }
  long endTime = millis() + seconds * 1000;
  while(millis() < endTime) _loop();
}


void _loop() {
  Encoder_1.loop();
  Encoder_2.loop();
}

void setup() {
  TCCR1A = _BV(WGM10);
  TCCR1B = _BV(CS11) | _BV(WGM12);
  TCCR2A = _BV(WGM21) | _BV(WGM20);
  TCCR2B = _BV(CS21);
  attachInterrupt(Encoder_1.getIntNum(), isr_process_encoder1, RISING);
  attachInterrupt(Encoder_2.getIntNum(), isr_process_encoder2, RISING);
  randomSeed((unsigned long)(lightsensor_12.read() * 123456));
}


void loop() {
    _loop();
    Encoder_1.loop();
    Encoder_2.loop();
    if(ultrasonic_10.distanceCm() < 10){
        Encoder_1.setTarPWM(0);
        Encoder_2.setTarPWM(0);
        _delay(0.5);

        move(2, 40 / 100.0 * 255);
        _delay(0.7);
        move(2, 0);

        move(4, 50 / 100.0 * 255);
        _delay(random(0.5, 2.5 +1));
        move(4, 0);

        move(1, 50 / 100.0 * 255);
        _delay(1);
        move(1, 0);

/*
Situation 1=  0  (not detect line )
sen 1 = 0
sen 2 = 0

Situation 2  = 1 (sensor 2 detect line)
sen1 = 0
sen2 = 1

Situation 3 = 2 (sensor 1 detect line)
sen 1= 1
sen 2 = 0

Situation 4 = 3 (both sensors detect)
sen 1= 1
sen2= 1
*/
    }else{
        if(linefollower_9.readSensors() == 2.000000){

            move(4, 50 / 100.0 * 255);

        }else{
            if(linefollower_9.readSensors() == 1.000000){

                move(3, 50 / 100.0 * 255);

            }else{
                if(linefollower_9.readSensors() == 0.000000){

                    move(1, 50 / 100.0 * 255);

                  }else{
                      if(linefollower_9.readSensors() == 3.000000){
                          if(random(1, 20 +1) == 4.000000){

                              move(1, 40 / 100.0 * 255);
                              _delay(1);
                              move(1, 0);

                          }else{

                              move(3, 40 / 100.0 * 255);
                              _delay(1);
                              move(3, 0);

                          }

                      }

                  }

              }

          }

      }
}