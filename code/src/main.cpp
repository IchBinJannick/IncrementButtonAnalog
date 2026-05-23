#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>

// SSD1306 128x64 I2C
// A4 = SDA, A5 = SCL
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

void setup()
{
  Wire.begin();
  u8g2.begin();

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB14_tr);
  u8g2.drawStr(10, 30, "Hallo!");
  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.drawStr(10, 50, "SSD1306 bereit");
  u8g2.sendBuffer();
}

void loop()
{
}
