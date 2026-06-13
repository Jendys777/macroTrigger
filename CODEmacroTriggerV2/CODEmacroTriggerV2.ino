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
#define LED_COUNT 13 // Fixed to 13 LEDs
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

// Button B Toggle Logic
bool bToggleState = false;  
const int BUTTON_B_LED = 10; // Change this to the exact LED index for Button B if needed

unsigned long lastMPressTime = 0;
int mPressCount = 0;

unsigned long lastLEDUpdateTime[3] = { 0, 0, 0 };
const unsigned long LEDUpdateInterval = 100;

//Saved variables
unsigned long mode = 1;
int ledA = 5;
int menu = 0;
int LED = 2;
bool mute = false;
bool enableL = false;
bool enable = true;
bool Light = true;
String function = "";
int bright = 100;

char preset1 = '1'; String pre1 = "Mode 1";
char preset2 = '2'; String pre2 = "Mode 2";
char preset3 = '3'; String pre3 = "Mode 3";
char preset4 = '4'; String pre4 = "Mode 4";

unsigned long currentMillis = millis();

void setup() {
  Serial.begin(9600);
  strip.begin();
  EEPROM.get(10, bright);
  strip.setBrightness(bright);
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
  for (int i = 0; i < LED_COUNT; i++) {
    strip.setPixelColor(i, strip.Color(255, 0, 0));
    strip.show();
    delay(166);
  }
  EEPROM.get(0, mode);
  EEPROM.get(5, LED);
}

void loop() {
  currentMillis = millis();

  //Check keypad at interval
  if (currentMillis - lastKeyCheck >= keyCheckInterval) {
    lastKeyCheck = currentMillis;
    char key = customKeypad.getKey();
    if (key) {
      keyPress(key);
    }
  }

  // Handle Encoder Rotation logic simplified
  long newPosition = myEnc.read() / 4;
  long encDiff = newPosition - lastPosition;

  switch (menu) {
    case 0: {
      // Volume Control
      if (encDiff > 0) Consumer.write(MEDIA_VOLUME_UP);
      else if (encDiff < 0) Consumer.write(MEDIA_VOLUME_DOWN);

      // Play / Pause
      bool buttonState = digitalRead(ENCODER_SW);
      if (buttonState == LOW && lastButtonState == HIGH) {
        Consumer.write(MEDIA_PLAY_PAUSE);
        delay(200);
      }
      lastButtonState = buttonState;

      enable = false;
      enableL = false; // FIX: Ensure LED editing resets when returning to main menu!

      // Handle Display & LEDs
      if (currentMillis - lastLEDUpdateTime[0] >= LEDUpdateInterval) {
        lastLEDUpdateTime[0] = currentMillis;
        setColor(LED);
        display.clearDisplay();
        display.setTextColor(1);
        display.setTextSize(1);
        display.setTextWrap(false);
        display.setCursor(4, 15);
        display.print(function);
        setDisplay(mode);
      }
      break;
    }
    case 1: {
      // Mode Selection
      if (encDiff > 0) mode = min(mode + 1, 4UL);
      else if (encDiff < 0) mode = max(mode - 1, 1UL);

      if (encDiff != 0) EEPROM.put(0, mode);

      enable = true;
      enableL = false;

      if (currentMillis - lastLEDUpdateTime[1] >= LEDUpdateInterval) {
        lastLEDUpdateTime[1] = currentMillis;
        setColor(LED);
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextWrap(false);
        display.setCursor(33, 2);
        display.print("Select MODE");
        setDisplay(mode);
      }
      break;
    }
    case 2: {
      // Brightness Control
      if (encDiff > 0) bright = min(bright + 10, 250);
      else if (encDiff < 0) bright = max(bright - 10, 0);

      if (encDiff != 0) {
        strip.setBrightness(bright);
        EEPROM.put(10, bright);
      }

      enable = false;
      enableL = true; // LED Editor Mode Enabled

      if (currentMillis - lastLEDUpdateTime[2] >= LEDUpdateInterval) {
        lastLEDUpdateTime[2] = currentMillis;
        setColor(LED);
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextWrap(false);
        display.setCursor(33, 32);
        display.print("LED Editor");
        display.display();

        if (LED >= 9) LED = 2;
        else if (LED <= 1) LED = 8;
      }
      break;
    }
  }

  // Update last position if encoder moved
  if (encDiff != 0) {
    lastPosition = newPosition;
  }
}

void setColor(int LED) {
  uint32_t c1 = strip.Color(255, 255, 255);
  uint32_t c2;

  // Determine secondary color based on LED state
  switch (LED) {
    case 2: c2 = strip.Color(255, 255, 255); break;
    case 3: c2 = strip.Color(255, 0, 0); break;
    case 4: c2 = strip.Color(255, 255, 0); break;
    case 5: c2 = strip.Color(0, 255, 0); break;
    case 6: c2 = strip.Color(0, 255, 255); break;
    case 7: c2 = strip.Color(0, 0, 255); break;
    case 8: c2 = strip.Color(255, 0, 255); break;
    default: c2 = strip.Color(255, 255, 255); break;
  }

  // Apply colors
  for (int i = 0; i < LED_COUNT; i++) {
    strip.setPixelColor(i, c1);
    if (i >= 5) { // Safe index check
      strip.setPixelColor(i - 5, c2); 
    }
  }

  // OVERRIDE: Button B LED Color logic
  if (bToggleState) {
    strip.setPixelColor(BUTTON_B_LED, strip.Color(255, 0, 0)); // Turn Red
  } else {
    strip.setPixelColor(BUTTON_B_LED, strip.Color(255, 255, 255)); // Turn White
  }

  strip.show();
}

void setDisplay(int mode) {
  if (enable == true) {
    display.setTextColor(1);
    display.setCursor(4, 15);
    if (mode == 1) display.print(pre1);
    else if (mode == 2) display.print(pre2);
    else if (mode == 3) display.print(pre3);
    else if (mode == 4) display.print(pre4);
  }

  // Draw preset labels with correct sizing and coordinates
  display.setTextSize(mode == 1 ? 3 : 2);
  display.setCursor(mode == 1 ? 8 : 5, mode == 1 ? 41 : 48);
  display.println(preset1);

  display.setTextSize(mode == 2 ? 3 : 2);
  display.setCursor(mode == 2 ? 41 : (mode == 1 ? 47 : 38), mode == 2 ? 39 : 48);
  display.println(preset2);

  display.setTextSize(mode == 3 ? 3 : 2);
  display.setCursor(mode == 3 ? 73 : (mode == 4 ? 71 : 82), mode == 3 ? 39 : 48);
  display.println(preset3);

  display.setTextSize(mode == 4 ? 3 : 2);
  display.setCursor(mode == 4 ? 105 : 113, mode == 4 ? 39 : 48);
  display.println(preset4);

  display.display();
  strip.show();
}

// Handles system keys (A, B, C, D) which are identical across all modes
void handleControlKeys(char key) {
  switch (key) {
    case 'A': {
      unsigned long gg = millis();
      if (gg - lastMPressTime < doublePressInterval) mPressCount++;
      else mPressCount = 1;
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
    case 'B': {
      unsigned long now = millis();
      if (now - lastBPressTime < doublePressInterval) bPressCount++;
      else bPressCount = 1;
      lastBPressTime = now;

      if (bPressCount == 1) {
        BootKeyboard.press(KEY_F13);
        delay(10);
        BootKeyboard.release(KEY_F13);
        bToggleState = !bToggleState; // Toggle color state
      } else if (bPressCount == 2) {
        BootKeyboard.press(KEY_F14);
        delay(10);
        BootKeyboard.release(KEY_F14);
        bPressCount = 0;
      }
      break;
    }
    case 'C': {
      if (enableL) {
        LED = LED + 1;
        EEPROM.put(5, LED);
      } else {
        Consumer.write(MEDIA_NEXT);
      }
      break;
    }
    case 'D': {
      if (enableL) {
        LED = LED - 1;
        EEPROM.put(5, LED);
      } else {
        Consumer.write(MEDIA_PREVIOUS);
      }
      break;
    }
  }
}

void keyPress(char key) {
  // If the key is a control key (A, B, C, D), offload logic to shared function
  if (key == 'A' || key == 'B' || key == 'C' || key == 'D') {
    handleControlKeys(key);
    return;
  }

  // Convert char '1'-'8' to index 0-7
  int keyNum = key - '1'; 

  if (keyNum >= 0 && keyNum <= 7) {
    
    // 1. APPLY MODIFIERS BASED ON MODE
    if (mode == 2) {
      BootKeyboard.press(KEY_LEFT_CTRL);
    } else if (mode == 3) {
      BootKeyboard.press(KEY_LEFT_CTRL);
      BootKeyboard.press(KEY_LEFT_ALT);
    } else if (mode == 4) {
      if (key == '1' || key == '3' || key == '4' || key == '5' || key == '6') {
        BootKeyboard.press(KEY_LEFT_CTRL);
        BootKeyboard.press(KEY_LEFT_ALT);
      } else if (key == '2') {
        BootKeyboard.press(KEY_LEFT_SHIFT);
      } else if (key == '7' || key == '8') {
        BootKeyboard.press(KEY_LEFT_CTRL);
        BootKeyboard.press(KEY_LEFT_GUI);
      }
    }

    // 2. APPLY BASE KEYS
    KeyboardKeycode fKeys[] = {KEY_F15, KEY_F16, KEY_F17, KEY_F18, KEY_F19, KEY_F20, KEY_F21, KEY_F22};
    
    if (mode == 1 || mode == 2 || mode == 3) {
      BootKeyboard.press(fKeys[keyNum]);
      function = "Function " + String(key);
    } else if (mode == 4) {
      switch (key) {
        case '1': BootKeyboard.press(KEYPAD_1); function = "Function 1"; break;
        case '2': BootKeyboard.press(KEY_DELETE); function = "Delete"; break;
        case '3': BootKeyboard.press(KEYPAD_3); function = "Function 3"; break;
        case '4': BootKeyboard.press(KEYPAD_4); function = "Function 4"; break;
        case '5': BootKeyboard.press(KEYPAD_5); function = "Function 5"; break;
        case '6': BootKeyboard.press(KEYPAD_6); function = "Function 6"; break;
        case '7': BootKeyboard.press(KEY_LEFT_ARROW); function = "D Left"; break;
        case '8': BootKeyboard.press(KEY_RIGHT_ARROW); function = "D Right"; break;
      }
    }

    // 3. RELEASE
    delay(10);
    BootKeyboard.releaseAll();
  }
}