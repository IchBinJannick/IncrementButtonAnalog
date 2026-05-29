#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <EEPROM.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>

#define res_button_pin 4
#define button_pin 3
#define led_pin 2
#define res_led_pin 5

#define DEBOUNCE_MS 50
unsigned long SLEEP_AFTER_MS = 20000;

// SSD1306 128x64 I2C
// A4 = SDA, A5 = SCL
U8G2_SSD1306_128X64_NONAME_F_HW_I2C dis(U8G2_R0, U8X8_PIN_NONE);

int counter = 0;
int buttonState = 0;
bool buttonWasPressed = false;
unsigned long lastActivity = 0;

int countButtonState = HIGH;
int lastCountButtonState = HIGH;
unsigned long lastDebounceCount = 0;

int resetButtonState = HIGH;
int lastResetButtonState = HIGH;
unsigned long lastDebounceReset = 0;

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
  delay(DEBOUNCE_MS);

  countButtonState = HIGH;
  lastCountButtonState = HIGH;
  lastDebounceReset = millis();

  resetButtonState = HIGH;
  lastResetButtonState = HIGH;
  lastDebounceReset = millis();

  drawCounter();
}

void setup()
{
  EEPROM.get(0, counter);
  if (counter < 0 || counter > 9999)
    counter = 0;

  pinMode(led_pin, OUTPUT);
  pinMode(res_led_pin, OUTPUT);
  pinMode(button_pin, INPUT_PULLUP);
  pinMode(res_button_pin, INPUT_PULLUP);

  Wire.begin();
  dis.begin();

  drawCounter();
  lastActivity = millis();
}

void loop()
{
  // ── Count Button Debounce ──────────────────────────────
  int countReading = digitalRead(button_pin);

  if (countReading != lastCountButtonState)
    lastDebounceCount = millis();

  if ((millis() - lastDebounceCount) > DEBOUNCE_MS)
  {
    if (countReading != countButtonState)
    {
      countButtonState = countReading;

      // INPUT_PULLUP → gedrückt = LOW
      if (countButtonState == LOW)
      {
        counter++;
        EEPROM.put(0, counter);
        drawCounter();
        lastActivity = millis();
      }
    }
  }
  lastCountButtonState = countReading;
  digitalWrite(led_pin, countReading); // leuchtet beim Drücken

  // ── Reset Button Debounce ──────────────────────────────
  int resetReading = digitalRead(res_button_pin);

  if (resetReading != lastResetButtonState)
    lastDebounceReset = millis();

  if ((millis() - lastDebounceReset) > DEBOUNCE_MS)
  {
    if (resetReading != resetButtonState)
    {
      resetButtonState = resetReading;

      if (resetButtonState == LOW)
      {
        counter = 0;
        EEPROM.put(0, counter);
        drawCounter();
        lastActivity = millis();
      }
    }
  }
  lastResetButtonState = resetReading;
  digitalWrite(res_led_pin, resetReading);

  // ── Sleep ──────────────────────────────────────────────
  if (millis() - lastActivity >= SLEEP_AFTER_MS)
  {
    goSleep();
    lastActivity = millis();
  }
}
