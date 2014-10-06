
/* 
 Stepper Motor Control - one revolution
 
 This program drives a unipolar or bipolar stepper motor. 
 The motor is attached to digital pins 8 - 11 of the Arduino.
 
 The motor should revolve one revolution in one direction, then
 one revolution in the other direction.  
 
  
 Created 11 Mar. 2007
 Modified 30 Nov. 2009
 by Tom Igoe
 
 */

#include <Stepper.h>
#include <LiquidCrystal.h>

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

const int stepsPerRevolution = 200;  // change this to fit the number of steps per revolution
                                     // for your motor

// initialize the stepper library on pins 8 through 11:
Stepper myStepper(stepsPerRevolution, 9, 10);            

void setup() {
  // set the speed at 60 rpm:
  myStepper.setSpeed(70);
  digitalWrite(13, LOW);
  // initialize the serial port:
  Serial.begin(9600);
  lcd.clear();
  lcd.print("dey see me rollin");
  myStepper.step(-stepsPerRevolution);
  delay(500);
  myStepper.step(-stepsPerRevolution);
  delay(500);
  myStepper.step(0);
}

void loop() {
  digitalWrite(13, HIGH);
  myStepper.step(stepsPerRevolution);
}

