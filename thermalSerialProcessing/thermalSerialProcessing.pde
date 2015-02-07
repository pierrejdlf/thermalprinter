import processing.serial.*;
Serial myPort;

//////////////////////////////////////////////////////
//// SEND to arduino
// 11 - to init
// 22 - [then XX(length)] before sending data
//    .. arduino will lock and wait XX values, then print

//// RECEIVE from arduino
// "aware" - print was done, head is back to start, arduino is ready to listen
//////////////////////////////////////////////////////

int Nband = 0;
int W = 0;
int H = 0;
boolean done = false;
int currentLine = 0;
String lines[];
boolean init = true;

void setup()
{
  println("COM Ports");
  println(myPort.list());
  println("=========");
  println("connect to :"+Serial.list()[6]);
  myPort = new Serial(this, Serial.list()[6], 9600);
}
void draw() {
  delay(500);
  if(init) {
    init = false;
    lines = loadStrings("imagedata.txt");
  }
}

void initBusiness() {
  String meta = lines[0];
  W = int(split(meta," ")[0]);
  H = int(split(meta," ")[1]);
  println("Loaded image: ",W,"x",H);
  if(W*8<=1700) {
    String[] sublines = subset(lines,1);
    askArduinoToPrintLine(sublines);
    currentLine = W*8+1;
  } else {
    println("YOUR WIDTH IS TOO MUCH.");
  }
}
void nextBusiness() {
  if(currentLine < lines.length-W*8) {
    println("Will ask Arduino another line.");
    String[] sublines = subset(lines,currentLine);
    askArduinoToPrintLine(sublines);
    currentLine += W*8;
  } else {
    println("everything FINISHED.");
    done = true;
  }
}

void askArduinoToPrintLine(String[] DATA) {
  Nband+=1;
  println("Will serial ask Adruino to print band:",Nband);
  myPort.write(22); // ask to listen !
  delay(10);
  myPort.write(W); // send width
  for(int d=0; d<W*8; d=d+1) {
    delay(20);
    //if(d<5) println("sending:",int(DATA[d]));
    myPort.write(int(DATA[d]));
  }
}

void keyPressed() {
  if (keyCode == UP) {
    println("ho.");
    //myPort.write(22);
    nextBusiness();
  }
}

////////////////////////////////////
void serialEvent(Serial myPort) {
  // read the serial buffer:
  String myString = myPort.readStringUntil('\n');
  if(myString != null) {
    println("------> ARDUINO:", myString.replaceAll("\n",""));
    if(myString.indexOf("!AWARE!")!=-1 && !done) {
      println("Arduino seems aware. let's continue.");
      delay(3000);
      nextBusiness();
    }
    if(myString.indexOf("!START!")!=-1 && !done) {
      println("Arduino seems started. let's go.");
      initBusiness();
    }
  }
}

