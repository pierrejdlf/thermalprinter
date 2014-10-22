#include <Stepper.h>
#include <iostream.h>
#include <copy>

// data you want to print comes-in as a bytes array from file
#include "sampleimage.cpp"

// change this depending on the number of steps
// per revolution of your motor
#define motorSteps 20
// stepper pins
#define motorPin1 8
#define motorPin2 9
#define motorPin3 10
#define motorPin4 11
#define ledPin 13

// thermal head pins
#define STB1 1
#define STB2 2
#define STB3 3
#define LAT 4
#define DATA 5
#define CLK 6

boolean debug = true;
boolean printing = false;
boolean onetime = true;

int t = 1; // loop for thermal head clock/data/...

// whole line data that will be sent to the DATA pin for one line
uint8_t linedata[] = {
  0x11,0x00,0x22};

int TCLOCK = 10; // clock period in microseconds

// durations as clock cycles
int NLAT = 432; // one dot data every clock period before releasing LATCH
int NSTB = 5; // means 5*10*2 = during 100 microseconds !
int NWAIT = 10; // lets wait 10 cycles between events 
int NTOTAL = NLAT + 3*NSTB + 5*NWAIT;

// initialize of the Stepper library:
Stepper myStepper(motorSteps,motorPin1,motorPin2,motorPin3,motorPin4); 

////////////////////////////////////////////////////////
void setup() {
  resetPrinter();
  // set the motor speed at 60 RPMS:
  myStepper.setSpeed(150);
  // Initialize the Serial port:
  Serial.begin(9600);
  // set up the LED pin:
  pinMode(ledPin, OUTPUT);
  // blink the LED:
  blink(3);
  Serial.println("Setup done.");
}

////////////////////////////////////////////////////////
void loop() {
  ////////////////////////////////// STEPPER
  //  Serial.println("Forward");
  //  myStepper.step(700); // Step forward
  //  delay(300);
  //  blink(5);
  //  Serial.println("Backward");
  //  myStepper.step(-700); // Step backward
  //  delay(300);
  //  blink(7);

  ////////////////////////////////// THERMAL HEAD CYCLE
  if(printing) {

    delay(900);

    if(t==1) {
      blink(1);
      Serial.println("Printing cycle launch in 1 seconds !");
      delay(1000);
      blink(2);
    }
    Serial.print("Printing cycle k:");
    Serial.println(t);
    cycle(t);
    t = t+1;
    if(t>NTOTAL) {
      printing = false;
      Serial.println("Printing cycle ends.");
    }
  }

  // do things at start, boy
  if(onetime) {
    onetime = false;
    resetPrinter();
    printImageData(sampleimage,432,1);
  }
}

////////////////////////////////////////////////////////
// this is a clock cycle
void cycle(int k) {
  // CLOCK
  delayMicroseconds(TCLOCK);
  setPinVal(CLK,LOW);

  // first we send the DATA
  if(k<NLAT) setPinVal(DATA, pixelValue(k-1) ? HIGH : LOW ); // k-1 cause we started at 1
  else setPinVal(DATA,LOW);

  // every 432 dots, we will load the data using LATCH
  if(k==NLAT) setPinVal(LAT,LOW);
  else setPinVal(LAT,HIGH);

  // after all that, let's burn paper with the STROBES
  if(k==NLAT + NWAIT)
    setPinVal(STB1,LOW);
  else
    setPinVal(STB1,HIGH);

  if(k==NLAT + 2*NWAIT + NSTB)
    setPinVal(STB2,LOW);
  else
    setPinVal(STB2,HIGH);

  if(k==NLAT + 3*NWAIT + 2*NSTB)
    setPinVal(STB3,LOW);
  else
    setPinVal(STB3,HIGH);

  // CLOCK
  delayMicroseconds(TCLOCK);
  setPinVal(CLK,HIGH);
}

////////////////////////////////////////////////////////
// is the pixel k black or white ?
boolean pixelValue(int k) { // k is in [0,431]
  // linedata is an array of 54 bytes
  uint8_t b = linedata[k/8];
  int gut = k%8;
  boolean res = (b & (1<<gut)) != 0; // if non-zero, the bit was set !
  if(k<27) {
    Serial.print("byte index:");
    Serial.println(k/8);
    Serial.print("the byte:");
    Serial.println(b,HEX);
    Serial.print("decalage:");    
    Serial.println(gut);
    Serial.print("------------------------ pixel value:");
    Serial.println(res);
  }
  return res;
};

////////////////////////////////////////////////////////
void printLine() {
  // 1. rotate stepper
  //myStepper.step(100);

  // 2. launch clock things (within loop).
  printing = true;
}

////////////////////////////////////////////////////////
// prints an image
void printImageData(const uint8_t *image,int w,int h) {
  //if(w>432) return 0;

  // a full width image line is 432 = 144*3 dots
  // each strobe gets 144 = 8*18 dots, aka 18 bytes
  // the minimal image is a full-width-one-line, aka an array of 18*3 = 54 bytes
  // let's suppose the image has exactly w=432, aka h*54 bytes in its data
  for(int rowStart=0; rowStart<h; rowStart+=54) {
    //int chunkHeight = ((h-rowStart) > 53) ? 53 : (h-rowStart);
//    Serial.println("SAMPLE Linedata array was (0,1):");
//    Serial.println(linedata[0]);
//    Serial.println(linedata[1]);
    // get linedata pointer
    *linedata = pgm_read_byte(image+rowStart);
    Serial.println("SAMPLE SET Linedata array is now (first 3):");
    Serial.println(linedata[0]);
    Serial.println(linedata[1]);
    Serial.println(linedata[2]);
    printLine();
    //    for (int i=0; i<((w/8)*chunkHeight); i++) {
    //      printLine( pgm_read_byte(image + (rowStart*(w/8)) + i) );
    //    }
  }
}

////////////////////////////////////////////////////////
void resetPrinter() {
  t = 1;
  printing = false;
  setPinVal(STB1,HIGH);
  setPinVal(STB2,HIGH);
  setPinVal(STB3,HIGH);
  setPinVal(LAT,HIGH);
  setPinVal(DATA,LOW);
}

////////////////////////////////////////////////////////
void setPinVal(int pin, int val) {
  //if(pin==STB1 || pin==STB2 || pin==STB3)
  if(debug) {
    //Serial.println("Setting pin:",pin,val);
  }
  else {
    //digitalWrite(pin,val);
  }
}

////////////////////////////////////////////////////////
// Blink the reset LED:
void blink(int howManyTimes) {
  int i;
  for (i=0; i< howManyTimes; i++) {
    digitalWrite(ledPin, HIGH);
    delay(200);
    digitalWrite(ledPin, LOW);
    delay(200);
  }
}





