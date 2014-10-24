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

// temperature & LED pins
#define tPin 0
#define ledPin 13

// thermal head pins
#define STB1 1
#define STB2 2
#define STB3 3
#define LAT 4
#define DATA 5
#define CLK 6
#define VOLTSWITCH 12

boolean debug = false;
boolean printing = false;
boolean goprint = true;

int t = 1; // loop for thermal head clock/data/...

// whole line data that will be sent to the DATA pin for one line
uint8_t linedata[] = {
  0x11,0x00,0x22};

int TCLOCK = 1; // clock period in microseconds
int MAXTEMP = 27; // max Temp°C before turning everything off

// durations as clock cycles
int NLAT = 432; // one dot data every clock period before releasing LATCH
int NSTB = 700; // MAX is 670 microseconds
int NWAIT = 25; // lets wait some cycles between events 
int NTOTAL = NLAT + 3*NSTB + 5*NWAIT;

long starttime = micros();

// initialize of the Stepper library:
Stepper myStepper(motorSteps,motorPin1,motorPin2,motorPin3,motorPin4); 

////////////////////////////////////////////////////////
void setup() {
  resetPrinter();
  // set the motor speed at 60 RPMS:
  myStepper.setSpeed(150);
  // Initialize the Serial port:
  Serial.begin(9600);

  // set up pin modes:
  pinMode(ledPin, OUTPUT);
  pinMode(STB1, OUTPUT);
  pinMode(STB2, OUTPUT);
  pinMode(STB3, OUTPUT);
  pinMode(LAT, OUTPUT);
  pinMode(DATA, OUTPUT);
  pinMode(CLK, OUTPUT);
  pinMode(VOLTSWITCH, OUTPUT);

  // blink the LED:
  blink(3);
  Serial.println("Printer setup done. Welcome.");
  myStepper.step(-20);

  getTemperature(true);
}

////////////////////////////////////////////////////////
void loop() {
  ////////////////////////////////// STEPPER TESTS
  //  Serial.println("Forward");
  //  myStepper.step(700); // Step forward
  //  delay(300);
  //  blink(5);
  //  Serial.println("Backward");
  //  myStepper.step(-700); // Step backward
  //  delay(300);
  //  blink(7);

  ////////////////////////////////// THERMAL HEAD CYCLE
  if(getTemperature(false) >= MAXTEMP) {
    Serial.println("========= TEMPERATURE WARNING !! aborting all");
    resetPrinter(); // will set printing=false and stop Vh current
    blink(10);
    delay(5000);
  }

  if(printing) {
    if(t==1) {
      blink(1);
      Serial.println("Printing cycle launch in 1 seconds !");
      delay(1000);
      blink(2);
    }
    if(t%1000000==0) {
      Serial.print("Printing cycle k:");
      Serial.println(t);
      printTime("cycle"); 
    }
    cycle(t);
    t = t+1;
    if(t>NTOTAL) {
      printing = false;
      Serial.println("Printing cycle ends.");
      resetPrinter();
      myStepper.step(-120);
    }
  }

  // do things once at start, boy
  if(goprint) {
    goprint = false;
    resetPrinter();
    printImageData(sampleimage,432,1);
    // plugging 24V POWER
    setPinVal(VOLTSWITCH,HIGH);
  }
}

////////////////////////////////////////////////////////
// this is a clock cycle
void cycle(int k) {
  //setPinVal(CLK,LOW);

  // first we send the DATA
  if(k<NLAT) setPinVal(DATA, HIGH); // TEST ALL BLACK
  //if(k<NLAT) setPinVal(DATA, pixelValue(k-1) ? HIGH : LOW ); // k-1 cause we started at 1
  //else setPinVal(DATA,LOW);

  // every 432 dots, we will load the data using LATCH
  if(k==NLAT) {
    setPinVal(LAT,LOW);
    Serial.println("!!!!! LATCHED");
  }
  if(k==NLAT+4)
    setPinVal(LAT,HIGH);

  // after all that, let's burn paper with the STROBES
  if(k==NLAT + NWAIT)
    setPinVal(STB1,LOW);
  if(k==NLAT + NWAIT + NSTB)
    setPinVal(STB1,HIGH);

  if(k==NLAT + 2*NWAIT + NSTB)
    setPinVal(STB2,LOW);
  if(k==NLAT + 2*NWAIT + 2*NSTB)
    setPinVal(STB2,HIGH);

  if(k==NLAT + 3*NWAIT + 2*NSTB)
    setPinVal(STB3,LOW);
  if(k==NLAT + 3*NWAIT + 3*NSTB)
    setPinVal(STB3,HIGH);

  // CLOCK
  delayMicroseconds(TCLOCK);
  setPinVal(CLK, k%2==0 ? HIGH : LOW);
}

////////////////////////////////////////////////////////
// is the pixel k black or white ?
boolean pixelValue(int k) { // k is in [0,431]
  // linedata is an array of 54 bytes
  uint8_t b = linedata[k/8];
  int gut = k%8;
  boolean res = (b & (1<<gut)) != 0; // if non-zero, the bit was set !
  //  if(k<27) {
  //    Serial.print("byte index:");
  //    Serial.println(k/8);
  //    Serial.print("the byte:");
  //    Serial.println(b,HEX);
  //    Serial.print("decalage:");    
  //    Serial.println(gut);
  //    Serial.print("------------------------ pixel value:");
  //    Serial.println(res);
  //  }
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
    /*Serial.println("SAMPLE SET Linedata array is now (first 3):");
    Serial.println(linedata[0]);
    Serial.println(linedata[1]);
    Serial.println(linedata[2]);*/
    printLine();
    //    for (int i=0; i<((w/8)*chunkHeight); i++) {
    //      printLine( pgm_read_byte(image + (rowStart*(w/8)) + i) );
    //    }
  }
}

////////////////////////////////////////////////////////
void resetPrinter() {
  t = 1;
  printing = false; // !! IMPORTANT
  setPinVal(VOLTSWITCH,LOW); // !! IMPORTANT
  setPinVal(STB1,HIGH);
  setPinVal(STB2,HIGH);
  setPinVal(STB3,HIGH);
  setPinVal(LAT,HIGH);
  setPinVal(DATA,LOW);
}

////////////////////////////////////////////////////////
void setPinVal(int pin,int val) {
  //if(pin==STB1 || pin==STB2 || pin==STB3)
  if(debug) {
    //Serial.print("Setting pin");
  }
  else {
    if(pin==STB1 || pin==STB2 || pin==STB3) {
      Serial.print(pin);
      Serial.print(" STROBE: ");
      Serial.println(val==1 ? "HIGH" : "LOW");
      printTime("made strobe:");
    }
    digitalWrite(pin,val);
  }
}

////////////////////////////////////////////////////////
// get the thermal head temp in C°
double getTemperature(boolean verbose) {
  // display initial temperature
  int rawA = analogRead(tPin); // 0 to 1023
  double Rk = 10.0/((1024.0/rawA)-1.0);
  double T = 0;
  if(Rk<40) T = 20;
  if(Rk<30) T = 25;
  if(Rk<20) T = 35;
  if(Rk<10) T = 45;
  if(Rk<5) T = 55;
  if(verbose) {
    Serial.print("Rk/Temperature: ");
    Serial.print(Rk);
    Serial.print(" / ");
    Serial.println(T);
  }
  return T;
}

////////////////////////////////////////////////////////
// printTime
void printTime(String str) {
  Serial.print(str);
  Serial.print(" Time(milliseconds) is: ");
  Serial.println((micros() - starttime)/1000.0);
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

















