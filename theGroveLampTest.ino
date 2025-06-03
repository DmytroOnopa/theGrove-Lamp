#include <Adafruit_NeoPixel.h>
#include <EEPROM.h>
#include "segment_driver.h"
#include <Encoder.h>
#include <FastLED.h>

#define LED_PIN    8
#define LED_COUNT  30

#define ENCODER_CLK  A0
#define ENCODER_DT   A1
#define ENCODER_SW   A2

#define SPEAKER_PIN 9
#define SHORT_BEEP_FREQ 1000
#define LONG_BEEP_FREQ 800
#define BEEP_DURATION 100
#define ENCODER_CLICK_FREQ 500

// Додаємо константи для звуків
#define LIGHTSABER_HUM_FREQ 350
#define LIGHTSABER_SWING_FREQ 150
#define LIGHTSABER_POWERUP_FREQ 600
#define LIGHTSABER_POWERDOWN_FREQ 200

// Додамо константи для меча
#define SABER_LENGTH 30
#define SABER_ANIMATION_SPEED 30
#define POWER_ANIMATION_STEP 3

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
Encoder encoder(ENCODER_CLK, ENCODER_DT);

int brightness = 128;
uint16_t hue = 0;
bool changeBrightness = false;

uint8_t currentEffect = 0;
const uint8_t numEffects = 4;

int lastClk = HIGH;

int lastBrightness = -1;

unsigned long lastDisplaySwitch = 0;
const unsigned long brightnessDisplayDuration = 1500; // показуємо яскравість 1.5 секунди

bool showingBrightness = false;

#define ADDR_HUE_L 1
#define ADDR_HUE_H 2

#define HUE_STEP 512

struct Meteor {
  int position;
  uint8_t brightness;
  uint32_t color;
  int speed;
};

Meteor meteors[5];

// Додаємо новий режим до enum Mode
enum Mode { NORMAL, ANIMATION, POLICE, INTERACTIVE }; // Додали INTERACTIVE
Mode currentMode = NORMAL;

enum FlasherType { POLICE_FLASHER, YELLOW_BLUE_FLASHER };
FlasherType currentFlasher = POLICE_FLASHER;

const unsigned long debounceDelay = 100;
unsigned long lastDebounceTime = 0;
bool lastButtonState = HIGH;
bool buttonHeld = false;
unsigned long pressStart = 0;

// Оновлюємо змінні для меча
bool saberActive = false;
bool saberPowering = false;
uint8_t saberPosition = 0;
uint8_t saberHue = 96; // Синій за замовчуванням
uint8_t saberBrightness = 0;

void saveSettings() {
  EEPROM.update(ADDR_HUE_L, hue & 0xFF);
  EEPROM.update(ADDR_HUE_H, hue >> 8);
}

void loadSettings() {
  uint8_t l = EEPROM.read(ADDR_HUE_L);
  uint8_t h = EEPROM.read(ADDR_HUE_H);
  hue = (h << 8) | l;
}

void playTone(uint16_t frequency, uint32_t duration) {
  tone(SPEAKER_PIN, frequency, duration);
  delay(duration);
  noTone(SPEAKER_PIN);
}

void shortBeep() {
  playTone(SHORT_BEEP_FREQ, BEEP_DURATION);
}

void longBeep() {
  playTone(LONG_BEEP_FREQ, BEEP_DURATION * 2);
}

void encoderClick() {
  playTone(ENCODER_CLICK_FREQ, 20);
}

void setup() {
  pinMode(ENCODER_CLK, INPUT_PULLUP);
  pinMode(ENCODER_DT, INPUT_PULLUP);
  pinMode(ENCODER_SW, INPUT_PULLUP);
  pinMode(SPEAKER_PIN, OUTPUT);

  segment_init();

  Serial.begin(9600);
  strip.begin();
  strip.show();

  loadSettings();

  delay(50);
  lastButtonState = digitalRead(ENCODER_SW);

  currentMode = NORMAL;
  changeBrightness = false;
  currentEffect = 0;
  currentFlasher = POLICE_FLASHER;

  testIndicator();
  shortBeep();

  fadeOn();
}

// Оновлюємо loop()
void loop() {
  handleEncoder();
  handleButton();

  switch (currentMode) {
    case NORMAL: updateLEDs(); break;
    case ANIMATION: runCurrentEffect(); break;
    case POLICE: 
      if (currentFlasher == POLICE_FLASHER) policeFlash(); 
      else yellowBlueFlash();
      break;
    case INTERACTIVE: lightsaberEffect(); break;
  }
  updateDisplay(); // Оновлюємо індикатор, щоб він не гас
}

void updateDisplay() {
  unsigned long now = millis();

  if (showingBrightness) {
    segment_showNumber(brightness);
    if (now - lastDisplaySwitch > brightnessDisplayDuration) {
      showingBrightness = false;
    }
  } else {
    switch(currentMode) {
      case NORMAL:
        segment_showChar(1, 'L');
        break;
      case ANIMATION:
        segment_showChar(2, 'A');
        break;
      case POLICE:
        segment_showChar(1, 'S');
        segment_showChar(2, 'I');
        segment_showChar(3, 'G');
        break;
      case INTERACTIVE:
        segment_showChar(1, 'G');
        segment_showChar(2, 'U');
        segment_showChar(3, 'F');
        break;
      default:
        segment_showNumber(currentMode);
        break;
    }
  }
}

void showHoldEffect() {
  // Створюємо зелений пульс по центру стрічки
  static uint8_t pulse = 0;
  static int8_t dir = 5;

  pulse += dir;
  if (pulse >= 255 || pulse <= 0) dir = -dir;

  strip.clear();
  for (int i = 14; i <= 15; i++) {
    strip.setPixelColor(i, strip.Color(0, pulse, 0));  // зелений
  }
  strip.show();
}

void handleEncoder() {
  static int8_t lastState = 0;
  static int8_t counter = 0; // Змінимо на знаковий тип
  const uint8_t threshold = 3;
  static unsigned long lastDisplayUpdate = 0;

  // Читання стану енкодера з підтяжкою
  bool clk = digitalRead(ENCODER_CLK);
  bool dt = digitalRead(ENCODER_DT);
  
  // Визначення поточного стану
  int8_t currentState = (clk << 1) | dt;

  // Масив переходів для визначення напрямку
  const int8_t transitions[16] = {
     0, -1,  1,  0,  // 00 -> [00,01,10,11]
     1,  0,  0, -1,  // 01 -> [00,01,10,11]
    -1,  0,  0,  1,  // 10 -> [00,01,10,11]
     0,  1, -1,  0   // 11 -> [00,01,10,11]
  };

  // Визначення напрямку обертання
  int8_t direction = transitions[(lastState << 2) | currentState];

  if (direction != 0) {
    counter += direction;
    
    // Реагуємо тільки при досягненні порогу
    if (abs(counter) >= threshold) {
      int8_t actualChange = counter / abs(counter); // +1 або -1

      if (changeBrightness) {
        brightness = constrain(brightness + actualChange * 10, 0, 255);
        strip.setBrightness(brightness);
        segment_showNumber(brightness);
        lastDisplaySwitch = millis();
        showingBrightness = true;
      } else {
        hue = (hue + actualChange * 500 + 65536) % 65536; // Додаємо 65536 для уникнення від'ємних значень
        if (!showingBrightness) {
          segment_showChar(1, 'C');
        }
      }
      
      counter = 0;
      encoderClick();
      lastDisplayUpdate = millis();
    }
  }
  
  lastState = currentState;

  // Автоматичне приховування яскравості
  if (showingBrightness && (millis() - lastDisplaySwitch > 1500)) {
    showingBrightness = false;
    updateDisplay(); // Повертаємо відображення режиму
  }
}

void handleButton() {
  static unsigned long lastPressTime = 0;
  static bool buttonHeld = HIGH;
  static bool pressInProgress = false;
  static unsigned long pressStart = 0;
  const unsigned long clickTimeout = 400;
  const unsigned long longPressDuration = 500;
  bool reading = digitalRead(ENCODER_SW);

  // Debounce
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonHeld) {
      buttonHeld = reading;

      if (buttonHeld == LOW) {
        // Звуковий фідбек одразу при натисканні
        encoderClick();

        pressStart = millis();
        pressInProgress = true;
      } else {
        unsigned long pressDuration = millis() - pressStart;
        pressInProgress = false;

        if (pressDuration >= longPressDuration) {
          longBeep(); // довгий звук
          currentMode = (Mode)((currentMode + 1) % 4);
        } else {
          shortBeep(); // короткий звук
          if (currentMode == ANIMATION)
            currentEffect = (currentEffect + 1) % numEffects;
          else if (currentMode == POLICE)
            currentFlasher = (currentFlasher == POLICE_FLASHER) ? YELLOW_BLUE_FLASHER : POLICE_FLASHER;
          else if (currentMode == INTERACTIVE) {
            saberActive = !saberActive;
            saberPowering = true;
            if (saberActive)
              playTone(LIGHTSABER_POWERUP_FREQ, 300);
            else
              playTone(LIGHTSABER_POWERDOWN_FREQ, 300);
          } else {
            changeBrightness = !changeBrightness;
          }
        }
      }
    }

    // Поки тримається кнопка — показуємо індикацію
    if (pressInProgress && (millis() - pressStart > 200)) {
      segment_showChar(2,'H'); // показуємо "H" на індикаторі
      showHoldEffect();      // запускаємо візуальний ефект
    }
  }

  lastButtonState = reading;
}

// Добавляем в начало файла объявления всех недостающих функций
void fadeOn() {
  for (int b = 0; b <= brightness; b += 5) {
    uint32_t color = strip.ColorHSV(hue, 255, b);
    for (int i = 0; i < LED_COUNT; i++) strip.setPixelColor(i, color);
    strip.show();
    delay(15);
  }
}

void updateLEDs() {
  uint32_t color = strip.ColorHSV(hue, 255, brightness);
  for (int i = 0; i < LED_COUNT; i++) strip.setPixelColor(i, color);
  strip.show();
}

void displayMode(Mode mode) {
  segment_showChar(1, 't');
  segment_showChar(2, '0' + mode);
}

// Оновлюємо lightsaberEffect()
void lightsaberEffect() {
  static unsigned long lastUpdate = 0;
  unsigned long now = millis();

  if (now - lastUpdate < SABER_ANIMATION_SPEED) return;
  lastUpdate = now;

  // Анімація тільки при запалюванні/затуханні
  if (saberPowering) {
    if (saberActive) {
      // Запалювання - плавне збільшення яскравості
      saberBrightness = min(255, saberBrightness + POWER_ANIMATION_STEP);
      if (saberBrightness == 255) {
        saberPowering = false;
        // Додатковий ефект "удар" при повному запалюванні
        playTone(LIGHTSABER_HUM_FREQ + 50, 100);
      }
    } else {
      // Затухання - плавне зменшення яскравості
      saberBrightness = max(0, saberBrightness - POWER_ANIMATION_STEP);
      if (saberBrightness == 0) {
        saberPowering = false;
        // Додатковий ефект при повному затуханні
        playTone(LIGHTSABER_HUM_FREQ - 30, 80);
      }
    }
  }

  strip.clear();

  if (saberBrightness > 0) {
    // Статичне відображення меча (без руху), якщо не в режимі анімації
    if (!saberPowering) {
      // Просто статичний меч з градієнтом
      for (int i = 0; i < SABER_LENGTH; i++) {
        uint8_t pos = (saberPosition + i) % LED_COUNT;
        uint8_t brightness = saberBrightness * (SABER_LENGTH - i) / SABER_LENGTH;
        strip.setPixelColor(pos, strip.ColorHSV(saberHue, 255, brightness));
      }
      strip.setPixelColor(saberPosition, strip.ColorHSV(saberHue, 200, saberBrightness));
      
      // Випадкові дуже тихі звукові ефекти (лише гул)
      if (random(100) < 2) {  // Дуже рідкісні
        playTone(LIGHTSABER_HUM_FREQ + random(-10, 10), 30);
      }
    } 
    else {
      // Анімація руху ТІЛЬКИ під час запалювання/затухання
      saberPosition = (saberPosition + 1) % LED_COUNT;
      
      for (int i = 0; i < SABER_LENGTH; i++) {
        uint8_t pos = (saberPosition + i) % LED_COUNT;
        uint8_t brightness = saberBrightness * (SABER_LENGTH - i) / SABER_LENGTH;
        strip.setPixelColor(pos, strip.ColorHSV(saberHue, 255, brightness));
      }
      strip.setPixelColor(saberPosition, strip.ColorHSV(saberHue, 200, saberBrightness));
      
      // Більш активні звукові ефекти під час анімації
      if (random(20) < 3) {
        playTone(LIGHTSABER_HUM_FREQ + random(-20, 20), 50 + random(50));
      }
    }
  }

  strip.show();
}

void policeFlash() {
  static unsigned long lastChange = 0;
  static uint8_t state = 0;
  unsigned long now = millis();

  if (now - lastChange > 100) {
    lastChange = now;

    for (int i = 0; i < LED_COUNT; i++) strip.setPixelColor(i, 0);

    switch (state) {
      case 0:
      case 2:
        // Червоний на першій половині
        for (int i = 1; i <= 8; i++) strip.setPixelColor(i, strip.Color(brightness, 0, 0));
        for (int i = 24; i <= 29; i++) strip.setPixelColor(i, strip.Color(brightness, 0, 0));
        break;

      case 4:
      case 6:
        // Синій на другій половині
        for (int i = 9; i <= 23; i++) strip.setPixelColor(i, strip.Color(0, 0, brightness));
        break;
    }

    strip.show();
    state = (state + 1) % 8;
  }
}

void yellowBlueFlash() {
  static unsigned long lastChange = 0;
  static float phase = 0.0; // Точність float для плавності
  unsigned long now = millis();

  // Повільніше оновлення фази
  if (now - lastChange > 8) { // Замість 4ms для повільнішого руху
    lastChange = now;
    phase += 0.20; // Зменшений крок для повільнішої зміни
    
    if (phase >= 2*PI) phase = 0; // Скидаємо на новий цикл
  }

  // Плавна синусоїда з "вирівнюванням" для красивих пульсацій
  float wave = (sin(phase) + 1.0) * 0.5; // Нормалізуємо до 0..1
  
  // Додаємо "плато" на піках для помітнішого світіння
  wave = smoothWave(wave); 

  uint8_t currentBrightness = brightness * wave;

  // Світлодіоди
  strip.clear();
  uint8_t r = map(currentBrightness, 0, brightness, 0, 255);
  uint8_t g = map(currentBrightness, 0, brightness, 0, 40);
  int emergencyPixels[] = {0, 1, 13, 14, 15, 16, 28, 29};
  
  for (int i = 0; i < 8; i++) {
    strip.setPixelColor(emergencyPixels[i], strip.Color(r, g, 0));
  }
  strip.show();
}

// Допоміжна функція для гладких піків
float smoothWave(float x) {
  if (x > 0.9) return 1.0; // Плато на піку
  if (x < 0.1) return 0.0; // Повне вимикання в нулі
  return x; // Лінійні переходи
}

// Оновлюємо runCurrentEffect()
void runCurrentEffect() {
  switch (currentEffect) {
    case 0: runningDotEffect(); break;
    case 1: raveStormEffect(); break;
    case 2: colorTornadoEffect(); break;
    case 3: plasmaVortexEffect(); break;
  }
}

void runningDotEffect() {
  static unsigned long lastUpdate = 0;
  static float position = 0.0;
  unsigned long now = millis();

  if (now - lastUpdate > 20) {  // Дуже мала затримка для плавності
    lastUpdate = now;
    strip.clear();
    
    // Створюємо градієнт, що рухається
    for (int i = 0; i < LED_COUNT; i++) {
      float distance = fmod(fabs(i - position), LED_COUNT);
      distance = min(distance, LED_COUNT - distance);  // Враховуємо циклічність
      
      if (distance < 5.0) {  // Ширина градієнта
        float intensity = 1.0 - (distance / 5.0);
        strip.setPixelColor(i, strip.ColorHSV(hue, 255, (int)(brightness * intensity)));
      }
    }
    
    strip.show();
    position = fmod(position + 0.5, LED_COUNT);  // Швидкість руху
  }
}

void raveStormEffect() {
  static unsigned long lastUpdate = 0;
  static uint8_t fade = 0;
  
  // Оновлення кожні 50ms (20 FPS)
  if(millis() - lastUpdate < 50) return;
  lastUpdate = millis();
  
  // Випадкові спалахи на 1/3 світлодіодів
  if(random(3) == 0) { // 33% шанс спалаху
    for(int i = 0; i < LED_COUNT/3; i++) {
      int pixel = random(LED_COUNT);
      uint16_t hue = random(65536); // Повний спектр
      strip.setPixelColor(pixel, strip.ColorHSV(hue, 255, 255));
    }
  }
  
  // Плавне згасання всіх пікселів
  for(int i = 0; i < LED_COUNT; i++) {
    uint32_t color = strip.getPixelColor(i);
    uint8_t r = (color >> 16) & 0xFF;
    uint8_t g = (color >> 8) & 0xFF;
    uint8_t b = color & 0xFF;
    
    r = max(0, r - 40);
    g = max(0, g - 40);
    b = max(0, b - 40);
    
    strip.setPixelColor(i, strip.Color(r, g, b));
  }
  
  strip.show();
}

void colorTornadoEffect() {
  static float offset = 0;
  static unsigned long lastUpdate = 0;
  
  // Оновлення кожні 30ms (~33 FPS)
  if(millis() - lastUpdate < 30) return;
  lastUpdate = millis();
  
  // Швидкість обертання (змініть для налаштування)
  offset += 0.3;
  if(offset > 100) offset = 0;
  
  for(int i = 0; i < LED_COUNT; i++) {
    // Визначаємо позицію у спіралі
    float pos;
    if(i <= 14) {
      pos = (float)i/14.0 * PI + offset;
    } else {
      pos = (float)(i-15)/14.0 * PI + offset;
    }
    
    // Генеруємо кольоровий градієнт
    uint16_t hue = (uint16_t)(pos * 4000) % 65536;
    uint8_t sat = 255;
    uint8_t val = (sin(pos * 5.0) + 1.0) * 127;
    
    strip.setPixelColor(i, strip.ColorHSV(hue, sat, val));
  }
  
  strip.show();
}

// Нова анімація - plasmaVortexEffect
void plasmaVortexEffect() {
  static float offset = 0.0;
  static unsigned long lastUpdate = 0;
  const float speed = 0.08; // швидкість
  const uint8_t brightness = 200;
  
  if (millis() - lastUpdate < 15) return; // ~66 FPS
  lastUpdate = millis();

  for (int i = 0; i < LED_COUNT; i++) {
    // Визначаємо позицію у "вихорі" (враховуємо фізичне розташування)
    float position;
    if (i <= 14) {
      position = map(i, 0, 14, 0, PI); // Ліва половина
    } else {
      position = map(i, 15, 29, PI, 2*PI); // Права половина (зеркально)
    }
    
    // Генеруємо 3 накладені хвилі різних частот. upd.:
    // Прискорили хвилі (множителі 2.0 -> 3.0)
    float wave1 = sin(position * 3.0 + offset * 2.0);  // Було 2.0
    float wave2 = cos(position * 4.0 - offset * 3.0);  // Було 3.0
    float wave3 = sin(position * 1.0 + offset * 1.2);  // Було 0.5
    
    // Комбінуємо хвилі
    float energy = (wave1 + wave2 + wave3) / 3.0; // -1..1
    energy = (energy + 1.0) / 2.0; // 0..1
    
    // Яскраві кольори (від фіолетового до блакитного)
 // Пришвидшили зміну кольорів (0.3 -> 0.5)
    uint16_t hue = 45000 + 10000 * sin(offset * 0.5);  // Плавна зміна палітри. Було 0.3
    uint8_t sat = 200 + 55 * cos(offset * 0.7);       // Коливання насиченості. Було 0.5
    
    // Підсилюємо ефект на кутах
    if (i == 0 || i == 7 || i == 14 || i == 15 || i == 22 || i == 29) {
      energy = min(1.0, energy * 1.5);
    }
    
    strip.setPixelColor(i, strip.ColorHSV(hue, sat, energy * brightness));
  }
  
  strip.show();
  offset += speed;
}
