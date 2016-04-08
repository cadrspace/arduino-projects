/* lcd-loading-animation.ino -- "Loading" animation for LCD
 *
 * This simple program is written for MELT (МЭЛТ) MT-16S2H LCD, but
 * likely it can be used with other LCDs with little or no
 * modification as well.
 *
 * See <http://cadrspace.ru/w/index.php/Arduino/МТ–16S2H> for
 * instructions on how to connect a display to an Arduino.
 *
 * Copyright (C) 2016 Artyom V. Poptsov <poptsov.artyom@gmail.com>
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see
 * <https://www.gnu.org/licenses/>.
 */

#include <LiquidCrystal.h>

// Initialize the library with the numbers of the interface pins.
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

#define CHARS 5
#define ROWS  8
#define DELAY 100

void setup() {
  byte chars[CHARS][ROWS] = {0};

  // set up the LCD's number of columns and rows: 
  lcd.begin(16, 2);
  
  for (int ch = 0; ch < CHARS; ++ch) {
    for (int row = 0; row <= ROWS; ++row)
      chars[ch][row] = 0xFF << ch;
    lcd.createChar(ch, chars[ch]);
  }
}

void clearRow(int row) {
 lcd.setCursor(0, 1);
 for (int i = 0; i < 16; ++i)
   lcd.write(0x20);
}

void loop() {
  for (int i = 0; i < 16; ++i) {
      lcd.setCursor(i, 0);
      for (int j = CHARS - 1; j >= 0; --j) {
        lcd.setCursor(i, 0);
        lcd.write((byte) j);
        delay (DELAY);
        
        lcd.setCursor(3, 1);
        if (j && !(j & (j - 1)))
          lcd.print("\xb7\x61\xb4""py""\xb7""\xba""a ...");
        else
          clearRow(1);
      }
  }
  lcd.clear();
}


