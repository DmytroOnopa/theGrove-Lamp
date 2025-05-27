#include "segment_driver.h"

#define D2 2
#define D3 3
#define D4 4
#define D5 5
#define D6 6
#define D7 7

struct Segment {
  int positivePin;
  int negativePin;
};

Segment segments[24] = {
  {D6, D4}, {D6, D5},
  {D2, D3}, {D2, D4}, {D2, D5}, {D2, D6}, {D2, D7}, {D3, D2}, {D3, D4},
  {D3, D5}, {D3, D6}, {D3, D7}, {D4, D2}, {D4, D3}, {D4, D5}, {D4, D6},
  {D4, D7},
  {D5, D2}, {D5, D3}, {D5, D4}, {D5, D6}, {D5, D7}, {D6, D2}, {D6, D3}
};

const uint8_t digits[10][7] = {
  {1,1,1,1,1,1,0}, // 0
  {0,1,1,0,0,0,0}, // 1
  {1,1,0,1,1,0,1}, // 2
  {1,1,1,1,0,0,1}, // 3
  {0,1,1,0,0,1,1}, // 4
  {1,0,1,1,0,1,1}, // 5
  {1,0,1,1,1,1,1}, // 6
  {1,1,1,0,0,0,0}, // 7
  {1,1,1,1,1,1,1}, // 8
  {1,1,1,1,0,1,1}  // 9
};

const uint8_t segmentIndexMap[4][7] = {
  {0, 1, 1, 1, 1, 1, 1},
  {2,3,4,5,6,7,8},
  {9,10,11,12,13,14,15},
  {17,18,19,20,21,22,23}
};

void segment_init() {
  for (int pin = D2; pin <= D7; pin++) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
  }
}

void segment_clear() {
  for (int pin = D2; pin <= D7; pin++) {
    digitalWrite(pin, LOW);
    pinMode(pin, INPUT);
  }
}

void segment_activate(int n) {
  segment_clear();
  pinMode(segments[n].positivePin, OUTPUT);
  pinMode(segments[n].negativePin, OUTPUT);
  digitalWrite(segments[n].positivePin, HIGH);
  digitalWrite(segments[n].negativePin, LOW);
}

void segment_update() {
  static uint8_t currentSegment = 0;
  segment_activate(currentSegment);
  currentSegment = (currentSegment + 1) % 24;
}

void segment_showDigit(uint8_t digitIndex, char c) {
  if (digitIndex < 1 || digitIndex > 3) return;
  if (c < '0' || c > '9') return;
  uint8_t num = c - '0';
  for (uint8_t i = 0; i < 7; i++) {
    if (digits[num][i]) {
      segment_activate(segmentIndexMap[digitIndex][i]);
      delay(3);
    }
  }
}

void segment_showNumber(uint16_t number) {
  uint8_t digitsToShow[3] = {
    (number / 100) % 10,
    (number / 10) % 10,
    number % 10
  };
  for (uint8_t i = 0; i < 3; i++) {
    segment_showDigit(i + 1, '0' + digitsToShow[i]);
  }
}

void displayBrightness(uint8_t val) {
  segment_showNumber(val);
}

void testIndicator() {
  for (int i = 0; i < 24; i++) {
    segment_activate(i);
    delay(100);
  }
  segment_clear();
}

void segment_animationLoop(uint16_t delayMs = 50) {
  for (uint8_t i = 0; i < 24; i++) {
    segment_activate(i);
    delay(delayMs);
  }
  segment_clear();
}

void segment_progressBar(uint8_t percent) {
  percent = constrain(percent, 0, 100);
  uint8_t totalSegments = 21; // ігноруємо 3 для пробілів/візу
  uint8_t activeCount = map(percent, 0, 100, 0, totalSegments);

  segment_clear();
  for (uint8_t i = 0; i < activeCount; i++) {
    segment_activate(i);
    delay(10);
  }
}