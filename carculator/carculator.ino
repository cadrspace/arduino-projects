/* carculator.ino -- A simple CARculator.
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

const int IN_BTN_PIN   = 21; /* State switching button. */
const int IN_DIGIT_PIN = 3;  /* Rotary dial digit switch. */
const int IN_START_PIN = 2;  /* Rotary dial start switch. */

/* LCD dimensions. */
const int LCD_HIGHT = 2;        /* rows */
const int LCD_WIDTH = 16;       /* symbols */

/* initialize the liquid crystal library with the numbers of the
   interface pins. */
LiquidCrystal lcd(12, 11, 7, 6, 5, 4);


volatile int num   = 0;
volatile int digit = 1;

volatile byte function     = 1;
volatile unsigned int num1 = 0;
volatile unsigned int num2 = 0;


/* A state machine for the device. */
enum STATE {
  FORTUNE,
  FUNCTION,
  FIRST_ARG,
  SECOND_ARG,
  EVAL,
  SHOW_RESULT
} STATE;

/* Available functions. */
enum FUNCTION {
  PLUS = 1,
  MINUS,
  MUL
};

static byte state = FORTUNE;


/* Convert FUNCTION code to a string. */
char* functionToString (int function) {
  switch (function) {
  case PLUS:
    return "+";
  case MINUS:
    return "-";
  case MUL:
    return "*";
  default:
    return NULL;
  }
}


/* ISRs */

/* Handle rotary dial stop. */
void handleStop() {
  delay(10);
  if (! digitalRead(IN_START_PIN)) {
    if (state == FUNCTION) {
      if (num == 10)
        function = 0;
      else
        function = num;

      lcd.setCursor(1, 0);
      lcd.print(functionToString(function));

    } else if (state == FIRST_ARG) {
      num1 = (num1 * 10) + num;
      digit *= 10;
    } else if (state == SECOND_ARG) {
      num2 = (num2 * 10) + num;
      digit *= 10;
    }

    num = 0;
  }
}

/* Handle rotary dial digit interrupt. */
void handleDigit() {
  ++num;
  delay(10);
}


/* Evaluate an expression with FUNCTION and arguments ARG1 and
   ARG2.  Return the result of evaluation. */
int eval(int function, int arg1, int arg2) {
  switch (function) {
  case PLUS:
    return num1 + num2;
  case MINUS:
    return num1 - num2;
  case MUL:
    return num1 * num2;
  default:
    /* XXX: A dirty hack, needs to be fixed. */
    return eval(PLUS, arg1, arg2);
  }
}

/* Handle control button interrupt. */
void handleButton() {
  switch (state) {
  case FORTUNE:
    lcd.cursor();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("(+"));
    break;
  case FUNCTION:
    lcd.print(F(" "));
    break;
  case FIRST_ARG:
    digit = 1;
    lcd.print(num1);
    lcd.print(F(" "));
    break;
  case SECOND_ARG:
    digit = 1;
    lcd.print(num2);
    lcd.print(F(")"));
    break;
  case EVAL:
    lcd.setCursor(0, 1);
    lcd.print(F("=> "));
    lcd.print(eval(function, num1, num2));
    function = 1;
    num1 = 0;
    num2 = 0;
    break;
  case SHOW_RESULT:
    lcd.noCursor();
    break;
  }

  if (state != SHOW_RESULT)
    ++state;
  else
    state = FORTUNE;
}


/* Scrolling text. */

const int SCROLL_DELAY = 200;   /* ms */

/* Scroll a string STR of size STR_LEN on the 2nd row of
   the LCD. */
void scroll(char* str, size_t str_len) {
  size_t idx1 = 0, idx2 = 1;

  for (int idx = 0; idx2 < 15; ++idx2) {
    if (state != FORTUNE)
      return;
    lcd.setCursor(15 - idx2, 1);
    for (size_t idx = idx1; idx < idx2; ++idx)
      lcd.print(str[idx]);
    delay(SCROLL_DELAY);
  }

  while (idx1 < idx2) {
    if (state != FORTUNE)
      return;
    lcd.setCursor(0, 1);
    for (size_t idx = idx1; idx < idx2; ++idx) {
      if (state != FORTUNE)
        return;
      lcd.print(str[idx]);
    }

    delay(SCROLL_DELAY);

    /* Clear the row. */
    lcd.setCursor(0, 1);
    for (size_t idx = idx1; (idx < str_len) && (idx < idx2); ++idx)
      lcd.print(' ');

    ++idx1;
    if (idx2 < str_len)
      ++idx2;
  }
}


/* Fortunes. */

static char str0[] = "I need your case, your bootloader and your motherboard.";
static char str1[] = "My mission is to 'chmod 700' you.";
static char str2[] = "I swear I will not 'kill -9' anyone.";

/* Array of fortunes. */
static char* fortunes[] = {
  str0, str1, str2
};


/* Initialization. */

void setup() {
  Serial.begin(9600);

  pinMode(IN_START_PIN, INPUT);
  pinMode(IN_DIGIT_PIN, INPUT);

  /* Initialize random number generator. */
  randomSeed(analogRead(0));

  // set up the LCD's number of columns and rows: 
  lcd.begin(LCD_WIDTH, LCD_HIGHT);

  attachInterrupt(digitalPinToInterrupt(IN_BTN_PIN),
                  handleButton,
                  RISING);
  attachInterrupt(digitalPinToInterrupt(IN_START_PIN),
                  handleStop,
                  FALLING);
  attachInterrupt(digitalPinToInterrupt(IN_DIGIT_PIN),
                  handleDigit,
                  FALLING);
}


/* Main loop */

void loop() {
  if (state == FORTUNE) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("I'm a carculator"));
    lcd.setCursor(0, 1);
    char* rstr = fortunes[random(2)];
    scroll(rstr, strlen(rstr));
    delay(100);
  }
}

/* main.ino ends here. */