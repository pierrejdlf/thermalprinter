import gab.opencv.*;
import java.awt.Rectangle;

import processing.serial.*;
Serial myPort;

//////////////////////////////////////////////////////
//// SEND to arduino
// 11 - to init
// 22 - [then XX(length)] before sending data
//    .. arduino will lock and wait XX values, then print
// 33 - back to start

//// RECEIVE from arduino
// "aware" - print was done, head is back to start, arduino is ready to listen
//////////////////////////////////////////////////////

int W = 0;
int H = 0;
boolean arduinoReady = false;
int currentLine = 0;
int[] lines;

boolean GOPRINT = true; // MODE - GO PRINT
boolean JUSTLOOK = false; // MODE - JUST LOOK

boolean FROMFILE = true; // if false, from webcam !

// (31pixel ~ 1cm)
// CENTERED MAX ! image should be max 350px for a 20px margin
int MARGINLEFT = 20; // min = 20 (~18)


boolean FROMWHEREYOUARE = false;

int DURDOTS = 10; // delay between dots
int DURBAND = 2000; // delay between bands

// OPENCV
OpenCV opencv;
Rectangle[] faces;
int FACEMARG = 68; // around detected face
int FACEW = 300; // extracted small image from webcam

void setup() {
  // OPENCV IMAGE
  opencv = new OpenCV(this, "cam.jpeg");
  size(opencv.width, opencv.height);
  opencv.loadCascade(OpenCV.CASCADE_FRONTALFACE);  
  faces = opencv.detect();
  image(opencv.getInput(), 0, 0);

  // SERIAL COM
  println("COM Ports");
  println(myPort.list());
  println("=========");
  println("connect to :"+Serial.list()[6]);
  myPort = new Serial(this, Serial.list()[6], 9600);
}

void draw() {
  if (JUSTLOOK) {
    waitForImageFaceAndGetImage();
    delay(3000);
  }
  if (GOPRINT) {
    GOPRINT = false;
    delay(2000);

    //////////////////////////// IMAGE SOURCE
    if(FROMFILE) lines = initFromFile();
    if(!FROMFILE) lines = initFromWebcam();

    println("OK. Loaded image: ", W, "x", H);

    arduinoReady = true;
    //////////////////////////// to rotate paper
    if (!FROMWHEREYOUARE) {
      backStartAndMargin(170);
    }
    waitUntilReady();

    //////////////////////////// LET's GO PRINT
    backStartAndMargin(MARGINLEFT);
    while (currentLine<lines.length-W*8) {
      int[] sublines = subset(lines, currentLine);
      println("Will serial ask Adruino to print a band");

      delay(DURBAND);

      // print one band
      for (int col=0; col<W*8; col=col+8) {
        waitUntilReady();
        arduinoReady = false;
        int[] subcol = subset(sublines, col);
        myPort.write(22); // ask to listen to 8-stream!
        //delay(2);
        // print 8 dots
        for (int d=0; d<8; d=d+1) {
          delay(DURDOTS);
          //print("pix",subcol[d]);
          myPort.write(int(subcol[d]));
        }
      }

      // next line
      currentLine += W*8;

      waitUntilReady();
      backStartAndMargin(MARGINLEFT);
    }
  }
}

int[] initFromWebcam() {
  int[] imArray = waitForImageFaceAndGetImage();
  while (imArray.length<=0) {
    imArray = waitForImageFaceAndGetImage();
    delay(700);
  }
  W = FACEW;
  H = FACEW;
  return imArray;
}
int[] initFromFile() {
  int[] res = {
  };
  String[] lines = loadStrings("imagedata.txt");
  for (int l=1; l<lines.length; l=l+1) { // first line is size
    res = append(res, int(lines[l]));
  }
  String meta = lines[0];
  W = int(split(meta, " ")[0]);
  H = int(split(meta, " ")[1]);
  return res;
}




void backStartAndMargin(int decalmarg) {
  // back to start
  arduinoReady = false;
  myPort.write(33);
  waitUntilReady();

  if (decalmarg>0) { // margin 
    arduinoReady = false;
    myPort.write(44);
    delay(10);
    myPort.write(decalmarg);
    waitUntilReady();
  }
}
void waitUntilReady() {
  while (!arduinoReady)
    delay(1);
}
void keyPressed() {
  if (keyCode == UP) {
    println("ho.");
  }
}

////////////////////////////////////
void serialEvent(Serial myPort) {
  // read the serial buffer:
  String myString = myPort.readStringUntil('\n');
  if (myString != null) {
    if (myString.indexOf("!AWARE!")!=-1) {
      //println("Arduino seems aware. let's continue.");
      arduinoReady = true;
    } else {
      println("------> ARDUINO:", myString.replaceAll("\n", ""));
    }
  }
}




////////////////////////////////////
int[] waitForImageFaceAndGetImage() {

  int[] CAPTIMAGE = {
  };
  PImage img = loadImage("cam.jpeg");

  opencv = new OpenCV(this, "cam.jpeg");
  image(opencv.getInput(), 0, 0);
  opencv.loadCascade(OpenCV.CASCADE_FRONTALFACE);  
  faces = opencv.detect();

  if (faces.length>0 && faces[0].width>180 && faces[0].x>FACEMARG && faces[0].y>FACEMARG && faces[0].x+faces[0].width<opencv.width-FACEMARG && faces[0].y+faces[0].height<opencv.height-FACEMARG) {
    //println(faces[0].x, faces[0].y, faces[0].width, faces[0].height);
    noFill();
    stroke(0, 255, 0);
    strokeWeight(3);
    rect(faces[0].x, faces[0].y, faces[0].width, faces[0].height);
    stroke(255, 0, 0);
    rect(faces[0].x-FACEMARG, faces[0].y-FACEMARG, faces[0].width+2*FACEMARG, faces[0].height+2*FACEMARG);

    PImage ext = img.get(faces[0].x-FACEMARG, faces[0].y-FACEMARG, faces[0].width+2*FACEMARG, faces[0].height+2*FACEMARG);
    //ext.filter(GRAY);
    //ext.filter(DILATE);
    ext.resize(FACEW, FACEW);
    image(ext, 0, 0);
    //ext.loadPixels();
    int minBright = 255; // init
    int maxBright = 0; // init
    for (int row=0; row<FACEW/8+1; row=row+1) {
      for (int col=0; col<FACEW; col=col+1) {
        for (int dot=0; dot<8; dot=dot+1) {
          if (row*8+dot<FACEW) {
            int b = (int)brightness(ext.get(col, row*8+dot));
            //int b = grayscale(ext.pixels[row*8+dot*wantW+col]);
            //println("val",b);
            minBright = min(b, minBright);
            maxBright = max(b, maxBright);
            CAPTIMAGE = append(CAPTIMAGE, (int)b);
          }
        }
      }
    }
    for (int i=0; i<CAPTIMAGE.length; i=i+1) {
      CAPTIMAGE[i] = 255 - (int)map( CAPTIMAGE[i], minBright, maxBright, 0, 255 );
      //println("res",iCAPTIMAGE[i]);
    }
  } else {
    println("No face found (or too small)");
  }
  println("array:", CAPTIMAGE.length);
  return CAPTIMAGE;
}


//convert RGB to grayscale
int grayscale(color _c) { 
  //extract RGB values
  int r = (_c >> 16) & 0xFF;
  int g = (_c >> 8) & 0xFF;
  int b = _c & 0xFF;

  //typical NTSC color to luminosity conversion
  int intensity = int(0.2989*r + 0.5870*g + 0.1140*b);
  return intensity;
}

