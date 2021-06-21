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
unsigned long timeInterval = 4000;
int selectedPattern = 3;
static float size = 2.;
//SimplexNoise sn;
int speeder, sizer = 0;

#define PI 3.14
#define SIZEMAX 10
#define SPEEDMAX 1000

struct Vec2f {
  float x;
  float y;
};

Bounce input1 = Bounce(12, 5);

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
  display.display();
  delay(1000); // Pause for 2 seconds

  // Clear the buffer
  display.clearDisplay();

  lastTime = millis();
  pinMode(13, OUTPUT);
  pinMode(12, INPUT);

  pinMode(14, INPUT);
  pinMode(15, INPUT);

  selectedPattern = 7;

}

void loop() {

  int pin14 = analogRead(14);
  sizer = map(pin14, 0, 1023, 1, SIZEMAX);

  int pin15 = analogRead(15);
  speeder = map(pin15, 0, 1023, 1, SPEEDMAX);

  Serial.print("Speeder is: ");
  Serial.println(speeder);
  Serial.print("Sizer is: ");
  Serial.println(sizer);

  display.clearDisplay();
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

  switch (selectedPattern) {
    case 0:
      drawNoise(speeder, sizer);
      break;
    case 1:
      drawGrowingCircles(speeder, sizer);
      break;
    case 2:
      drawWrongLine();
      break;
    case 3:

      //      drawRotatedSquare(10, 10, millis() / 10.0);

      for (int i = 0; i < 10; i++) {
        drawRotatedSquare(i * 2, i * 2, millis() / 100. * i);
      }
      break;
    case 4:
      drawSimplexNoise();
      break;
    case 5:
      drawStrobes();
      break;
    case 6:
      drawVector();
      break;
    case 7:
      drawTestFlight();
      break;
  }

  display.display();
  //  delay(1);

  digitalWrite(13, HIGH);

  input1.update();

  if (input1.risingEdge()) {
    selectedPattern++;
    selectedPattern = selectedPattern % 4;
    //    Serial.println(selectedPattern);
  }

}

void drawNoise(int sp, int sz) {
  int randMax = SIZEMAX + 2;
  for (int x = 0; x < display.width(); x++) {
    for (int y = 0; y < display.width(); y++) {
      int r = random(0, (randMax-sz));
      if (r  == 1) {
        display.drawPixel(x, y, SSD1306_WHITE);
      }
    }
  }
}

void drawGrowingCircles(int sp, int sz) {
  float increment = float(sp)/float(SPEEDMAX);
  increment = constrain(increment, .1, 1.);
  for (int i = 1; i <= sz; i += 1) {
    display.drawCircle(display.width() / 2, display.height() / 2, i * size / 2, SSD1306_WHITE);

  }

  display.display();

  int smallestSize = sz * size / 2;

  if (smallestSize > display.width()/2) size = 2.;
  size += increment;
}

void resetGrowingCircles() {
  size = 0;
}

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

void drawSimplexNoise() {
  static int offsetx = 0;
  static int offsety = 0;
  for (int x = 0; x < display.width(); x += 2) {
    for (int y = 0; y < display.width(); y += 2) {
      int r = inoise8(x + offsetx, y + offsety);
      //      Serial.println(r);
      if (r > 212) {
        display.drawPixel(x, y, SSD1306_WHITE);
      } else {
        // DO NOTHING :)
      }
    }
  }

  offsetx += 15;
  offsety += 10;
}

void drawRotationThing(int x, int y, float theta, int radius) {

  float radtheta = theta * PI / 180.;

  float costheta = cos(radtheta);
  float sintheta = sin(radtheta);

  float xprime = x * costheta  - y * sintheta;
  float yprime = x * sintheta + y * costheta;

  //  display.drawCircle(display.width() / 2 + 2 * xprime, display.height() / 2 + 2 * yprime, radius, SSD1306_WHITE);
  display.fillCircle(display.width() / 2 + 2 * xprime, display.height() / 2 + 2 * yprime, radius, SSD1306_WHITE);
}

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

void drawWrongLine() {

  int step = 10;
  float lastx = -999;
  float lasty = -999;
  float y = display.height() / 2;
  for (int x = 0; x < display.width() ; x += step) {
    y = display.height() / 2 + 10 - random(5);
    if (lastx > -999) {
      display.drawLine(x, y, lastx, lasty, SSD1306_WHITE);
    }
    lastx = x;
    lasty = y;
  }

}

void drawVector() {
  float x = sin(110. * millis() / 1.);
  float y = sin(55. * millis() / 1.);

  float mappedx = map(x, -1., 1., -display.height() / 2, display.height() / 2);
  float mappedy = map(y, -1., 1., -display.height() / 2, display.height() / 2);

  display.drawCircle(mappedx + display.width() / 2, mappedy + display.height() / 2, 1, SSD1306_WHITE);
}

void drawTestFlight() {
  float radius = sin(millis() / 100.0);
  radius = map(radius, -1, 1, 1, 5);
  drawRotationThing(-6, 6, millis() / 10., radius);
  drawRotationThing(6, 6, millis() / 10., radius);
  drawRotationThing(6, -6, millis() / 10., radius);
  drawRotationThing(-6, -6, millis() / 10., radius);
  float rad2 = map(radius, 1, 5, 10, 1);
  display.fillCircle(display.width() / 2, display.height() / 2, rad2, SSD1306_WHITE);

  drawRotationThing(-12, 6, millis() / 10., radius);
  drawRotationThing(12, 6, millis() / 10., radius);
  drawRotationThing(12, -6, millis() / 10., radius);
  drawRotationThing(-12, -6, millis() / 10., radius);

  drawRotationThing(-6, 12, millis() / 10., radius);
  drawRotationThing(6, 12, millis() / 10., radius);
  drawRotationThing(6, -12, millis() / 10., radius);
  drawRotationThing(-6, -12, millis() / 10., radius);


}
