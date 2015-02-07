///////////////////////////////////////////////////
#include <Stepper.h>

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
// PAPER margins (min/max:
#define MARGINLEFT 70
// pas between dots
#define PAS 1

///////////////////////////////////////////////////
int W = 0;
boolean aware = true;
boolean receiving = false;
int received = 0;
unsigned char LINEDATA[1130]; // let's allocate max size anyway for a band of 8 lines


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
  Serial.println("OK. printed band, went back.");
  receiving = false;
  received = 0;
  W = 0;
  aware = true;
  // let's inform Processing
  Serial.println("OK. i'm !AWARE! now");
}

void loop() {
  delay(2);

  if(aware) {
    if(Serial.available() > 0) {
      int in = Serial.read();
      if(!receiving && in==11) {
        Serial.println("OK. i will init");
      }
      if(receiving && W!=0 && received<W*8 ) { // THREE
        LINEDATA[received] = (unsigned char)in;
        received = received + 1;
        if(received%70==0) {
          Serial.print("OK. ...received so far...: ");
          Serial.println(received);
          //Serial.print(":");
          //Serial.println(in);
        }
      }
      if(receiving && W==0) { // TWO
        W = in;
        Serial.print("OK. i understood W is: ");
        Serial.println(in);
      }
      if(!receiving && in==22) { // ONE
        receiving = true;
        Serial.println("OK. i will listen to your data");
      }
      if(receiving && W!=0 && received==W*8) {
        Serial.print("OK. i now have all the data. i will print the received W*8: ");
        Serial.println(W*8);    
        receiving = false;
        aware = false;
        printLine();
      }
    }
  }
}

void printLine() {
  int po = 0;

  initHeadAtStart();
  myStepper.step(-MARGINLEFT);
  // let's print a line
  for(int c=0; c<W; c=c+1) {
    heatVertical(
     map( (int)LINEDATA[po+7], 0,255,MINHEATMICROS,MAXHEATMICROS),
     map( (int)LINEDATA[po+6], 0,255,MINHEATMICROS,MAXHEATMICROS),
     map( (int)LINEDATA[po+5], 0,255,MINHEATMICROS,MAXHEATMICROS),
     map( (int)LINEDATA[po+4], 0,255,MINHEATMICROS,MAXHEATMICROS),
     map( (int)LINEDATA[po+3], 0,255,MINHEATMICROS,MAXHEATMICROS),
     map( (int)LINEDATA[po+2], 0,255,MINHEATMICROS,MAXHEATMICROS),
     map( (int)LINEDATA[po+1], 0,255,MINHEATMICROS,MAXHEATMICROS),
     map( (int)LINEDATA[po], 0,255,MINHEATMICROS,MAXHEATMICROS)
    );
    po = po+8;
    myStepper.step(-PAS);
    delay(5);
  }
  backToAware();
}


void heatVertical(int v0, int v1, int v2, int v3, int v4, int v5, int v6, int v7) {
  digitalWrite(DOT0, HIGH);
  delayMicroseconds(v0);
  digitalWrite(DOT0, LOW);

  digitalWrite(DOT1, HIGH);
  delayMicroseconds(v1);
  digitalWrite(DOT1, LOW);

  digitalWrite(DOT2, HIGH);
  delayMicroseconds(v2);
  digitalWrite(DOT2, LOW);

  digitalWrite(DOT3, HIGH);
  delayMicroseconds(v3);
  digitalWrite(DOT3, LOW);

  digitalWrite(DOT4, HIGH);
  delayMicroseconds(v4);
  digitalWrite(DOT4, LOW);

  digitalWrite(DOT5, HIGH);
  delayMicroseconds(v5);
  digitalWrite(DOT5, LOW);

  digitalWrite(DOT6, HIGH);
  delayMicroseconds(v6);
  digitalWrite(DOT6, LOW);

  digitalWrite(DOT7, HIGH);
  delayMicroseconds(v7);
  digitalWrite(DOT7, LOW);
}

void initHeadAtStart() {
  while(analogRead(buttonPin) < 200) {
    myStepper.step(1);
  }
}



