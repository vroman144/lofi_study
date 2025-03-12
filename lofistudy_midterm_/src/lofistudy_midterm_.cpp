/*
 * Project lofi_study
 * Author: Victor Roman
 * Date: 03.05.25
 * For comprehensive documentation and examples, please visit:
 * https://docs.particle.io/firmware/best-practices/firmware-template/
 */

// Include Particle Device OS APIs
#include "Particle.h"
#include "IotClassroom_CNM.h"
#include "hue.h"
#include "wemo.h"
#include "IoTTimer.h"
#include "Colors.h"
#include "Button.h"
#include "neopixel.h"
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1306.h"
#include "Adafruit_BME280.h"
#include "encoder.h"

byte status, address, nDevices;

#define XPOS 0
#define YPOS 1

// PIXELCOUNT VARIABLES //
const int REDLED = D19;
const int GREENLED = D13;
const int BLUELED = D11;

// DECLARING OLED && BME VARIABLES //
Adafruit_BME280 bme;

const char degree = 248;
const char cent = 37;
int colorPurple;
const int HEXADDRESS = 0x76;
float tempC, tempF;
float pressPA;
float humidRH;
float pressureHg;

// DECLARING ENCODER VARIABLES //
int switchState;
int preSwitchState;
int position;
int prePosition;
const int PIXELCOUNT = 69;

// HUE LIGHT VARIABLES //
unsigned int currentTime;
int lastSecond;

bool buttonPress;
float t;
bool studyMode;
bool breakMode;

#define LOGO16_GLCD_HEIGHT 16
#define LOGO16_GLCD_WIDTH 16
void testdrawline();
void testdrawrect(void);

// PIXEL FILL FUNCTION //
void pixelFill(int startP, int endP, int hex);

// Let Device OS manage the connection to the Particle Cloud
SYSTEM_MODE(SEMI_AUTOMATIC);
const int WEMO1 = 1;
const int WEMO2 = 2;
const int BULB = 1;
const int BULB = 2;
const int BULB = 3;
const int BULB = 4;
const int BULB = 5;

SYSTEM_THREAD(ENABLED);

SerialLogHandler logHandler(LOG_LEVEL_INFO);

// OLED PINS //
const int OLED_RESET = -1;
Adafruit_SSD1306 display(OLED_RESET);
// DECLARING  PINS pixel, SPI1 PIN //
Adafruit_NeoPixel pixel(PIXELCOUNT, SPI1, WS2812B);
// DECLARING ENCODER PINS //

Encoder encoder(D9, D17);
Button button1(D12);
IoTTimer breakTimer;
void setup()
{
  Wire.begin();
  Serial.begin(9600);
  waitFor(Serial.isConnected, 15000);
  Serial.printf("I2c Snanner \n");

  //MYWEMO && HUE SETUP //
  WiFi.on();
  WiFi.clearCredentials();
  WiFi.setCredentials("IoTNetwork");

  while (WiFi.connecting())
  {
    Serial.printf(".");
  }
  Serial.printf("\n\n");

  // BME SETUP //
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  status = bme.begin(0x76);
  if (status == false)
  {
    Serial.printf("BME280 at address 0x%02X failed to start", 0x76);
  }

  // NEOPIXEL SETUP //
  pixel.begin();
  pixel.setBrightness(70);
  pixel.show();

  // OLED //
  display.display();
  display.clearDisplay();

  // ENCODER SETUP //
  pinMode(REDLED, OUTPUT);
  pinMode(BLUELED, OUTPUT);
  pinMode(GREENLED, OUTPUT);
  pixelFill(0, 69, purple);
}

void loop()
{

  currentTime = millis();
  digitalWrite(REDLED, HIGH);

  // STUDY MODE //
  if (studyMode)
  {
    breakTimer.startTimer(250000);
    lastSecond = millis();
    studyMode = true;

    // BME INPUT READINGS
    humidRH = bme.readHumidity();
    tempC = bme.readTemperature();
    pressPA = bme.readPressure();
    tempF = tempC * (9.0 / 5.0) + 32.0;
    pressureHg = pressPA / 3386.39;

    // TURNING ON WEMO1(HUMIDIFIER) && WEMO2 (STUDY LIGHT)
    Serial.printf("Turning on Wemo# %i\n Wemo on Wemo %i\n", WEMO1, WEMO2);
    wemoWrite(WEMO1, HIGH);
    wemoWrite(WEMO2, HIGH);

    // STUDY MODE HUE LIGHTS (PURPLE)
    Serial.printf("Setting color of bulb %i to color %06i\n", BULB, HueRainbow[6]);
    setHue(BULB, true, HueRainbow[6], 255, 255);

    // PIXELFILL PURPLE FOR STUDY MODE //
    if (breakTimer.isTimerReady())
    {
      pixelFill(0, 46, purple);
    }

    // BME PRINT TO MONITOR
    if ((currentTime - lastSecond) > 500)
    {
      lastSecond = millis();
      Serial.printf("Humidity =%f\n", humidRH);
      Serial.printf("TempF: %f F\n", tempF);
      Serial.printf("pressPA =%f inHg\n", pressPA);
    }

    // BME READINGS PRINT TO OLED
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.printf("T:%0.0f%c  H:%0.0f%c", tempF, degree, pressureHg, cent);
    display.setTextColor(BLACK, WHITE);
    display.display();
    display.clearDisplay();
  }

  // TIME GREATER THAN 2.5 MINUTES DO THIS
  if ((currentTime - lastSecond) > 250000)
  {
    lastSecond = millis();
    breakMode = true;

    /// ENTERING BREAK MODE//
    /// BREAKMODE IS 15 SECONDS ///
    /// TURN WEMO1(HUMIDIFER) INTO MANUAL READING FOR BREAK
  }
  // TURN NEO PIXELS ORANGE FOR BREAKMODE///
  if (breakMode)
  {
    breakTimer.startTimer(31000);
    pixelFill(0, 46, orange);
    if (humidRH < 31.0)
    { // turn on humidifier
      Serial.printf("Turning on Wemo# %i\n", WEMO1);
      wemoWrite(WEMO1, HIGH);
    }

    if (humidRH >= 31.0)
    { // turn off humidifer
      Serial.printf("Turning off Wemo#%i\n", WEMO1);
      wemoWrite(WEMO1, LOW);
    }
    /// TURN WEMO2 (STUDY LIGHT) OFF FOR BREAKTIME
    Serial.printf("Turning off Wemo%i\n", WEMO2);
    wemoWrite(WEMO2, LOW);
  }
}
  // MANUAL MODE //
  // if (button1.isClicked())
  // {
  //   buttonPress = !buttonPress;
  //   Serial.printf("%i Button Click", button1);
  // }

  // if (buttonPress)
  // {
  //   pixel.clear();

  //   // ENCODER LOOP //

  //   // ENCODER MIN-MAX //

  //   switchState = encoder.read();
  //   if (switchState > 1)
  //   { // DISCOVER MODE (WHITELED) //
  //     digitalWrite(REDLED, HIGH);
  //     digitalWrite(GREENLED, HIGH);
  //     digitalWrite(BLUELED, HIGH);
  //     setHue(3, true, HueYellow, 255, 255);
  //     pixelFill(0, 46, WHITE);
  //   }

  //   // AWAY MODE (BLUELED) //
  //   if (switchState < 1)
  //   {
  //     digitalWrite(BLUELED, HIGH);
  //     digitalWrite(REDLED, LOW);
  //     digitalWrite(GREENLED, LOW);
  //     pixelFill(0, PIXELCOUNT, blue);
  //   }
    // DISPLAY READINGS TO OLED //
    // // BME READINGS PRINT TO OLED
    // display.setTextSize(1);
    // display.setTextColor(WHITE);
    // display.setCursor(0, 0);
    // display.printf("T:%0.0f%c  H:%0.0f%c", tempF, degree, pressureHg, cent);
    // display.setTextColor(BLACK, WHITE);
    // display.display();
    // display.clearDisplay();

    //   // PIXELFILL PURPLE FOR STUDY MODE //
    //   currentTime = millis();
    //   digitalWrite(REDLED, HIGH);

    //   // TURNING ON WEMO1(HUMIDIFIER) && WEMO2 (STUDY LIGHT)
    //   Serial.printf("Turning on Wemo# %i\n Wemo on Wemo %i\n", WEMO1, WEMO2);
    //   wemoWrite(WEMO1, HIGH);
    //   wemoWrite(WEMO2, HIGH);

    //   // STUDY MODE HUE LIGHTS (PURPLE)
    //   Serial.printf("Setting color of bulb %i to color %06i\n", BULB, HueRainbow[6]);
    //   setHue(BULB, true, HueRainbow[6], 255, 255);

  // BME PRINT TO MONITOR
  // if ((currentTime - lastSecond) > 500)
  // {
  //   lastSecond = millis();
  //   Serial.printf("Humidity =%f\n", humidRH);
  //   Serial.printf("TempF: %f F\n", tempF);
  //   Serial.printf("pressPA =%f inHg\n", pressPA);
  // }

  // BME READINGS PRINT TO OLED
//   display.setTextSize(1);
//   display.setTextColor(WHITE);
//   display.setCursor(0, 0);
//   display.printf("T:%0.0f%c  H:%0.0f%c", tempF, degree, pressureHg, cent);
//   display.setTextColor(BLACK, WHITE);
//   display.display();
//   display.clearDisplay();

//   //TIME GREATER THAN 2.5 MINUTES DO THIS
//   if ((currentTime - lastSecond) > 150000)
//   {
//     lastSecond = millis();
//     breakMode = true;
//     breakTimer.startTimer(31000);
//     /// ENTERING BREAK MODE AFTER 2.5 MINUTES///
//     /// BREAKMODE IS 15 SECONDS ///
//     /// TURN WEMO1(HUMIDIFER) INTO MANUAL READING FOR BREAK
//   }
//   TURN NEO PIXELS ORANGE FOR BREAKTIME///
//   if (breakMode)
//   {
//     pixelFill(0, 69, orange)
//     if (humidRH < 31.0)
//     { // turn on humidifier
//       Serial.printf("Turning on Wemo# %i\n", WEMO1);
//       wemoWrite(WEMO1, HIGH);
//     }

//     if (humidRH >= 31.0)
//     { // turn off humidifer
//       Serial.printf("Turning off Wemo#%i\n", WEMO1);
//       wemoWrite(WEMO1, LOW)
    
//     /// TURN WEMO2 (STUDY LIGHT) OFF FOR BREAKTIME
//     Serial.printf("Turning off Wemo%i\n", WEMO2)
//     wemoWrite(WEMO2, LOW)
  

//   // SET HUMIDIFIER TO AUTO READ //
//   if (humidRH < 31.0)
//   { // turn on humidifier
//     Serial.printf("Turning on Wemo# %i\n", WEMO1);
//     wemoWrite(WEMO1, HIGH);
//   }
// }

// DECLARING THE PIXELFILL FUNCTION //
void pixelFill(int startP, int endP, int hex)
{
  for (int i = startP; i <= endP; i++)
  {
    pixel.setPixelColor(i, hex);
    pixel.show();
    delay(100);
  }
}