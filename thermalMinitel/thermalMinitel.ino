
#include <Stepper.h>
#include <iostream.h>
#include <copy>

// data you want to print comes-in as a bytes array from file
#include "imagedata.h"

// change this depending on the number of steps
// per revolution of your motor
#define motorSteps 40

// stepper pins
#define motorPin1 2
#define motorPin2 3
#define motorPin3 4
#define motorPin4 5

#define buttonPin A0

// thermal pins
#define DOT0 6
#define DOT1 7
#define DOT2 8
#define DOT3 9
#define DOT4 10
#define DOT5 11
#define DOT6 12
#define DOT7 13

Stepper myStepper(motorSteps,motorPin1,motorPin2,motorPin3,motorPin4); 

// HEAT DURATIONS
#define MINHEATMICROS 100
#define MAXHEATMICROS 1900
// PAPER margins (min/max:
#define MARGINLEFT 90
// pas between dots
#define PAS 1

void setup() {
  Serial.begin(9600);
  pinMode(DOT0, OUTPUT);
  pinMode(DOT1, OUTPUT);
  pinMode(DOT2, OUTPUT);
  pinMode(DOT3, OUTPUT);
  pinMode(DOT4, OUTPUT);
  pinMode(DOT5, OUTPUT);
  pinMode(DOT6, OUTPUT);
  pinMode(DOT7, OUTPUT);
  pinMode(buttonPin, INPUT);

  digitalWrite(DOT0, LOW);
  digitalWrite(DOT1, LOW);
  digitalWrite(DOT2, LOW);
  digitalWrite(DOT3, LOW);
  digitalWrite(DOT4, LOW);
  digitalWrite(DOT5, LOW);
  digitalWrite(DOT6, LOW);
  digitalWrite(DOT7, LOW);
  
  myStepper.setSpeed(150);
  
  sleepVerbose(4000);
  Serial.print("Welcome TO THE MINITEL, will print WxH = ");
  Serial.print(W);
  Serial.print("x");
  Serial.println(H);
  //Serial.println(sizeof(IMGD)/2);
  
  if(MAXHEATMICROS<=4000)
    printImage();
   else
    Serial.println("TOO MUCH HEAT DURAITION!");
   //testVoltage(300);
  //printTest();
}


int mics(prog_uint16_t * point) {
  int d = min(pgm_read_word_near(point), 255);
  return map(d,0,255,MINHEATMICROS,MAXHEATMICROS);
}
void printImage() {
  int po = 0;
  
  initHeadAtStart();
  myStepper.step(-MARGINLEFT);
  initHeadAtStart();
      
  for(int l=0;l<(int)(H/8+0.5);l=l+1) {

    // go back to start
    initHeadAtStart();
    myStepper.step(-MARGINLEFT);
    delay(20);
    Serial.print("... Printing 8-band ");
    Serial.print(l);
    Serial.print(" over ");
    Serial.println(H/8);
    // let's print a line
    for(int c=0; c<W; c=c+1) {
      heatVertical(
        mics(IMGD+po+7),
        mics(IMGD+po+6),
        mics(IMGD+po+5),
        mics(IMGD+po+4),
        mics(IMGD+po+3),
        mics(IMGD+po+2),
        mics(IMGD+po+1),
        mics(IMGD+po)
      );
      po = po+8;
      myStepper.step(-PAS);
      delay(5);
    }
  }
}

//
//void printTest() {
//  for(int p=1; p<50; p=p+1) {
//    heatLine(255); 
//    delay(100);
//    myStepper.step(-30);
//    delay(1200);
//    myStepper.step(28);
//    delay(1200);
//  }
//}

void initHeadAtStart() {
  while(!isAtTheEnd()) {
    myStepper.step(1);
    //delay(100);
  }
  Serial.println("I am at the end !!");
}

boolean isAtTheEnd() {
  int val = analogRead(buttonPin);
  return (val>200); // is at 0 or ~675 (end!)
}

void testVoltage(int from) {
  for(int d = from; d < 1500; d=d+40) {
    Serial.print("Testing dirac voltage, duration= ");
    Serial.println(d);
    dotdirac(d);
    myStepper.step(-50);
    delay(1000);
    myStepper.step(48);
    delay(1000);
  }
}
void dotdirac(int duration) {
  digitalWrite(DOT0, HIGH);
  delay(duration);
  digitalWrite(DOT0, LOW);
}

void sleepVerbose(int dur) {
  for(int t = 0; t < dur; t=t+1000) {
    Serial.print("sleep ");
    Serial.print(t/1000);
    Serial.print(" over ");
    Serial.print(dur/1000);
    Serial.println(" ");
    delay(1000);
  }
}
void heatVertical(int v0, int v1, int v2, int v3, int v4, int v5, int v6, int v7) {
//  Serial.print("bite= ");
//  for(int v=0;v<8;v=v+1) {
//    Serial.print( bitRead(line,v) );
//  }
//  Serial.println(" ");

  digitalWrite(DOT0, v0>=MINHEATMICROS && v0<=MAXHEATMICROS ? HIGH : LOW );
  delayMicroseconds(v0);
  digitalWrite(DOT0, LOW);
  
  digitalWrite(DOT1, v1>=MINHEATMICROS && v1<=MAXHEATMICROS ? HIGH : LOW );
  delayMicroseconds(v1);
  digitalWrite(DOT1, LOW);

  digitalWrite(DOT2, v2>=MINHEATMICROS && v2<=MAXHEATMICROS ? HIGH : LOW );
  delayMicroseconds(v2);
  digitalWrite(DOT2, LOW);

  digitalWrite(DOT3, v3>=MINHEATMICROS && v3<=MAXHEATMICROS ? HIGH : LOW );
  delayMicroseconds(v3);
  digitalWrite(DOT3, LOW);

  digitalWrite(DOT4, v4>=MINHEATMICROS && v4<=MAXHEATMICROS ? HIGH : LOW );
  delayMicroseconds(v4);
  digitalWrite(DOT4, LOW);

  digitalWrite(DOT5, v5>=MINHEATMICROS && v5<=MAXHEATMICROS ? HIGH : LOW );
  delayMicroseconds(v5);
  digitalWrite(DOT5, LOW);

  digitalWrite(DOT6, v6>=MINHEATMICROS && v6<=MAXHEATMICROS ? HIGH : LOW );
  delayMicroseconds(v6);
  digitalWrite(DOT6, LOW);

  digitalWrite(DOT7, v7>=MINHEATMICROS && v7<=MAXHEATMICROS ? HIGH : LOW );
  delayMicroseconds(v7);
  digitalWrite(DOT7, LOW);
}

void loop() {
}


















