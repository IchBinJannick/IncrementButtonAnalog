#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <EEPROM.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>

#define button_pin 3
#define led_pin 2

// SSD1306 128x64 I2C
// A4 = SDA, A5 = SCL
U8G2_SSD1306_128X64_NONAME_F_HW_I2C dis(U8G2_R0, U8X8_PIN_NONE);

int counter = 0;
int buttonState = 0;
bool buttonWasPressed = false;
unsigned long lastActivity = 0;
const unsigned long SLEEP_AFTER_MS = 20000;
const unsigned long LONG_PRESS_MS = 5000;
unsigned long pressStart = 0;

void drawCounter()
{
  char buffer[10];
  sprintf(buffer, "%d", counter);

  dis.clearBuffer();
  dis.setFont(u8g2_font_7Segments_26x42_mn);

  int textWidth = dis.getStrWidth(buffer);
  int textHeight = dis.getAscent();

  int x = (128 - textWidth) / 2;
  int y = (64 + textHeight) / 2;

  dis.drawStr(x, y, buffer);
  dis.sendBuffer();
}

void goSleep()
{
  dis.clearBuffer();
  dis.sendBuffer();

  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();

  attachInterrupt(digitalPinToInterrupt(button_pin), []() {}, LOW);

  sei();       // activate interrutp
  sleep_cpu(); // SLEEP NGR!

  sleep_disable();
  detachInterrupt(digitalPinToInterrupt(button_pin));

  while (digitalRead(button_pin) == LOW)
  {
    delay(10);
  }
  delay(50);
  buttonWasPressed = (digitalRead(button_pin) == LOW);
  drawCounter();
}

void setup()
{
  EEPROM.get(0, counter);
  if (counter < 0 || counter > 9999)
    counter = 0;
  pinMode(led_pin, OUTPUT);
  pinMode(button_pin, INPUT_PULLUP);
  Wire.begin();
  dis.begin();
  delay(50);
  buttonWasPressed = (digitalRead(button_pin) == LOW);
  drawCounter();
}

void loop()
{
  bool pressed = digitalRead(button_pin) == LOW;
  buttonState = digitalRead(button_pin);
  digitalWrite(led_pin, buttonState);

  if (pressed && !buttonWasPressed)
  {
    pressStart = millis();
    buttonWasPressed = true;
    lastActivity = millis();
  }

  if (pressed && buttonWasPressed)
  {
    if (millis() - pressStart >= LONG_PRESS_MS)
    {
      counter = 0;
      EEPROM.put(0, counter);
      drawCounter();
      pressStart = millis();
    }
  }

  if (!pressed && buttonWasPressed)
  {
    if (millis() - pressStart < LONG_PRESS_MS)
    {
      counter++;
      EEPROM.put(0, counter);
      drawCounter();
    }
    buttonWasPressed = false;
    lastActivity = millis();
  }

  if (millis() - lastActivity >= SLEEP_AFTER_MS)
  {
    goSleep();
    lastActivity = millis();
  }
}
