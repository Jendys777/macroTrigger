/*
This code is for the macroTrigger keyboard. You can find addicional information on GitHub: https://github.com/Jendys777/macroTrigger

You can also find 3D models on printables.
If you have any questions go contact me.
This code is pretty demanding on the Arduino Pro Micro programe memory. It takes up 99% of it, so you will no be able to do a lot of changes. 
If you have any recomendations just contact me.

Created in April 2025 by Jendys and KNC
*/

//Libraries
#include <Adafruit_NeoPixel.h>
#include <Keypad.h>
#include <Encoder.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <HID-Project.h>
#include <HID-Settings.h>
#include <EEPROM.h>

//NeoPixel Setup
#define LED_PIN A3
#define LED_COUNT 12
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

//OLED Display Setup
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//Keypad Setup
const byte ROWS = 4;
const byte COLS = 3;
char hexaKeys[ROWS][COLS] = {
  { '1', '2', 'A' },
  { '3', '4', 'B' },
  { '5', '6', 'C' },
  { '7', '8', 'D' }
};
byte rowPins[ROWS] = { 15, 14, 16, 10 };
byte colPins[COLS] = { A2, A1, A0 };
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

//Encoder Setup
#define ENCODER_CLK 7
#define ENCODER_DT 6
#define ENCODER_SW 5
Encoder myEnc(ENCODER_CLK, ENCODER_DT);
long lastPosition = 0;
bool lastButtonState = HIGH;


//Timers
unsigned long lastKeyCheck = 0;
const unsigned long keyCheckInterval = 50;

unsigned long lastBPressTime = 0;
int bPressCount = 0;
const unsigned long doublePressInterval = 500;

unsigned long lastMPressTime = 0;
int mPressCount = 0;

unsigned long lastLEDUpdateTime[3] = { 0, 0, 0 };  // Separate timers for LED updates for each mode
const unsigned long LEDUpdateInterval = 100;       // Set NeoPixel update interval

//saved variables

unsigned long mode = 1;
int ledA = 5;
int menu = 0;
int LED = 2;
bool enableL = false;
bool enable = true;
bool Light = true;
String function = "";
int bright = 100;
int fStart = 0;


//---------------------------------------------------------Here you can change basic preset name and other, for more options go to line:     --------------------------------------------------------------------------------------------------------------------

char preset1 = '1';
String pre1 = "Mode 1";

char preset2 = '2';
String pre2 = "Mode 2";

char preset3 = '3';
String pre3 = "Mode 3";

char preset4 = '4';
String pre4 = "Mode 4";

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

unsigned long currentMillis = millis();

void setup() {
  //Setup
  Serial.begin(9600);
  strip.begin();
  strip.show();

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  BootKeyboard.begin();
  Consumer.begin();

  //Startup animation Display
  display.clearDisplay();
  display.setTextColor(1);
  display.setTextSize(2);
  display.setCursor(35, 8);
  display.print("macro");

  display.setCursor(23, 28);
  display.print("Trigger");

  display.setTextSize(1);
  display.setCursor(16, 54);
  display.print("By: Jendys & KNC");

  display.display();

  //LEDs Startup
  for (int i = 0; i <= 12; i++) {
    strip.setPixelColor(i, strip.Color(0, 50, 50));  //Purple for mode 1
    strip.show();
    delay(166);
  }
  EEPROM.get(0, mode);
  EEPROM.get(5, LED);
  EEPROM.get(10, bright);
  EEPROM.get(15, ledA);
  EEPROM.get(20, fStart);
  strip.setBrightness(bright);
  if (fStart == 0) {
    menu = 0;
    mode = 1;
    bright = 100;
    LED = 8;
    ledA = 5;
    fStart = fStart + 5;
    EEPROM.put(20, fStart);
  }
}


void loop() {
  //Timers update
  currentMillis = millis();

  //Check keypad at interval
  if (currentMillis - lastKeyCheck >= keyCheckInterval) {
    lastKeyCheck = currentMillis;
    char key = customKeypad.getKey();
    if (key) {
      keyPress(key);
    }
  }

  //Handle mode-specific logic and timing
  switch (menu) {
    case 0:
      {

        //Encoder Rotation Volume control
        long newPosition = myEnc.read() / 4;  //Some encoders have 4 steps per detent
        if (newPosition != lastPosition) {
          if (newPosition > lastPosition) {
            Consumer.write(MEDIA_VOLUME_UP);
          } else {
            Consumer.write(MEDIA_VOLUME_DOWN);
          }
          lastPosition = newPosition;
        }

        //Encoder Button Press Play and pause
        bool buttonState = digitalRead(ENCODER_SW);
        if (buttonState == LOW && lastButtonState == HIGH) {
          Consumer.write(MEDIA_PLAY_PAUSE);
          delay(200);  //debounce
        }
        lastButtonState = buttonState;


        //Handle display update timing for case 0
        enable = false;

        //Handle LED update timing for case 0
        if (currentMillis - lastLEDUpdateTime[0] >= LEDUpdateInterval) {
          lastLEDUpdateTime[0] = currentMillis;
          setColor(LED, ledA);
          display.clearDisplay();
          display.setTextColor(1);
          display.setTextSize(1);
          display.setTextWrap(false);
          display.setCursor(4, 20);
          display.print(function);
          setDisplay(mode);
        }

        break;
      }
    case 1:
      {

        //Encoder Rotation
        long newPosition2 = myEnc.read() / 4;  //Some encoders have 4 steps per detent
        if (newPosition2 != lastPosition) {
          if (newPosition2 > lastPosition) {
            if (mode >= 4) {
              mode = 4;
              EEPROM.put(0, mode);
            } else {
              mode = mode + 1;
              EEPROM.put(0, mode);
            }
          } else {
            if (mode <= 1) {
              mode = 1;
              EEPROM.put(0, mode);
            } else {
              mode = mode - 1;
              EEPROM.put(0, mode);
            }
          }
          lastPosition = newPosition2;
        }

        //Encoder Button Press


        //Handle display update timing for case 1
        enable = true;
        enableL = false;

        //Handle LED update timing for case 1
        if (currentMillis - lastLEDUpdateTime[1] >= LEDUpdateInterval) {
          lastLEDUpdateTime[1] = currentMillis;
          setColor(LED, ledA);
          display.clearDisplay();
          display.setTextSize(1);
          display.setTextWrap(false);
          display.setCursor(33, 2);
          display.print("Select MODE");
          setDisplay(mode);
        }

        break;
      }
    case 2:
      {
        //Encoder Rotation
        long newPosition3 = myEnc.read() / 4;  //Some encoders have 4 steps per detent
        if (newPosition3 != lastPosition) {
          if (newPosition3 > lastPosition) {
            if (bright >= 250) {
              bright = 250;
              strip.setBrightness(bright);
              EEPROM.put(10, bright);
            } else {
              bright = bright + 10;
              strip.setBrightness(bright);
              EEPROM.put(10, bright);
            }
          } else {
            if (bright <= 0) {
              bright = 0;
              strip.setBrightness(bright);
              EEPROM.put(10, bright);
            } else {
              bright = bright - 10;
              strip.setBrightness(bright);
              EEPROM.put(10, bright);
            }
          }
          lastPosition = newPosition3;
        }

        //Encoder Button Press
        bool buttonState2 = digitalRead(ENCODER_SW);
        if (buttonState2 == LOW && lastButtonState == HIGH) {
          if (ledA == 5) {
            ledA = 0;
            EEPROM.put(15, ledA);
          } else {
            ledA = 5;
            EEPROM.put(15, ledA);
          }
          delay(200);  //Debounce
        }
        lastButtonState = buttonState2;

        //Handle display update timing for case 1
        enable = false;
        enableL = true;

        //Handle LED update timing for case 1
        if (currentMillis - lastLEDUpdateTime[1] >= LEDUpdateInterval) {
          lastLEDUpdateTime[1] = currentMillis;
          setColor(LED, ledA);
          display.clearDisplay();
          display.setTextSize(1);
          display.setTextWrap(false);
          display.setCursor(5, 32);
          display.print("Mediaskip and encoder");
          display.setCursor(5, 5);
          display.print("LED Editor");
          display.display();

          if (LED >= 14) {
            LED = 1;
          } else if (LED <= 0) {
            LED = 13;
          }
        }
        break;
      }
  }
}


void setColor(int LED, int ledA) {
  //LEDs Selector(case=colour or colour combination with white, can be turned on/off by pressing the encoder)
  switch (LED) {
    case 1:
      for (int i = 0; i <= 12; i++) {
        strip.setPixelColor(i, strip.Color(255, 255, 255));
      }
      break;
    case 2:
      for (int i = 0; i <= 12; i++) {
        strip.setPixelColor(i, strip.Color(255, 255, 255));
        strip.setPixelColor(i - ledA, strip.Color(255, 0, 0));
      }
      break;
    case 3:
      for (int i = 0; i <= 12; i++) {
        strip.setPixelColor(i, strip.Color(255, 255, 255));
        strip.setPixelColor(i - ledA, strip.Color(255, 128, 50));
      }
      break;
    case 4:
      for (int i = 0; i <= 12; i++) {
        strip.setPixelColor(i, strip.Color(255, 255, 255));
        strip.setPixelColor(i - ledA, strip.Color(255, 255, 0));
      }
      break;
    case 5:
      for (int i = 0; i <= 12; i++) {
        strip.setPixelColor(i, strip.Color(255, 255, 255));
        strip.setPixelColor(i - ledA, strip.Color(128, 255, 0));
      }
      break;
    case 6:
      for (int i = 0; i <= 12; i++) {
        strip.setPixelColor(i, strip.Color(255, 255, 255));
        strip.setPixelColor(i - ledA, strip.Color(0, 255, 0));
      }
      break;
    case 7:
      for (int i = 0; i <= 12; i++) {
        strip.setPixelColor(i, strip.Color(255, 255, 255));
        strip.setPixelColor(i - ledA, strip.Color(0, 255, 128));
      }
      break;
    case 8:
      for (int i = 0; i <= 12; i++) {
        strip.setPixelColor(i, strip.Color(255, 255, 255));
        strip.setPixelColor(i - ledA, strip.Color(0, 255, 255));
      }
      break;
    case 9:
      for (int i = 0; i <= 12; i++) {
        strip.setPixelColor(i, strip.Color(255, 255, 255));
        strip.setPixelColor(i - ledA, strip.Color(0, 128, 255));
      }
      break;
    case 10:
      for (int i = 0; i <= 12; i++) {
        strip.setPixelColor(i, strip.Color(255, 255, 255));
        strip.setPixelColor(i - ledA, strip.Color(0, 0, 255));
      }
      break;
    case 11:
      for (int i = 0; i <= 12; i++) {
        strip.setPixelColor(i, strip.Color(255, 255, 255));
        strip.setPixelColor(i - ledA, strip.Color(128, 0, 255));
      }
      break;
    case 12:
      for (int i = 0; i <= 12; i++) {
        strip.setPixelColor(i, strip.Color(255, 255, 255));
        strip.setPixelColor(i - ledA, strip.Color(255, 0, 255));
      }
      break;
    case 13:
      for (int i = 0; i <= 12; i++) {
        strip.setPixelColor(i, strip.Color(255, 255, 255));
        strip.setPixelColor(i - ledA, strip.Color(255, 0, 128));
      }
      break;
  }
  strip.show();  //Start the LEDs lights
}


//Setup of display function (here you can setup what to show on display when you do something)
void setDisplay(int mode) {
  switch (mode) {
    case 1:  //Mode 1
      if (enable == true) {
        display.setTextColor(1);
        display.setCursor(4, 20);
        display.print(pre1);  //Name of the first mode text
      }
      display.setTextSize(3.5);
      display.setCursor(8, 41);
      display.println(preset1);  //Sign for the first mode

      display.setTextSize(2.5);
      display.setCursor(47, 48);
      display.println(preset2);  //Sign for the second mode

      display.setCursor(82, 48);
      display.println(preset3);  //Sign for the third mode

      display.setCursor(113, 48);
      display.println(preset4);  //Sign for the fourth mode
      display.display();
      break;

    case 2:  //Mode 2
      if (enable == true) {
        display.setTextColor(1);
        display.setCursor(4, 20);
        display.print(pre2);  //Name of the second mode text
      }
      display.setTextSize(2.5);
      display.setCursor(5, 48);
      display.println(preset1);  //Sign for the first mode

      display.setTextSize(3.5);
      display.setCursor(41, 39);
      display.println(preset2);  //Sign for the second mode

      display.setTextSize(2.5);
      display.setCursor(82, 48);
      display.println(preset3);  //Sign for the third mode

      display.setCursor(113, 48);
      display.println(preset4);  //Sign for the fourth mode
      display.display();
      break;

    case 3:  //Mode 3
      if (enable == true) {
        display.setTextColor(1);
        display.setCursor(4, 20);
        display.print(pre3);  //Name of the third mode text
      }

      display.setTextSize(2.5);
      display.setCursor(5, 48);
      display.println(preset1);  //Sign for the first mode

      display.setCursor(38, 48);
      display.println(preset2);  //Sign for the second mode

      display.setTextSize(3.5);
      display.setCursor(73, 39);
      display.println(preset3);  //Sign for the third mode

      display.setTextSize(2.5);
      display.setCursor(113, 48);
      display.println(preset4);  //Sign for the fourth mode
      display.display();
      break;

    case 4:  //Mode 4
      if (enable == true) {
        display.setTextColor(1);
        display.setCursor(4, 20);
        display.print(pre4);  //Name of the fourth mode text
      }

      display.setTextSize(2.5);
      display.setCursor(5, 48);
      display.println(preset1);  //Sign for the first mode

      display.setCursor(38, 48);
      display.println(preset2);  //Sign for the second mode

      display.setCursor(71, 48);
      display.println(preset3);  //Sign for the third mode

      display.setTextSize(3.5);
      display.setCursor(105, 39);
      display.println(preset4);  //Sign for the fourth mode
      display.display();
      break;
  }
  strip.show();
}

/*
  Layout of the keys
/=======================/
/ (1) (2)  /    /  (A)  /
/ (3) (4)  /    /  (B)  /
/ (5) (6)   (E)    (C)  /
/ (7) (8)          (D)  /
/=======================/
*/

//Custom Key Mapping (here you can set your custom key or action that will apper on the dipsley after triggering them)
void keyPress(char key) {
  switch (mode) {
    case 1:  //Mode 1
      switch (key) {

        case '1':  //Key1 in Mode 1
          BootKeyboard.press(KEY_F15);
          delay(10);
          BootKeyboard.release(KEY_F15);
          function = "Function 1";
          break;

        case '2':  //Key2 in Mode 1
          BootKeyboard.press(KEY_F16);
          delay(10);
          BootKeyboard.release(KEY_F16);
          function = "Function 2";
          break;

        case '3':  //Key3 in Mode 1
          BootKeyboard.press(KEY_F17);
          delay(10);
          BootKeyboard.release(KEY_F17);
          function = "Function 3";
          break;

        case '4':  //Key4 in Mode 1
          BootKeyboard.press(KEY_F18);
          delay(10);
          BootKeyboard.release(KEY_F18);
          function = "Function 4";
          break;

        case '5':  //Key5 in Mode 1
          BootKeyboard.press(KEY_F19);
          delay(10);
          BootKeyboard.release(KEY_F19);
          function = "Function 5";
          break;

        case '6':  //Key6 in Mode 1
          BootKeyboard.press(KEY_F20);
          delay(10);
          BootKeyboard.release(KEY_F20);
          function = "Function 6";
          break;

        case '7':  //Key7 in Mode 1
          BootKeyboard.press(KEY_F21);
          delay(10);
          BootKeyboard.release(KEY_F21);
          function = "Function 7";
          break;

        case '8':  //Key8 in Mode 1
          BootKeyboard.press(KEY_F22);
          delay(10);
          BootKeyboard.release(KEY_F22);
          function = "Function 8";
          break;

        case 'A':  //KeyA in Mode 1 and also in all others mods !DO NOT CHANGE ENYTHING IN HERE, UNLESS YOU KNOW WHAT ARE YOU DOING!
          {
            unsigned long gg = millis();
            if (gg - lastMPressTime < doublePressInterval) {
              mPressCount++;
            } else {
              mPressCount = 1;
            }
            lastMPressTime = gg;

            if (mPressCount == 1 && menu == 0) {
              menu = 1;
              EEPROM.get(0, mode);
            } else if (mPressCount == 2 && menu == 1) {
              menu = 2;
              mPressCount = 0;
            } else if (menu >= 1) {
              menu = 0;
              EEPROM.get(0, mode);
            }
            break;
          }


        case 'B':  //KeyB in Mode 1 and also in all others mods !DO NOT CHANGE ENYTHING IN HERE, UNLESS YOU KNOW WHAT ARE YOU DOING!
          {
            unsigned long now = millis();
            if (now - lastBPressTime < doublePressInterval) {
              bPressCount++;
            } else {
              bPressCount = 1;
            }
            lastBPressTime = now;

            if (bPressCount == 1) {
              BootKeyboard.press(KEY_F13);
              delay(10);
              BootKeyboard.release(KEY_F13);
            } else if (bPressCount == 2) {
              BootKeyboard.press(KEY_F14);
              delay(10);
              BootKeyboard.release(KEY_F14);
              bPressCount = 0;
            }
            break;
          }

        case 'C':  //KeyC in Mode 1 and also in all others mods !DO NOT CHANGE ENYTHING IN HERE, UNLESS YOU KNOW WHAT ARE YOU DOING!
          if (enableL) {
            LED = LED + 1;
            EEPROM.put(5, LED);
          } else {
            Consumer.write(MEDIA_NEXT);
          }
          break;

        case 'D':
          if (enableL) {
            LED = LED - 1;
            EEPROM.put(5, LED);
          } else {
            Consumer.write(MEDIA_PREVIOUS);
          }
          break;
      }
      break;
    case 2:  //Mode 2
      switch (key) {

        case '1':  //Key1 in Mode 2
          BootKeyboard.press(KEY_LEFT_CTRL);
          BootKeyboard.press(KEY_F15);
          delay(10);
          BootKeyboard.releaseAll();
          function = "Function 1";
          break;

        case '2':  //Key2 in Mode 2
          BootKeyboard.press(KEY_LEFT_CTRL);
          BootKeyboard.press(KEY_F16);
          delay(10);
          BootKeyboard.releaseAll();
          function = "Function 2";
          break;

        case '3':  //Key3 in Mode 2
          BootKeyboard.press(KEY_LEFT_CTRL);
          BootKeyboard.press(KEY_F17);
          delay(10);
          BootKeyboard.releaseAll();
          function = "Function 3";
          break;

        case '4':  //Key4 in Mode 2
          BootKeyboard.press(KEY_LEFT_CTRL);
          BootKeyboard.press(KEY_F18);
          delay(10);
          BootKeyboard.releaseAll();
          function = "Function 4";
          break;

        case '5':  //Key5 in Mode 2
          BootKeyboard.press(KEY_LEFT_CTRL);
          BootKeyboard.press(KEY_F19);
          delay(10);
          BootKeyboard.releaseAll();
          function = "Function 5";
          break;

        case '6':  //Key6 in Mode 2
          BootKeyboard.press(KEY_LEFT_CTRL);
          BootKeyboard.press(KEY_F20);
          delay(10);
          BootKeyboard.releaseAll();
          function = "Function 6";
          break;

        case '7':  //Key7 in Mode 2
          BootKeyboard.press(KEY_LEFT_CTRL);
          BootKeyboard.press(KEY_F21);
          delay(10);
          BootKeyboard.releaseAll();
          function = "Function 7";
          break;

        case '8':  //Key8 in Mode 2
          BootKeyboard.press(KEY_LEFT_CTRL);
          BootKeyboard.press(KEY_F22);
          delay(10);
          BootKeyboard.releaseAll();
          function = "Function 8";
          break;

        case 'A':  //KeyA in Mode 2 and also in all others mods !DO NOT CHANGE ENYTHING IN HERE, UNLESS YOU KNOW WHAT ARE YOU DOING!
          {
            unsigned long gg = millis();
            if (gg - lastMPressTime < doublePressInterval) {
              mPressCount++;
            } else {
              mPressCount = 1;
            }
            lastMPressTime = gg;

            if (mPressCount == 1 && menu == 0) {
              menu = 1;
              EEPROM.get(0, mode);
            } else if (mPressCount == 2 && menu == 1) {
              menu = 2;
              mPressCount = 0;
            } else if (menu >= 1) {
              menu = 0;
              EEPROM.get(0, mode);
            }
            break;
          }

        case 'B':  //KeyB in Mode 2 and also in all others mods !DO NOT CHANGE ENYTHING IN HERE, UNLESS YOU KNOW WHAT ARE YOU DOING!
          {
            unsigned long now = millis();
            if (now - lastBPressTime < doublePressInterval) {
              bPressCount++;
            } else {
              bPressCount = 1;
            }
            lastBPressTime = now;

            if (bPressCount == 1) {
              BootKeyboard.press(KEY_F13);
              delay(10);
              BootKeyboard.release(KEY_F13);
            } else if (bPressCount == 2) {
              BootKeyboard.press(KEY_F14);
              delay(10);
              BootKeyboard.release(KEY_F14);
              bPressCount = 0;
            }
            break;
          }

        case 'C':  //KeyC in Mode 2 and also in all others mods !DO NOT CHANGE ENYTHING IN HERE, UNLESS YOU KNOW WHAT ARE YOU DOING!
          if (enableL) {
            LED = LED + 1;
            EEPROM.put(5, LED);
          } else {
            Consumer.write(MEDIA_NEXT);
          }
          break;

        case 'D':  //KeyD in Mode 2 and also in all others mods !DO NOT CHANGE ENYTHING IN HERE, UNLESS YOU KNOW WHAT ARE YOU DOING!
          if (enableL) {
            LED = LED - 1;
            EEPROM.put(5, LED);
          } else {
            Consumer.write(MEDIA_PREVIOUS);
          }
          break;
      }
      break;
    case 3:  //Mode 3
      switch (key) {

        case '1':  //Key1 in Mode 3
          BootKeyboard.press(KEY_LEFT_CTRL);
          BootKeyboard.press(KEY_LEFT_ALT);
          BootKeyboard.press(KEY_F15);
          delay(10);
          BootKeyboard.releaseAll();
          function = "Function 1";
          break;

        case '2':  //Key2 in Mode 3
          BootKeyboard.press(KEY_LEFT_CTRL);
          BootKeyboard.press(KEY_LEFT_ALT);
          BootKeyboard.press(KEY_F16);
          delay(10);
          BootKeyboard.releaseAll();
          function = "Function 2";
          break;

        case '3':  //Key3 in Mode 3
          BootKeyboard.press(KEY_LEFT_CTRL);
          BootKeyboard.press(KEY_LEFT_ALT);
          BootKeyboard.press(KEY_F17);
          delay(10);
          BootKeyboard.releaseAll();
          function = "Function 3";
          break;

        case '4':  //Key4 in Mode 3
          BootKeyboard.press(KEY_LEFT_CTRL);
          BootKeyboard.press(KEY_LEFT_ALT);
          BootKeyboard.press(KEY_F18);
          delay(10);
          BootKeyboard.releaseAll();
          function = "Function 4";
          break;

        case '5':  //Key5 in Mode 3
          BootKeyboard.press(KEY_LEFT_CTRL);
          BootKeyboard.press(KEY_LEFT_ALT);
          BootKeyboard.press(KEY_F19);
          delay(10);
          BootKeyboard.releaseAll();
          function = "Function 5";
          break;

        case '6':  //Key6 in Mode 3
          BootKeyboard.press(KEY_LEFT_CTRL);
          BootKeyboard.press(KEY_LEFT_ALT);
          BootKeyboard.press(KEY_F20);
          delay(10);
          BootKeyboard.releaseAll();
          function = "Function 6";
          break;

        case '7':  //Key7 in Mode 3
          BootKeyboard.press(KEY_LEFT_CTRL);
          BootKeyboard.press(KEY_LEFT_ALT);
          BootKeyboard.press(KEY_F21);
          delay(10);
          BootKeyboard.releaseAll();
          function = "Function 7";
          break;

        case '8':  //Key8 in Mode 3
          BootKeyboard.press(KEY_LEFT_CTRL);
          BootKeyboard.press(KEY_LEFT_ALT);
          BootKeyboard.press(KEY_F22);
          delay(10);
          BootKeyboard.releaseAll();
          function = "Function 8";
          break;

        case 'A':  //KeyA in Mode 3 and also in all others mods !DO NOT CHANGE ENYTHING IN HERE, UNLESS YOU KNOW WHAT ARE YOU DOING!
          {
            unsigned long gg = millis();
            if (gg - lastMPressTime < doublePressInterval) {
              mPressCount++;
            } else {
              mPressCount = 1;
            }
            lastMPressTime = gg;

            if (mPressCount == 1 && menu == 0) {
              menu = 1;
              EEPROM.get(0, mode);
            } else if (mPressCount == 2 && menu == 1) {
              menu = 2;
              mPressCount = 0;
            } else if (menu >= 1) {
              menu = 0;
              EEPROM.get(0, mode);
            }
            break;
          }

        case 'B':  //KeyB in Mode 3 and also in all others mods !DO NOT CHANGE ENYTHING IN HERE, UNLESS YOU KNOW WHAT ARE YOU DOING!
          {
            unsigned long now = millis();
            if (now - lastBPressTime < doublePressInterval) {
              bPressCount++;
            } else {
              bPressCount = 1;
            }
            lastBPressTime = now;

            if (bPressCount == 1) {
              BootKeyboard.press(KEY_F13);
              delay(10);
              BootKeyboard.release(KEY_F13);
            } else if (bPressCount == 2) {
              BootKeyboard.press(KEY_F14);
              delay(10);
              BootKeyboard.release(KEY_F14);
              bPressCount = 0;
            }
            break;
          }

        case 'C':  //KeyC in Mode 3 and also in all others mods !DO NOT CHANGE ENYTHING IN HERE, UNLESS YOU KNOW WHAT ARE YOU DOING!
          if (enableL) {
            LED = LED + 1;
            EEPROM.put(5, LED);
          } else {
            Consumer.write(MEDIA_NEXT);
          }
          break;

        case 'D':  //KeyD in Mode 3 and also in all others mods !DO NOT CHANGE ENYTHING IN HERE, UNLESS YOU KNOW WHAT ARE YOU DOING!
          if (enableL) {
            LED = LED - 1;
            EEPROM.put(5, LED);
          } else {
            Consumer.write(MEDIA_PREVIOUS);
          }
          break;
      }
      break;
    case 4:  //Mode 4
      switch (key) {

        case '1':  //Key1 in Mode 4
          BootKeyboard.press(KEY_LEFT_CTRL);
          BootKeyboard.press(KEY_LEFT_ALT);
          BootKeyboard.press(KEYPAD_1);
          delay(10);
          BootKeyboard.releaseAll();
          function = "Function 1";
          break;

        case '2':  //Key2 in Mode 4
          BootKeyboard.press(KEY_LEFT_CTRL);
          BootKeyboard.press(KEY_LEFT_ALT);
          BootKeyboard.press(KEYPAD_2);
          delay(10);
          BootKeyboard.releaseAll();
          function = "Function 2";
          break;

        case '3':  //Key3 in Mode 4
          BootKeyboard.press(KEY_LEFT_CTRL);
          BootKeyboard.press(KEY_LEFT_ALT);
          BootKeyboard.press(KEYPAD_3);
          delay(10);
          BootKeyboard.releaseAll();
          function = "Function 3";
          break;

        case '4':  //Key4 in Mode 4
          BootKeyboard.press(KEY_LEFT_CTRL);
          BootKeyboard.press(KEY_LEFT_ALT);
          BootKeyboard.press(KEYPAD_4);
          delay(10);
          BootKeyboard.releaseAll();
          function = "Function 4";
          break;

        case '5':  //Key5 in Mode 4
          BootKeyboard.press(KEY_LEFT_CTRL);
          BootKeyboard.press(KEY_LEFT_ALT);
          BootKeyboard.press(KEYPAD_5);
          delay(10);
          BootKeyboard.releaseAll();
          function = "Function 5";
          break;

        case '6':  //Key6 in Mode 4
          BootKeyboard.press(KEY_LEFT_CTRL);
          BootKeyboard.press(KEY_LEFT_ALT);
          BootKeyboard.press(KEYPAD_6);
          delay(10);
          BootKeyboard.releaseAll();
          function = "Function 6";
          break;

        case '7':  //Key7 in Mode 4
          BootKeyboard.press(KEY_LEFT_CTRL);
          BootKeyboard.press(KEY_LEFT_ALT);
          BootKeyboard.press(KEYPAD_7);
          delay(10);
          BootKeyboard.releaseAll();
          function = "Function 7";
          break;

        case '8':  //Key8 in Mode 4
          BootKeyboard.press(KEY_LEFT_CTRL);
          BootKeyboard.press(KEY_LEFT_ALT);
          BootKeyboard.press(KEYPAD_8);
          delay(10);
          BootKeyboard.releaseAll();
          function = "Function 8";
          break;

        case 'A':  //KeyD in Mode 4 and also in all others mods !DO NOT CHANGE ENYTHING IN HERE, UNLESS YOU KNOW WHAT ARE YOU DOING!
          {
            unsigned long gg = millis();
            if (gg - lastMPressTime < doublePressInterval) {
              mPressCount++;
            } else {
              mPressCount = 1;
            }
            lastMPressTime = gg;

            if (mPressCount == 1 && menu == 0) {
              menu = 1;
              EEPROM.get(0, mode);
            } else if (mPressCount == 2 && menu == 1) {
              menu = 2;
              mPressCount = 0;
            } else if (menu >= 1) {
              menu = 0;
              EEPROM.get(0, mode);
            }
            break;
          }

        case 'B':  //KeyD in Mode 4 and also in all others mods !DO NOT CHANGE ENYTHING IN HERE, UNLESS YOU KNOW WHAT ARE YOU DOING!
          {
            unsigned long now = millis();
            if (now - lastBPressTime < doublePressInterval) {
              bPressCount++;
            } else {
              bPressCount = 1;
            }
            lastBPressTime = now;

            if (bPressCount == 1) {
              BootKeyboard.press(KEY_F13);
              delay(10);
              BootKeyboard.release(KEY_F13);
            } else if (bPressCount == 2) {
              BootKeyboard.press(KEY_F14);
              delay(10);
              BootKeyboard.release(KEY_F14);
              bPressCount = 0;
            }
            break;
          }

        case 'C':  //KeyD in Mode 4 and also in all others mods !DO NOT CHANGE ENYTHING IN HERE, UNLESS YOU KNOW WHAT ARE YOU DOING!
          if (enableL) {
            LED = LED + 1;
            EEPROM.put(5, LED);
          } else {
            Consumer.write(MEDIA_NEXT);
          }
          break;

        case 'D':
          if (enableL) {
            LED = LED - 1;
            EEPROM.put(5, LED);
          } else {
            Consumer.write(MEDIA_PREVIOUS);
          }
          break;
      }
      break;
  }
}
