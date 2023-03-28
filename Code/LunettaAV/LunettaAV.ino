#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <FastLED.h>
#include <Bounce.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library.
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

unsigned long lastTime;
unsigned long currentTime;
unsigned long timeInterval = 1000;

int selectedPattern = 3;
int lastPattern = 0;
int speeder, sizer = 0;

//Global Variables
float gl_size = 2.;
long double gl_time;
bool gl_refresh = true;
long double gl_lastRefresh = 0;

int lastDigipotValue = 0;

//Time calculation thing so that it can be reset
long double lastMillis = 0;

#define PI 3.14
#define SIZEMAX 10
#define SPEEDMAX 1000

struct Vec2f {
  float x;
  float y;
};

int bouncePin = 9;

Bounce patternButton = Bounce(bouncePin, 5);
int patternOffset = 0;

int pinXOOO = 3;
int pinOXOO = 4;
int pinOOXO = 5;
int pinOOOX = 6;

int slaveSelectPin = 10;

int XOOO, OXOO, OOXO, OOOX = 0;
int inputPattern = 0;
int lastReset = 0;

void setup() {
  Serial.begin(9600);
  Wire.setSCL(19);
  Wire.setSDA(18);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  // display.display();

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 20);
  display.println(F("N8IRAV"));
  display.println(F("v0.0.1"));
  display.println(F("Sourya Sen 2021"));
  display.display();

  delay(2000); // Pause for 2 seconds

  // Clear the buffer
  display.clearDisplay();

  lastTime = millis();
  pinMode(13, OUTPUT);
  pinMode(bouncePin, INPUT); //this is for the button

  pinMode(14, INPUT); //ANALOGINPUTS
  pinMode(15, INPUT);

  pinMode(pinXOOO, INPUT);
  pinMode(pinOXOO, INPUT);
  pinMode(pinOOXO, INPUT);
  pinMode(pinOOOX, INPUT);

  pinMode(slaveSelectPin, OUTPUT);
  SPI.begin();

  selectedPattern = random(0,8);

}

void loop() {

  patternButton.update();

  if (patternButton.risingEdge()) {
    patternOffset++;
  }

  XOOO = digitalRead(pinXOOO);
  OXOO = digitalRead(pinOXOO);
  OOXO = digitalRead(pinOOXO);
  OOOX = digitalRead(pinOOOX);

//  XOOO = 1;
//  OXOO = 1;
//  OOXO = 0;
//  OOOX = 1;

  //  int debugOut = 10000 + 1000 * XOOO + 100 * OXOO + 10 * OOXO + OOOX;
  //  Serial.println(debugOut);

  inputPattern = bToD(100 * OXOO + 10 * OOXO + OOOX);
  selectedPattern = (inputPattern + patternOffset) % 8;

  Serial.print("Selected Pattern: ");
  Serial.println(selectedPattern);

  if (lastReset != XOOO) {
    //RisingEdge
    if (XOOO == 1) {
      //doGlobalReset();
    }
    //Falling Edge
    else {
      //
    }
    lastReset = XOOO;
  }

  int pin14 = analogRead(14);
  sizer = map(pin14, 0, 1023, 1, SIZEMAX);

  int pin15 = analogRead(15);
  speeder = map(pin15, 0, 1023, 1, SPEEDMAX);
  
    Serial.print("Speeder is: ");
    Serial.println(speeder);
    Serial.print("Sizer is: ");
    Serial.println(sizer);

  int dPotValue = map(pin14, 0, 1023, 0, 255);

  if(abs(lastDigipotValue - dPotValue) > 10){
    digitalPotWrite(dPotValue);
    lastDigipotValue = dPotValue;
  }
  
  

  if (gl_refresh) display.clearDisplay();
  unsigned long currentTime = millis();

  /*
    //CHANGE PATTERNS EVERY X INTERVALS.
    if (currentTime - lastTime >= timeInterval) {
      lastTime = currentTime;

      selectedPattern ++;
      selectedPattern = selectedPattern % 5;
      resetGrowingCircles(); //better to reset all global variables here, what's the best way to do?

      //Ignore the top just to test...
      selectedPattern = 5;
    }
  */

  /*
    //RESET GLOBALS EVERY X INTERVALS.
    if (currentTime - lastTime >= timeInterval) {
      lastTime = currentTime;

      doResetGlobals();
    }
  */

  switch (selectedPattern) {
    case 0:
      drawNoise(speeder, sizer);
      break;
    case 1:
      drawGrowingCircles(speeder, sizer);
      break;
    case 2:
      drawWrongLine(speeder, sizer);
      break;
    case 3:
      drawRotationalSquarePattern(speeder, sizer);
      break;
    case 4:
      drawPulsarBalls(speeder, sizer);
      break;
    case 5:
      drawMatrix(speeder, sizer);
      break;
    case 6:
      drawNoisyCircles(speeder, sizer);
      break;
    case 7:
      drawSimplexNoise();
      break;
  }

  if(selectedPattern != lastPattern){
    display.ssd1306_command(SSD1306_SETSTARTLINE | 0);
    doResetGlobals();
    lastPattern = selectedPattern;
  }

  display.display();
  //  delay(1);

  //digitalWrite(13, HIGH);

  long double timeDelta = millis() - lastMillis;
  gl_time += timeDelta;

  lastMillis = millis();

}

//-------------------------------------------------------------
//FUNCTIONS FOR MAIN
//-------------------------------------------------------------
int bToD(unsigned num) {
  unsigned res = 0;

  for (int i = 0; num > 0; ++i) {
    if ((num % 10) == 1) res += (1 << i);
    num /= 10;
  }

  return res;
}

//-------------------------------------------------------------
void doResetGlobals() {
  gl_size = 0.;
  gl_time = 0;
  gl_lastRefresh = 0;
}

//-------------------------------------------------------------
int digitalPotWrite(unsigned int value){
  SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
  //take the SS pin low.
  digitalWrite(slaveSelectPin, LOW);
  //send the address.
  SPI.transfer(B00010001);
  SPI.transfer(value);
  //take the SS pin high.
  digitalWrite(slaveSelectPin, HIGH);
  SPI.endTransaction();
}

//-------------------------------------------------------------
//-------------------------------------------------------------
//ALL PATTERNS BELOW
//-------------------------------------------------------------
//-------------------------------------------------------------

//-------------------------------------------------------------
//0 -> WHITE NOISE
//-------------------------------------------------------------
void drawNoise(int sp, int sz) {
  gl_refresh = true;
  int randMax = SIZEMAX + 2;
  for (int x = 0; x < display.width(); x++) {
    for (int y = 0; y < display.width(); y++) {
      int r = random(0, (randMax - sz));
      if (r  == 1) {
        display.drawPixel(x, y, SSD1306_WHITE);
      }
    }
  }
}

//-------------------------------------------------------------
//1 - > GROWING CIRCLES
//-------------------------------------------------------------
void drawGrowingCircles(int sp, int sz) {
  gl_refresh = true;
  float increment = float(sp) / float(SPEEDMAX) * 10.;
  increment = constrain(increment, 1., 10.);
  for (int i = 1; i <= sz; i += 1) {
    display.drawCircle(display.width() / 2, display.height() / 2, i * gl_size / 2, SSD1306_WHITE);

  }

  display.display();

  int smallestSize = gl_size / 2;

  if (smallestSize > display.width() / 4) gl_size = 0.; //this is slightly random but oh well so is life
  gl_size += increment;
}

//-------------------------------------------------------------
//2 - > ROTATIONAL SQUARES
//-------------------------------------------------------------
void drawRotationalSquarePattern(int sp, int sz) {
  gl_refresh = true;
  for (int i = 0; i < sz + 3; i ++) {
    int mappedSpeed = map(sp, 1, 1000, 10, 1);
    //    Serial.println(mappedSpeed);
    drawRotatedSquare(i * 2, i * 2, gl_time / (10. * mappedSpeed) * i);
  }
}

//CORE FUNCTION
//-------------------------------------------------------------
void drawRotatedSquare(int x, int y, float theta) {
  Vec2f p1, p2, p3, p4;

  p1.x = 2 * -x;
  p1.y = 2 * y;

  p2.x = 2 * x;
  p2.y = 2 * y;

  p3.x = 2 * x;
  p3.y = 2 * -y;

  p4.x = 2 * -x;
  p4.y = 2 * -y;

  rotateVec2f(p1, theta);
  rotateVec2f(p2, theta);
  rotateVec2f(p3, theta);
  rotateVec2f(p4, theta);

  display.drawLine(p1.x, p1.y, p2.x, p2.y, SSD1306_WHITE);

  display.drawLine(p2.x, p2.y, p3.x, p3.y, SSD1306_WHITE);

  display.drawLine(p3.x, p3.y, p4.x, p4.y, SSD1306_WHITE);

  display.drawLine(p4.x, p4.y, p1.x, p1.y, SSD1306_WHITE);

}

//-------------------------------------------------------------
//3 - > SQUIGGLY LINE
//-------------------------------------------------------------
void drawWrongLine(int sp, int sz) {
  gl_refresh = true;
  int step = map(sp, 1, 1000, 63, 1);
  float lastx = -999;
  float lasty = -999;
  float y = display.height() / 2;
  for (int x = 0; x < display.width() ; x += step) {
    y = display.height() / 2 + random(-(sz * 2), (sz * 2));
    if (lastx > -999) {
      display.drawLine(x, y, lastx, lasty, SSD1306_WHITE);
    }
    lastx = x;
    lasty = y;
  }
}

//-------------------------------------------------------------
//4 - > PULSAR BALLS
//-------------------------------------------------------------
//This entire function probably can and should be written better
void drawPulsarBalls(int sp, int sz) {
  gl_refresh = true;

  int mappedSpeedForRadius = map(sp, 1, SPEEDMAX, 5, 20);

  float rad = sin(gl_time / 1000. * mappedSpeedForRadius);
  int radiusMax = map(sz, 1, SIZEMAX, 2, 6);
  rad = map(rad, -1, 1, 2, radiusMax);

  float mappedSpeedForTheta = map(sp, 1, SPEEDMAX, 10, 2);
  //  Serial.println(mappedSpeedForTheta);

  drawRotatedCircle(-6, 6, gl_time / mappedSpeedForTheta, rad);
  drawRotatedCircle(6, 6, gl_time / mappedSpeedForTheta, rad);
  drawRotatedCircle(6, -6, gl_time / mappedSpeedForTheta, rad);
  drawRotatedCircle(-6, -6, gl_time / mappedSpeedForTheta, rad);

  float rad2 = map(rad, 1, 5, radiusMax + 1, 1);
  display.fillCircle(display.width() / 2, display.height() / 2, rad2, SSD1306_WHITE);

  drawRotatedCircle(-12, 6, gl_time / mappedSpeedForTheta, rad - 1);
  drawRotatedCircle(12, 6, gl_time / mappedSpeedForTheta, rad - 1);
  drawRotatedCircle(12, -6, gl_time / mappedSpeedForTheta, rad - 1);
  drawRotatedCircle(-12, -6, gl_time / mappedSpeedForTheta, rad - 1);

  drawRotatedCircle(-6, 12, gl_time / mappedSpeedForTheta, rad - 1);
  drawRotatedCircle(6, 12, gl_time / mappedSpeedForTheta, rad - 1);
  drawRotatedCircle(6, -12, gl_time / mappedSpeedForTheta, rad - 1);
  drawRotatedCircle(-6, -12, gl_time / mappedSpeedForTheta, rad - 1);
}

//CORE ROTATIONAL CIRCLE FUNCTION
//-------------------------------------------------------------
void drawRotatedCircle(int x, int y, float theta, int radius) {

  float radtheta = theta * PI / 180.;

  float costheta = cos(radtheta);
  float sintheta = sin(radtheta);

  float xprime = x * costheta  - y * sintheta;
  float yprime = x * sintheta + y * costheta;

  display.fillCircle(display.width() / 2 + 2 * xprime, display.height() / 2 + 2 * yprime, radius, SSD1306_WHITE);
}

//-------------------------------------------------------------
//5 - > MATRIX
//-------------------------------------------------------------
void drawMatrix(int sp, int sz) {

  int refreshTime = map(sp, 1, SPEEDMAX, 800, 100);
  int density = map(sz, 1, SIZEMAX, 1, 16);

  if (gl_time - gl_lastRefresh >= refreshTime) {
    display.clearDisplay();
    gl_refresh = true;
    gl_lastRefresh = gl_time;
    //    Serial.println(int(gl_time));
    //    Serial.println(int(gl_lastRefresh));
  }

  if (gl_refresh) {
    display.setTextSize(1);
    display.cp437(true);
    for (int x = 0; x < display.width(); x += 8) {
      for (int y = 0; y < display.height(); y += 8) {
        int selector = random(0, 8 + y / density);
        int rc = random(256);
        display.setCursor(x, y);
        switch (selector) {
          case 0:
            display.write(' ');
            break;
          case 1:
            display.write(x + gl_time / 100.);
            break;
          case 2:
            display.write(y + gl_time / 100.);
            break;
          case 3:
            display.fillRect(x, y, 4, 4, SSD1306_WHITE);
            break;
          case 4:
            display.fillRect(x, y, 4, 4, SSD1306_WHITE);
            break;
          default:
            display.write(' ');
            break;
        }
      }
    }
    gl_refresh = false;
  }

  if(!gl_refresh){
     display.ssd1306_command(SSD1306_SETSTARTLINE | (64 - int(gl_time/100.) % 64));
  }

  for (int x = 0; x < display.width(); x += 8) {
    for (int y = 0; y < display.height(); y += 8) {
      int r = random(0, 512);
      if (r == 1) {
        int t = random(0, 2);
        if (t == 0) {
          display.fillRect(x, y, 4, 4, SSD1306_WHITE);
        } else {
          int c = random(256);
          display.setCursor(x, y);
          display.write(c);
        }
      }
    }
  }
}

//-------------------------------------------------------------
//6 - > NOISY CIRCLES
//-------------------------------------------------------------
void drawNoisyCircles(int sp, int sz) {
  gl_refresh = true;

  float timeFactor = map(sp, 1, SPEEDMAX, 1000., 10.);
  float sizeFactor = map(sz, 1, SIZEMAX, 1, 16);

  for (int num = 0; num < 8; num ++) {
    float lastx = -999;
    float lasty = -999;
    for (int i = 0; i < 361; i += 8) {
      float radtheta = i * PI / 180.;
      float noisy = inoise8(i * gl_time / timeFactor);

      noisy = map(noisy, -70, 70, 0, 16);

      float radius = 4 * sin(i + gl_time / timeFactor) + num * sizeFactor + noisy;

      float x = display.width() / 2 + radius * cos(radtheta);
      float y = display.height() / 2 + radius * sin(radtheta);

      if (lastx > -999) {
        display.drawLine(x, y, lastx, lasty, SSD1306_WHITE);
      }

      lastx = x;
      lasty = y;
    }
  }

}

//-------------------------------------------------------------
//-------------------------------------------------------------
//FUNCTIONS FOR GEOMETRY MANIPULATION
//-------------------------------------------------------------
//-------------------------------------------------------------

//ROTATE VEC2F AROUND THETA
//-------------------------------------------------------------
void rotateVec2f(Vec2f &vec, float theta) {
  float radtheta = theta * PI / 180.;

  float costheta = cos(radtheta);
  float sintheta = sin(radtheta);

  float xprime = vec.x * costheta  - vec.y * sintheta;
  float yprime = vec.x * sintheta + vec.y * costheta;

  //  xprime += display.width()/2;
  //  yprime += display.height()/2;

  vec.x = display.width() / 2 + xprime;
  vec.y = display.height() / 2 + yprime;
}

//-------------------------------------------------------------
//-------------------------------------------------------------
//-------------------------------------------------------------
//-------------------------------------------------------------


//SKETCHPAD

//------------------------------------------------------------


void drawStrobes() {
  int strobeInterval = 500;

  unsigned long cTime = millis();
  static unsigned long lastStrobe = 0;
  static bool state = false;

  if (cTime - lastStrobe >= strobeInterval) {
    lastStrobe = cTime;

    if (state) {
      display.fillScreen(WHITE);
    } else {
      display.clearDisplay();
    }
    state = !state;
  }

}

//-------------------------------------------------------------
void drawSimplexNoise() {
  gl_refresh = true;
  
  for (int x = 0; x < display.width(); x += 2) {
    for (int y = 0; y < display.width(); y += 2) {
      int r = inoise8(x * gl_time/10., y * gl_time/100.);
      r = map(r, -70, 70, 0, 255);
//      Serial.println(r);
      if (r > 300) {
        display.drawPixel(x, y, SSD1306_WHITE);
      } else {
        // DO NOTHING :)
      }
    }
  }
}

//-------------------------------------------------------------
void drawVector() {
  float x = sin(110. * millis() / 1.);
  float y = sin(55. * millis() / 1.);

  float mappedx = map(x, -1., 1., -display.height() / 2, display.height() / 2);
  float mappedy = map(y, -1., 1., -display.height() / 2, display.height() / 2);

  display.drawCircle(mappedx + display.width() / 2, mappedy + display.height() / 2, 1, SSD1306_WHITE);
}

//-------------------------------------------------------------
//void drawTestFlight() {
//  float radius = sin(millis() / 100.0);
//  radius = map(radius, -1, 1, 1, 5);
//
//  drawRotationThing(-6, 6, millis() / 10., radius);
//  drawRotationThing(6, 6, millis() / 10., radius);
//  drawRotationThing(6, -6, millis() / 10., radius);
//  drawRotationThing(-6, -6, millis() / 10., radius);
//
//  float rad2 = map(radius, 1, 5, 10, 1);
//  display.fillCircle(display.width() / 2, display.height() / 2, rad2, SSD1306_WHITE);
//
//  drawRotationThing(-12, 6, millis() / 10., radius);
//  drawRotationThing(12, 6, millis() / 10., radius);
//  drawRotationThing(12, -6, millis() / 10., radius);
//  drawRotationThing(-12, -6, millis() / 10., radius);
//
//  drawRotationThing(-6, 12, millis() / 10., radius);
//  drawRotationThing(6, 12, millis() / 10., radius);
//  drawRotationThing(6, -12, millis() / 10., radius);
//  drawRotationThing(-6, -12, millis() / 10., radius);
//}
