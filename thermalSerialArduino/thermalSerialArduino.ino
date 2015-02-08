///////////////////////////////////////////////////
#include <Stepper.h>
#include <avr/pgmspace.h>

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
Stepper myStepper(40, 2, 3, 4, 5); 
///////////////////////////////////////////////////

// HEAT DURATIONS
#define MINHEATMICROS 200
#define MAXHEATMICROS 1900

// pas between dots
#define PAS 1


///////////////////////////////////////////////////
boolean aware = true;

boolean receiving = false;
int received = 0;
unsigned char LINEDATA[8];

boolean moving = false;


// the setup routine runs once when you press reset:
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
  Serial.println("Ok. I'm Arduino");
  delay(2000);
  Serial.println("!START!");
}

// reset listener
void backToAware() {
  receiving = false;
  received = 0;
  aware = true;
  // let's inform Processing
  Serial.println("OK. i'm !AWARE! now");
}

void loop() {
  //delay(2);

  if(aware) {
    if(Serial.available() > 0) {
      int in = Serial.read();
      //////////////////////// STREAMDATA
      if(receiving) {
        if(received<8 ) {
          LINEDATA[received] = (unsigned char)in;
          received = received + 1;
        }
        if(received==8) {
          //Serial.print("OK. i now have all the data. i will print the received 8: ");  
          receiving = false;
          aware = false;
          printLine();
          //delay(2);
          backToAware();
        }
      }
      //////////////////////// MOVING
      else if(moving) {
        myStepper.step(-in);
        moving = false;
        delay(10);
        backToAware();
      }
      //////////////////////// asking code
      else {
        if(in==11) { // INIT
          Serial.println("OK. i will init.");
        }
        if(in==22) { // STREAMDATA
          receiving = true;
          //Serial.println("OK. i will listen to your data");
        }
        if(in==33) { // BACKSTART
          Serial.println("OK. back to start.");
          initHeadAtStart();
          delay(10);
          backToAware();
        }
        if(in==44) { // MOVE
          moving = true;
        }
      }
    }
  }
}

void printLine() {
  // let's print a line
  for(int dot=0; dot<8; dot=dot+1) {
    int pin = 13-dot; // 6-13
    int durat = map( (int)LINEDATA[dot], 0,255,MINHEATMICROS,MAXHEATMICROS );
    digitalWrite(pin, HIGH);
    delayMicroseconds(durat);
    digitalWrite(pin, LOW);
  }
  myStepper.step(-PAS);
}

void initHeadAtStart() {
  while(analogRead(buttonPin) < 200) {
    myStepper.step(1);
  }
}





