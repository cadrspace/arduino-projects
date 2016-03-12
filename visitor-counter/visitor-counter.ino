#include <NewPing.h>

const int ECHO_PIN = 10;
const int TRIG_PIN = 11;
const int MAXIMUM_RANGE = 100;
const int MINIMUM_RANGE = 0;
const int INDICATOR_MAX_VALUE = 99;

// Delays.
const int PING_DELAY = 5; // ms
const int DETECTION_DELAY = 1000; // ms

// Segments.
const int a = 2;
const int b = 3;
const int c = 4;
const int d = 5;
const int e = 6;
const int f = 7;
const int g = 8;
const int h = 9;

const int offset = 10;

long distance; 

NewPing sonar(TRIG_PIN, ECHO_PIN, MAXIMUM_RANGE);

short visitors_counter = 0;

const int digit[][10] {
  // 0
  { a, b, c, f, e, g, h },
  // 1
  { c, h },
  // 2
  { a, b, d, e, g, h },
  // 3
  { a, b, d, f, g },
  // 4
  { a, c, d, f } ,
  // 5
  { b, c, d, f, g },
  // 6
  { b, c, d, f, g, h },
  // 7
  { a, b, f, e },
  // 8
  { a, b, c, d, e, f, g, h },
  // 9
  { a, b, c, d, f, g }
};

void reset(int digit_idx) {
  for (int pin = (a + digit_idx); pin <= (h + digit_idx); ++pin)
    digitalWrite(pin, HIGH);
}

void show_digit(const int d[], int d_size, short pos) {
  for (int idx = 0; idx < d_size; ++idx)
    digitalWrite(d[idx] + (pos * offset), LOW); 
}

void show_number(int number) {
  int d1 = number / 10;
  int d2 = number % 10;

  if (d1)
    show_digit(digit[d1], sizeof(digit[d1]) / 2, 0);
  else
    reset(1);

  show_digit(digit[d2], sizeof(digit[d2]) / 2, 1);
}

/// Calibration

int max_distance = 0;
int threshold;

void calibrate() {
  while (! max_distance) {
    max_distance = sonar.ping_cm();
  }
  
  threshold = max_distance / 3;

  Serial.print(F("Calibrated.  Maximum distance: "));
  Serial.print(max_distance);
  Serial.print(F("; thresold: "));
  Serial.println(threshold);
}

///

void setup() {
  Serial.begin(9600);
  for (int pin = a; pin <= h; ++pin)
    pinMode(pin, OUTPUT);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  for (int pin = 12; pin <= 19; ++pin) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, HIGH);
  }

  reset(0);
  reset(offset);
  
  calibrate();
}

long old_distance = 0;

void loop() {
  distance = sonar.ping_cm();

  if ((distance > MINIMUM_RANGE) && (distance <= max_distance)) {
    if (abs(distance - old_distance) > threshold) {
      old_distance = distance;
      reset(0);
      reset(offset);
      show_number(++visitors_counter);
      
      Serial.print(F("Distance: "));
      Serial.println(distance);
      Serial.print(F("CADR Visitors: "));
      Serial.println(visitors_counter);

      delay(DETECTION_DELAY);
    }
  } else {
    reset(0);
    reset(offset);
    show_number(visitors_counter);
  }

  if (visitors_counter > INDICATOR_MAX_VALUE)
    visitors_counter = 0;

  delay(PING_DELAY);
}

