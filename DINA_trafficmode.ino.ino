// Created by Brett.Builds part of the D.I.N.A (DoINeedA) project 
// https://brettbuilds.myportfolio.com/dina-doineeda-notification-device
// Last updated 4/5/17

#include <Bridge.h>
#include <Process.h>
#include <Adafruit_NeoPixel.h>
#define PIN            7   //neopixel gpio pin
#define NUMPIXELS      24  // number of pixels in ring/strip
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

float functionoutput;  //output from the checktraffic() function
int delayval;   //delay between assigning value to next LED
int inputrng[2]; //used to establish traffic value ranges
int r;
int g;
int b;
long previousrun = 0;  //intiate previousrun
const long timebetweenruns = 300000;  //300000 = 5 min
unsigned long currentrun; //set variable to time store

void setup() {
  // Initialize Bridge
  delay(60000); // delay to give the linux processor time to boot
  pixels.begin();
  Bridge.begin();
  // Initialize Serial
  SerialUSB.begin(9600);
  currentrun = millis();
  loading_colors();  // fun loading light sequence
  traffic_check();  // run the traffic check at start up, if no lights come on then most likely the delay above needs to be increased
  clearcolors();  // clear colors before setting new values
  delay(1000);
  fullcolors_dial();   // sets the neopixels according to the value returned from the trafficcheck function. Sets it like a dial counting from best to worst commute time
}

void loop() {
    currentrun = millis(); //sets the counter
    // re-runs program according to the value set for timebetweenruns
    if (currentrun - previousrun >= timebetweenruns){
      previousrun = currentrun; 
      SerialUSB.print("timing trigger"); //for reference
      loading_colors();
      traffic_check();
      clearcolors();
      delay(1000);
      fullcolors_dial();
      }
}
  
///////////////////// sets neopixel colors full with dial //////////////////////////////////

void fullcolors_dial(){
  delayval = 150;  // how quick the neopixel fills LEDs
  SerialUSB.print(functionoutput);
  int startpixel = 0;    //input for each instance
  int endpixel = 24;       //input for each instance
  float totalpixelcount = abs(endpixel - startpixel); //established total possibly pixel count for this instance 
  float currentpixel = ((functionoutput / ((float)inputrng[1])) * totalpixelcount) + startpixel;
  for(float i = startpixel; i < currentpixel; i++) 
  {   
      pixels.setPixelColor(i, pixels.Color(r,g,b)); 
      pixels.show();
      delay(delayval);
      }
}

///////////////////// clear neopixel //////////////////////////////////

void clearcolors(){
  for(int i=0;i<NUMPIXELS;i++){  
  // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
  pixels.setPixelColor(i, pixels.Color(0,0,0)); // set rgb values
  
  pixels.show(); // This sends the updated pixel color to the hardware.
  }
}

///////////////////// loading neopixel //////////////////////////////////

uint32_t Wheel(byte WheelPos) {
if(WheelPos < 85) {
 return (pixels.Color(WheelPos * 3, 255 - WheelPos * 3, 0) * 3);
} else if(WheelPos < 170) {
 WheelPos -= 85;
 return (pixels.Color(255 - WheelPos * 3, 0, WheelPos * 3) * .3);
} else {
 WheelPos -= 170;
 return (pixels.Color(0, WheelPos * 3, 255 - WheelPos * 3) * .3);
  }
}

void loading_colors(){
  uint16_t i, j;
  for(j=0; j<256; j++) {
    for(i=0; i<NUMPIXELS; i++) {
      pixels.setPixelColor(i, Wheel((i+j) & 200));
    }
    pixels.show();
    delay(10);
  }
}

///////////////////// Traffic Mode //////////////////////////////////

void traffic_check() {
  SerialUSB.print("beginning traffic check");
  delay(1000);
  char inData[20]; // Allocate some space for the string
  char inChar=-1; // Where to store the character read
  byte index = 0; // Index into array; where to store the character
  Process p;		// Create a process and call it "p"
  p.begin("python");	// Process that launch the python code
  p.addParameter("/mnt/sda1/arduino/www/python/transittime_check.py"); // initiated python code make sure to update this file path if you change the name or directory
  p.run();		// Run the process and wait for its termination
  while (p.available() > 0) {
     inChar = p.read(); // Read results being transmitted onto the serial communication between processors
     inData[index] = inChar; // Store it
     index++; // Increment where to write next
     inData[index] = '\0'; // Null terminate the string
  }
  // Ensure the last bit of data is sent.
  SerialUSB.flush();
  int traveltime = atoi(inData);
  functionoutput = traveltime; //main purpose is used when adding additional modes
  SerialUSB.print(traveltime);  //for reference

////Change these values///

  inputrng[0] = 1080;   // the quickest time for your commute in sec
  inputrng[1] = 3000;  // the longest time for your commute in sec
  int dif = (inputrng[1] - inputrng[0])/3;  //sets ranges
  int rng1 = inputrng[0] + dif;  
  int rng2 = rng1 + dif;

  // set RGB colors for the quickest commute and before
  if (traveltime < inputrng[0]){   
    r = 0;
    g = 120;
    b = 0;
  } // set RGB colors for the quickest - 1/3 range
   else if (inputrng[0] < traveltime && traveltime < rng1 ){
    r = 50;
    g = 120;
    b = 0;
  } // set RGB colors for the 1/3 - 2/3 range
   else if (rng1 < traveltime && traveltime < rng2 ){
    r = 100;
    g = 80;
    b = 0;
  } // set RGB colors for the 2/3 - longest range
  else if (rng2 < traveltime && traveltime < inputrng[1]) {
    r = 100;
    g = 25;
    b = 0;
  } // set RGB colors for the longest commute and beyond
  else {
    r = 120;
    g = 0;
    b = 0;
  }

 for ( uint8_t i = 0; i < 20; i++ )  //clear inData
 inData[i] = 0;
}


