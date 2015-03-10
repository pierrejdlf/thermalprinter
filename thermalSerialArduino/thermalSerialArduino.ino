///////////////////////////////////////////////////
#include <Stepper.h>
#include <avr/pgmspace.h>
#include <PS2Keyboard.h>

const int DataPin = 3;
const int IRQpin = 2;

PS2Keyboard keyboard;

#define CHARRIOT 130

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
Stepper myStepper(40, A2, A3, 4, 5); 
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
int CURRENTCHARPOS = 0;

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

  keyboard.begin(DataPin, IRQpin, PS2Keymap_French);

  Serial.println("Ok. I'm Arduino");
  delay(2000);
  Serial.println("!START!");
  
  initHeadAtStartAndMargin();
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

  //////////////////////////////////////////////// KEYBOARD
  if(keyboard.available()) {
    // read the next key
    char c = keyboard.read();
    processChar(c);
  }

  //////////////////////////////////////////////// IMAGE
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

byte a[5] = { B00110000, B01001010, B01001010, B01001010, B01111100 };
byte b[5] = { B01111111, B01001000, B01001000, B01001000, B00110000 };
byte c[5] = { B00111100, B01000010, B01000010, B01000010, B00100100 };
byte d[5] = { B00111000, B01000100, B01000100, B01000100, B01111111 };
byte e[5] = { B00111100, B01001010, B01001010, B01001010, B00101100 };
byte f[5] = { B00001000, B01111100, B00001010, B00001010, B00000010 };

byte g[5] = { B00111100, B01000010, B01010010, B01010010, B00110100 };
byte h[5] = { B01111110, B01001000, B01001000, B01001000, B00110000 };
byte i[5] = { B00000000, B00001000, B01111010, B00000000, B00000000 };
byte j[5] = { B00000000, B01000000, B01001000, B00111010, B00000000 };
byte k[5] = { B01111110, B00010000, B00101000, B01000100, B00000000 };

byte l[5] = { B00000000, B00000010, B00111110, B01000000, B00000000 };
byte m[5] = { B01111100, B00000100, B01111000, B00000100, B01111000 };
byte n[5] = { B01111110, B01001000, B01001000, B01001000, B00110000 };
byte o[5] = { B00111100, B01000010, B01000010, B01000010, B00111100 };
byte p[5] = { B01111110, B00010010, B00010010, B00010010, B00001100 };

byte q[5] = { B01111110, B01001000, B01001000, B01001000, B00110000 };
byte r[5] = { B00110000, B01001000, B01001000, B01001000, B01111110 };
byte s[5] = { B01001100, B01001010, B01010010, B01010010, B00110010 };
byte t[5] = { B00000100, B00111111, B01000100, B01000100, B00000100 };
byte u[5] = { B00111100, B01000000, B01000000, B01000000, B01111100 };

byte v[5] = { B00001100, B00110000, B01000000, B00110000, B00001100 };
byte w[5] = { B00011110, B01100000, B00011000, B01100000, B00011110 };
byte x[5] = { B01000010, B00100100, B00011000, B00100100, B01000010 };
byte y[5] = { B00100110, B01001000, B01001000, B01001000, B00111110 };
byte z[5] = { B01000010, B01100010, B01010010, B01001010, B01000110 };

const byte* alph[26] = {a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z};

void processChar(char ch) {
  
  int charint = (int)ch;
  if(charint>=97 && charint <=122) { // ALPHABET !
    myStepper.step(-PAS);
    printChar(alph[charint-97]);
    CURRENTCHARPOS += 7;
  }
  
  if(charint==32) { // SPACE
    myStepper.step(-6);
    CURRENTCHARPOS+=6;
  }
  
  if(ch==PS2_ENTER || ch==PS2_DOWNARROW || CURRENTCHARPOS>350) {
    initRetourCharriotAndMargin();
    CURRENTCHARPOS = 0;
  }
  
}

void printChar(const byte *charlist) {
  for(int vert=0; vert<5; vert=vert+1) {
    for(int dot=0; dot<8; dot=dot+1) {
      boolean on = charlist[vert] & (1 << dot);
      int durat = 0;
      if(on) {
        durat = MAXHEATMICROS;
        int pin = 13-dot;
        digitalWrite(pin, HIGH);
        delayMicroseconds(durat);
        digitalWrite(pin, LOW);
      }
    }
    delay(10);
    myStepper.step(-PAS);
    delay(10);
  }
  myStepper.step(-PAS);
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

void initRetourCharriotAndMargin() {
  if(CURRENTCHARPOS<CHARRIOT)
    myStepper.step(-CHARRIOT+CURRENTCHARPOS);
  initHeadAtStart();
  myStepper.step(-20);
}
void initHeadAtStartAndMargin() {
  initHeadAtStart();
  myStepper.step(-20);
}
void initHeadAtStart() {
  while(analogRead(buttonPin) < 200) {
    myStepper.step(1);
  }
}








