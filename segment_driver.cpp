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

const uint8_t letters[][7] = {
  {1,1,1,0,1,1,1}, // A
  {0,0,1,1,1,1,1}, // b
  {1,0,0,1,1,1,0}, // C
  {0,1,1,1,1,0,1}, // d
  {1,0,0,1,1,1,1}, // E
  {1,0,0,0,1,1,1}, // F
  {1,0,1,1,1,1,0}, // G
  {0,1,1,0,1,1,1}, // H
  {0,1,1,0,0,0,0}, // I
  {0,1,1,1,0,0,0}, // J
  {0,0,0,0,0,0,0}, // K — skip
  {0,0,0,1,1,1,0}, // L
  {0,0,0,0,0,0,0}, // M — skip
  {0,0,1,0,1,0,1}, // n
  {1,1,1,1,1,1,0}, // O
  {1,1,0,0,1,1,1}, // P
  {1,1,1,0,0,1,1}, // q
  {0,0,0,0,1,0,1}, // r
  {1,0,1,1,0,1,1}, // S
  {0,0,0,1,1,1,1}, // t
  {0,1,1,1,1,1,0}, // U
  {0,0,0,0,0,0,0}, // V — skip
  {0,0,0,0,0,0,0}, // W — skip
  {0,0,0,0,0,0,0}, // X — skip
  {0,1,1,1,0,1,1}, // Y
  {1,1,0,1,1,0,1}  // Z
};

const uint8_t customChars[][7] = {
  {0,0,0,0,0,0,0}, // ' ' — пусто
  {0,0,0,1,0,0,0}, // '_' — нижній сегмент
  {0,0,1,1,0,0,0}, // 'n' — низ і частина
  {1,1,1,1,1,1,1}  // 'H' — повний заповнений
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

void segment_showChar(uint8_t digitIndex, char c) {
  if (digitIndex < 1 || digitIndex > 3) return;

  if (c >= '0' && c <= '9') {
    uint8_t num = c - '0';
    for (uint8_t i = 0; i < 7; i++) {
      if (digits[num][i]) {
        segment_activate(segmentIndexMap[digitIndex][i]);
        delay(3);
      }
    }
  } else if (c >= 'A' && c <= 'Z') {
    uint8_t index = c - 'A';
    for (uint8_t i = 0; i < 7; i++) {
      if (letters[index][i]) {
        segment_activate(segmentIndexMap[digitIndex][i]);
        delay(3);
      }
    }
  }
}

void segment_showCustom(uint8_t digitIndex, const uint8_t pattern[7]) {
  if (digitIndex < 1 || digitIndex > 3) return;
  for (uint8_t i = 0; i < 7; i++) {
    if (pattern[i]) {
      segment_activate(segmentIndexMap[digitIndex][i]);
      delay(3);
    }
  }
}

void segment_clearDigit(uint8_t digitIndex) {
  if (digitIndex < 1 || digitIndex > 3) return;
  
  // Спосіб 1: через segmentIndexMap (якщо ви його використовуєте)
  for (uint8_t i = 0; i < 7; i++) {
    uint8_t segNum = segmentIndexMap[digitIndex-1][i];
    digitalWrite(segments[segNum].positivePin, LOW);
    digitalWrite(segments[segNum].negativePin, LOW);
  }
  
  // Або спосіб 2: через пряме вимикання (якщо немає segmentIndexMap)
  // for (int pin = D2; pin <= D7; pin++) {
  //   digitalWrite(pin, LOW);
  // }
}

void segment_showNumber(uint16_t number) {
  uint8_t digitsToShow[3] = {
    (number / 100) % 10,
    (number / 10) % 10,
    number % 10
  };

  bool leadingZero = true;
  for (uint8_t i = 0; i < 3; i++) {
    if (leadingZero) {
      if (digitsToShow[i] == 0 && i != 2) {
        // Не показуємо провідний нуль, очищуємо розряд
        segment_clearDigit(i + 1);
      } else {
        leadingZero = false;
        segment_showChar(i + 1, '0' + digitsToShow[i]);
      }
    } else {
      segment_showChar(i + 1, '0' + digitsToShow[i]);
    }
  }
}

void showBrightnessBar(uint8_t brightness) {
  uint8_t level = map(brightness, 0, 255, 0, 9); // Конвертація у 10 рівнів
  
  // Очищаємо індикатор перед оновленням
  segment_clear();
  
  // Відображаємо градієнт на 3 розрядах
  for (uint8_t digit = 0; digit < 3; digit++) {
    if (level >= (digit + 1) * 3) {
      // Повністю заповнений розряд
      segment_showCustom(digit + 1, (const uint8_t[]){1,1,1,1,1,1,1});
    } 
    else if (level > digit * 3) {
      // Частково заповнений
      uint8_t segments = level - digit * 3;
      uint8_t pattern[7] = {0};
      for (uint8_t i = 0; i < segments; i++) {
        pattern[i] = 1;
      }
      segment_showCustom(digit + 1, pattern);
    }
    delay(2); // Невелика затримка для стабільності
  }
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

void segment_showGradient(uint8_t level) {
  level = constrain(level, 0, 9);
  
  // Визначаємо, які сегменти активувати
  uint8_t pattern[3][7] = {
    {0,0,0,0,0,0,0}, // Пусто
    {0,0,0,1,0,0,0}, // Нижня риска (_)
    {0,0,1,1,0,0,0}  // Напівсегмент (n)
  };

  for(uint8_t digit = 0; digit < 3; digit++) {
    uint8_t fillLevel = (level > digit*3) ? min(level-digit*3, 3) : 0;
    
    if(fillLevel == 0) {
      segment_clearDigit(digit+1);
    } else {
      segment_showCustom(digit+1, pattern[fillLevel]);
    }
  }
}