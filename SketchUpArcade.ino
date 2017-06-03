/********************************************************************************
  SketchUp & SparkFun Arcade Controls
  Line (L)
  Rectangle (R)
  Circle (C)
  Push / Pull (P)
  Explosion (CTRL+Shift+N)
  Move (M)
  Paint Bucket (B)
  Tape Measure (T)
  Erase (E)
  Volume Down (CTRL+SHIFT+DOWN)
  Volume Up (CTRL+SHIFT+UP)

  Based on: http://www.arduino.cc/en/Tutorial/KeyboardMessage

  Side LED strips: 43.5" -- roughly 1.1049 m. 66 LEDs (maybe 67)
  Front LED strips: 25.27" -- roughly 64.1 cm -- 38.5 LEDs
  Mux 8 input button channels using 74HC4051


  /*******************************************************************************/
#include <Keyboard.h>
#include <Adafruit_NeoPixel.h>

byte selectPins[] = {14, 16, 10};
byte muxInputPin = 15;

byte comboButtonPins[] = {4, A3, A2, A1, A0};  //bomb, Retro Sound, Retro Video, Volume Up, Volume Down

byte fogRelayPin = 8;
byte fogMachineMain = 9;

byte volumeLEDPin = 5;
byte frontLEDPin = 3;
byte sideLEDPin = 2;
byte startUpWAVPin = 6;
byte bombWAVPin = 7;

Adafruit_NeoPixel frontPixels =  Adafruit_NeoPixel(40, frontLEDPin, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel sidePixels =  Adafruit_NeoPixel(68, sideLEDPin, NEO_GRB + NEO_KHZ800);

const byte brightnessLevel = 255;

unsigned long defaultColor = 0x0000FF;

//standard keypress buttons
char keypress[8] = {'l', 'r', 'c', 'p', 'm', 'b', 't', 'e'};

//setup to handle up to three sequential keystrokes
//create a 2d array for keymapping of combo buttons / commands
char comboKeypress[5][3] = {
  {KEY_LEFT_CTRL, KEY_LEFT_SHIFT, 'n'},
  {KEY_LEFT_CTRL, 'r', 0},
  {KEY_LEFT_CTRL, 'e', 0},
  {KEY_LEFT_CTRL, '.', 0},
  {KEY_LEFT_CTRL, ',', 0}
};

boolean fogFlag = false;
unsigned int fogTime = 1;  
unsigned long fogStartTime;
unsigned long fogStopTime;

unsigned long activityTimeOut;
unsigned long timeOut = 2 * 3600 * 1000;  //activity timeout for fog machine

int previousButtonState = HIGH;   // for checking the state of a pushButton
int counter = 0;                  // button push counter

/********************************************************************************/
void setup() {
  Serial.begin(9600);

  // initialize control over the keyboard:
  Keyboard.begin();

  //setup mux pins for main buttons
  for (int i = 0; i < 3; i++)
    pinMode(selectPins[i], OUTPUT);

  pinMode(muxInputPin, INPUT_PULLUP);

  pinMode(fogMachineMain, OUTPUT);
  pinMode(fogRelayPin, OUTPUT);
  pinMode(volumeLEDPin, OUTPUT);
  pinMode(startUpWAVPin, OUTPUT);
  pinMode(bombWAVPin, OUTPUT);
  pinMode(17, OUTPUT);
  digitalWrite(startUpWAVPin, HIGH);
  digitalWrite(bombWAVPin, HIGH);

  //setup combo buttons
  for (int i = 0; i < 5; i++)
    pinMode(comboButtonPins[i], INPUT_PULLUP);

  frontPixels.begin();
  sidePixels.begin();

  frontPixels.setBrightness(brightnessLevel);
  sidePixels.setBrightness(brightnessLevel);

  //trigger start-up sound.
  digitalWrite(startUpWAVPin, LOW);
  delay(100);
  digitalWrite(startUpWAVPin, HIGH);

  setAllLEDs(colorVal(255, 0, 0));  //startup all LED strands to RED
  digitalWrite(volumeLEDPin, HIGH);
  digitalWrite(fogMachineMain, HIGH);  //turn on fogMachine relay


  setAllLEDs(colorVal(255, 0, 0));
  
 theaterChaseRainbow(100);
 setAllLEDs(colorVal(255, 0, 0));

}

/********************************************************************************/
void loop() {
  readStdButtons();
  readComboButtons();

  if (Serial.available() > 0)
  {
    int inNum = Serial.parseInt();
   
    if (inNum == 1)
    {
      digitalWrite(startUpWAVPin, LOW);
      delay(20);
      digitalWrite(startUpWAVPin, HIGH);
    }
    if (inNum == 2)
    {
        digitalWrite(bombWAVPin, LOW);
        delay(20);
        digitalWrite(bombWAVPin, HIGH);
    }    
  }

    if (fogFlag == true)
    {
      if (millis() < fogStopTime)
      {
        digitalWrite(fogMachineMain, HIGH);
        digitalWrite(fogRelayPin, HIGH);
        //Erics addition for the bomb button
        theaterChaseRainbow(4);
        setAllLEDs(colorVal(255, 0, 0));
      }
      else
      {
//        digitalWrite(fogMachineMain, LOW);
        digitalWrite(fogRelayPin, LOW);
        fogFlag = false;
      }
    }
}

/********************************************************************************/
void readStdButtons() {
  for (int i = 0; i < 8; i++)
  {

    //set the appropriate select pins
    digitalWrite(selectPins[0], (i & B001) >> 0);
    digitalWrite(selectPins[1], (i & B010) >> 1);
    digitalWrite(selectPins[2], (i & B100) >> 2);

    //read the input
    if (digitalRead(muxInputPin) == LOW)
    {
      Serial.print(i);
      Serial.print(":\t");
      Serial.print((i & B100) >> 2 );
      Serial.print(" ");
      Serial.print((i & B010) >> 1 );
      Serial.print(" ");
      Serial.print((i & B001) >> 0 );
      Serial.print(" --> ");

      // activityTimeOut = millis() + timeOut;
      Keyboard.print(keypress[i]);
      Serial.println(keypress[i]);

      setAllLEDs(colorVal(0, 0, 255));
      delay(100);
      setAllLEDs(colorVal(255, 0, 0));
      digitalWrite(17, LOW); //light up debug LED
      //      delay(20);

      //catch to wait for the button release
      while (digitalRead(muxInputPin) == LOW);
      delay(20);
    }
    else
      digitalWrite(17, HIGH);  //turn off debug LED
  }
}

/********************************************************************************/
void readComboButtons()
{
  for (int i = 0; i < 5; i++)
  {
    if (digitalRead(comboButtonPins[i]) == LOW)
    {
      Serial.print("combo");
      Serial.println(i);
      digitalWrite(17, LOW);  //light up debug LED

      //if large red button press -- trigger bomb sound effect
      if (i == 0)
      {
        fogFlag = true;
        fogStartTime = millis();
        fogStopTime = fogStartTime + fogTime * 1000;
        digitalWrite(bombWAVPin, LOW);
        delay(10);
        digitalWrite(bombWAVPin, HIGH);
      }
      for (int key = 0; key < 3; key++) {
        Keyboard.press(comboKeypress[i][key]);
      }
      Keyboard.releaseAll();

      //catch to wait for the button release
      while (digitalRead(comboButtonPins[i]) == LOW);
    }
    else
    {
      digitalWrite(17, HIGH);  //turn off debug LED
    }
  }
}


/********************************************************************************/
/* NeoPixel Routines                                                            */
/********************************************************************************/

void testLEDs()
{
  colorWipe(frontPixels, colorVal(0, 0, 255), 50);
  colorWipe(sidePixels, colorVal(0, 255, 0), 50);
}

/********************************************************************************/
void setAllLEDs(unsigned long c)
{
  for (int i = 0; i < frontPixels.numPixels(); i++) {
    frontPixels.setPixelColor(i, c);
  }
  frontPixels.show();

  for (int i = 0; i < sidePixels.numPixels(); i++) {
    sidePixels.setPixelColor(i, c);
  }
  sidePixels.show();
}

/********************************************************************************/
// Fill the dots one after the other with a color
void colorWipe(Adafruit_NeoPixel strip, unsigned long c, byte wait) {
  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

/********************************************************************************/
void rainbow(Adafruit_NeoPixel strip, byte wait) {
  int i, j;

  for (j = 0; j < 256; j++) {
    for (i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

/********************************************************************************/
// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(Adafruit_NeoPixel strip, int wait) {
  int i, j;

  for (j = 0; j < 256 * 5; j++) { // 5 cycles of all colors on wheel
    for (i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

/********************************************************************************/
//Theatre-style crawling lights.
void theaterChase(Adafruit_NeoPixel strip, unsigned long c, int wait) {
  for (int j = 0; j < 10; j++) { //do 10 cycles of chasing
    for (int q = 0; q < 3; q++) {
      for (int i = 0; i < strip.numPixels(); i = i + 3) {
        strip.setPixelColor(i + q, c);  //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (int i = 0; i < strip.numPixels(); i = i + 3) {
        strip.setPixelColor(i + q, 0);      //turn every third pixel off
      }
    }
  }
}

/********************************************************************************/
//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(byte wait) {
  for (int j = 0; j < 256; j++) {   // cycle all 256 colors in the wheel
    for (int q = 0; q < 3; q++) {
      for (int i = 0; i < frontPixels.numPixels(); i = i + 3) {
        frontPixels.setPixelColor(i + q, Wheel( (i + j) % 255)); //turn every third pixel on
      }
      frontPixels.show();

      for (int i = 0; i < sidePixels.numPixels(); i = i + 3) {
        sidePixels.setPixelColor(i + q, Wheel( (i + j) % 255)); //turn every third pixel on
      }
      sidePixels.show();

      delay(wait);

      for (int i = 0; i < sidePixels.numPixels(); i = i + 3) {
        sidePixels.setPixelColor(i + q, 0);      //turn every third pixel off
      }
      for (int i = 0; i < frontPixels.numPixels(); i = i + 3) {
        frontPixels.setPixelColor(i + q, 0);      //turn every third pixel off
      }
    }
  }
}

/********************************************************************************/
// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return colorVal(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return colorVal(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return colorVal(WheelPos * 3, 255 - WheelPos * 3, 0);
}

/********************************************************************************/
//returns a 32-bit value (24-bits) of RGB given three values R, G, & B
unsigned long colorVal(byte r, byte g, byte b) {
  return ((unsigned long)r << 16) | ((unsigned long)g <<  8) | b;
}


