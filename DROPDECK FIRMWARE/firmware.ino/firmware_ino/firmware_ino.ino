#include <Adafruit_TinyUSB.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <RotaryEncoder.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
RotaryEncoder encoder(D2, D1, RotaryEncoder::LatchMode::FOUR3);

const int sliderPin = A0; 
const int switchPins[5] = {D6, D7, D8, D9, D10};

int currentLayer = 0;
bool lastSwitch5State = HIGH;
int lastEncoderPos = 0;
int currentVolumeLevel = 15; 

uint8_t const desc_hid_report[] = {
  TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(1)),
  TUD_HID_REPORT_DESC_MOUSE(HID_REPORT_ID(2)),
  TUD_HID_REPORT_DESC_CONSUMER(HID_REPORT_ID(3))
};

Adafruit_USBD_HID usb_hid(desc_hid_report, sizeof(desc_hid_report), HID_ITF_PROTOCOL_NONE, 2, false);

void sendConsumerKey(uint16_t usage_code) {
  usb_hid.sendReport(3, &usage_code, 2);
  uint16_t release = 0;
  usb_hid.sendReport(3, &release, 2);
  delay(10); 
}

void updateDisplay() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  
  display.setTextSize(2);
  display.setCursor(0, 0);
  if (currentLayer == 0) {
    display.print("CHILL");
  } else {
    display.print("BEAST");
  }
  
  display.setTextSize(1);
  display.setCursor(0, 20);
  if (currentLayer == 0) {
    display.print("DESKTOP MODE");
  } else {
    display.print("TACTICAL LOADOUT");
  }
  
  display.display();
}

void setup() {
  usb_hid.begin();
  
  Wire.setSDA(D4);
  Wire.setSCL(D5);
  Wire.begin();
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    for(;;);
  }
  
  updateDisplay();
  
  for (int i = 0; i < 5; i++) {
    pinMode(switchPins[i], INPUT_PULLUP);
  }
  
  while( !TinyUSBDevice.mounted() ) delay(1);
}

void loop() {
  if ( !usb_hid.ready() ) return;

  encoder.tick();
  int currentEncoderPos = encoder.getPosition();
  if (currentEncoderPos != lastEncoderPos) {
    if (currentLayer == 1) {
      if (currentEncoderPos > lastEncoderPos) {
        int8_t scroll_step = 1;
        usb_hid.mouseReport(2, 0, 0, 0, scroll_step, 0);
      } else {
        int8_t scroll_step = -1;
        usb_hid.mouseReport(2, 0, 0, 0, scroll_step, 0);
      }
    } else {
      if (currentEncoderPos > lastEncoderPos) {
        sendConsumerKey(HID_USAGE_CONSUMER_VOLUME_INCREMENT);
        if(currentVolumeLevel < 25) currentVolumeLevel++;
      } else {
        sendConsumerKey(HID_USAGE_CONSUMER_VOLUME_DECREMENT);
        if(currentVolumeLevel > 0) currentVolumeLevel--;
      }
    }
    lastEncoderPos = currentEncoderPos;
  }

  int rawSlider = analogRead(sliderPin);
  int targetVolumeLevel = map(rawSlider, 0, 1023, 0, 25); 

  if (abs(targetVolumeLevel - currentVolumeLevel) >= 1) {
    while (currentVolumeLevel < targetVolumeLevel) {
      sendConsumerKey(HID_USAGE_CONSUMER_VOLUME_INCREMENT);
      currentVolumeLevel++;
    }
    while (currentVolumeLevel > targetVolumeLevel) {
      sendConsumerKey(HID_USAGE_CONSUMER_VOLUME_DECREMENT);
      currentVolumeLevel--;
    }
  }

  bool switch5State = digitalRead(switchPins[4]);
  if (switch5State == LOW && lastSwitch5State == HIGH) {
    currentLayer = 1 - currentLayer;
    updateDisplay();
    delay(250);
  }
  lastSwitch5State = switch5State;

  uint8_t keycode[6] = { 0 };

  if (currentLayer == 0) {
    if (digitalRead(switchPins[0]) == LOW) {
      keycode[0] = HID_KEY_T;
      usb_hid.keyboardReport(1, KEYBOARD_MODIFIER_LEFTCTRL, keycode);
      delay(10);
      usb_hid.keyboardRelease(1);
      while(digitalRead(switchPins[0]) == LOW);
    }
    if (digitalRead(switchPins[1]) == LOW) {
      keycode[0] = HID_KEY_W;
      usb_hid.keyboardReport(1, KEYBOARD_MODIFIER_LEFTCTRL, keycode);
      delay(10);
      usb_hid.keyboardRelease(1);
      while(digitalRead(switchPins[1]) == LOW);
    }
    if (digitalRead(switchPins[2]) == LOW) {
      keycode[0] = HID_KEY_S;
      usb_hid.keyboardReport(1, (KEYBOARD_MODIFIER_LEFTCTRL | KEYBOARD_MODIFIER_LEFTSHIFT), keycode);
      delay(10);
      usb_hid.keyboardRelease(1);
      while(digitalRead(switchPins[2]) == LOW);
    }
    if (digitalRead(switchPins[3]) == LOW) {
      keycode[0] = HID_KEY_D;
      usb_hid.keyboardReport(1, KEYBOARD_MODIFIER_LEFTGUI, keycode);
      delay(10);
      usb_hid.keyboardRelease(1);
      while(digitalRead(switchPins[3]) == LOW);
    }
  } 
  else if (currentLayer == 1) {
    if (digitalRead(switchPins[0]) == LOW) {
      keycode[0] = HID_KEY_Q;
      usb_hid.keyboardReport(1, 0, keycode);
      delay(10);
      usb_hid.keyboardRelease(1);
      while(digitalRead(switchPins[0]) == LOW);
    }
    if (digitalRead(switchPins[1]) == LOW) {
      keycode[0] = HID_KEY_E;
      usb_hid.keyboardReport(1, 0, keycode);
      delay(10);
      usb_hid.keyboardRelease(1);
      while(digitalRead(switchPins[1]) == LOW);
    }
    if (digitalRead(switchPins[2]) == LOW) {
      keycode[0] = HID_KEY_R;
      usb_hid.keyboardReport(1, 0, keycode);
      delay(10);
      usb_hid.keyboardRelease(1);
      while(digitalRead(switchPins[2]) == LOW);
    }
    if (digitalRead(switchPins[3]) == LOW) {
      keycode[0] = HID_KEY_F;
      usb_hid.keyboardReport(1, 0, keycode);
      delay(10);
      usb_hid.keyboardRelease(1);
      while(digitalRead(switchPins[3]) == LOW);
    }
  }

  delay(1);
}