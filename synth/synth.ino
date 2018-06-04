/* synth.ino -- Simple Arduino synthesizer based on CD4021BE.
 *
 * A simple Arduino music synthesizer based on CD4021BE shift
 * register.
 * 
 * Copyright (C) 2018 Artyom V. Poptsov <poptsov.artyom@gmail.com>
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

const int DATA_PIN = 9;
const int LATCH_PIN = 8;
const int CLOCK_PIN = 10;

const int SPEAKER_PIN = 2;

const float a3 = 220.00;
const float c4 = 261.63;
const float d4 = 293.66;
const float e4 = 329.63;
const float f4 = 349.23;
const float g4 = 392.00;
const float a4 = 440.00;
const float h4 = 493.88;

void setup() {
  pinMode(DATA_PIN, INPUT);
  pinMode(LATCH_PIN, OUTPUT);
  pinMode(CLOCK_PIN, OUTPUT);
  pinMode(SPEAKER_PIN, OUTPUT);
  Serial.begin(9600);
}

void play_tone(int pin, float f, long len) {
  long p = 1000000 / f;
  long d = p / 2;
  int count = len / p;
  for (int i = 0; i < count; ++i) {
    digitalWrite(pin, HIGH);
    delayMicroseconds(d);
    digitalWrite(pin, LOW);
    delayMicroseconds(d);
  }
}

struct note {
  byte c;
  float f;
};

const byte NOTES_COUNT = 9;

struct note notes[NOTES_COUNT] = {
  { 0b10000001, h4 }, // dirty hack
  { 0b00000001, h4 },
  { 0b00000010, a4 },
  { 0b00000100, g4 },
  { 0b00001000, f4 },
  { 0b00010000, e4 },
  { 0b00100000, d4 },
  { 0b01000000, c4 },
  { 0b10000000, a3 },
};

void play_note(byte b) {
  for (struct note n : notes) {
    if (n.c == b)
      play_tone(SPEAKER_PIN, n.f, 100000);
  }

}

void loop() {
  digitalWrite(LATCH_PIN, HIGH);
  delayMicroseconds(40);
  digitalWrite(LATCH_PIN, LOW);
  byte b = shiftIn(DATA_PIN, CLOCK_PIN, LSBFIRST);
 // Serial.println(b, BIN);
  //Serial.println();
  play_note(b);
  delay(10);
}
