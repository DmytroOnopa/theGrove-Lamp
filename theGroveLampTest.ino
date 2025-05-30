#include <Adafruit_NeoPixel.h>
#include <EEPROM.h>
#include "segment_driver.h"

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
#define SABER_LENGTH 10
#define SABER_ANIMATION_SPEED 30
#define POWER_ANIMATION_STEP 3

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

int brightness = 128;
uint16_t hue = 0;
bool changeBrightness = false;

uint8_t currentEffect = 0;
const uint8_t numEffects = 3;

int lastClk = HIGH;

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
  displayMode(currentMode);
}

void handleEncoder() {
  int currentClk = digitalRead(ENCODER_CLK);
  if (currentClk != lastClk && currentClk == LOW) {
    int dtState = digitalRead(ENCODER_DT);
    if (dtState != currentClk) {
      if (changeBrightness && brightness < 250) brightness += 5;
      else if (!changeBrightness) hue = (hue + HUE_STEP) % 65536;
    } else {
      if (changeBrightness && brightness > 5) brightness -= 5;
      else if (!changeBrightness) hue = (hue + 65536 - HUE_STEP) % 65536;
    }
    encoderClick();
    saveSettings();
    Serial.print("Яскравість: "); Serial.print(brightness);
    Serial.print(" | Відтінок: "); Serial.println(hue);

    if (currentMode == NORMAL) updateLEDs();
  }
  lastClk = currentClk;
}

void handleButton() {
  static unsigned long lastPressTime = 0;
  static uint8_t clickCount = 0;
  const unsigned long clickTimeout = 400;
  bool reading = digitalRead(ENCODER_SW);

  static unsigned long pressStart = 0;
  static bool pressInProgress = false;

  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonHeld) {
      buttonHeld = reading;

      if (buttonHeld == LOW) {
        pressStart = millis();
        pressInProgress = true;
      } else {
        unsigned long pressDuration = millis() - pressStart;
        pressInProgress = false;

        if (pressDuration < 500) {
          clickCount++;
          lastPressTime = millis();
        } else {
          // довге натискання — обробка лише для INTERACTIVE
          if (currentMode == INTERACTIVE) {
            saberHue = (saberHue + 32) % 256;
            playTone(LIGHTSABER_SWING_FREQ, 150);
          }
        }
      }
    }
  }

  // обробка clickCount (всегда працює)
  if (clickCount > 0 && (millis() - lastPressTime) > clickTimeout) {
    switch (clickCount) {
      case 1:
        shortBeep();
        if (currentMode == ANIMATION)
          currentEffect = (currentEffect + 1) % numEffects;
        else if (currentMode == POLICE)
          currentFlasher = (currentFlasher == POLICE_FLASHER) ? YELLOW_BLUE_FLASHER : POLICE_FLASHER;
        else if (currentMode == INTERACTIVE) {
          // коротке натискання — вкл/викл меч
          saberActive = !saberActive;
          saberPowering = true;
          if (saberActive)
            playTone(LIGHTSABER_POWERUP_FREQ, 300);
          else
            playTone(LIGHTSABER_POWERDOWN_FREQ, 300);
        } else {
          changeBrightness = !changeBrightness;
        }
        break;

      case 2:
        longBeep();
        currentMode = (Mode)((currentMode + 1) % 6);  // або скільки у тебе їх є
        break;
    }
    clickCount = 0;
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
  segment_showDigit(0, 'M');
  segment_showDigit(1, '0' + mode);
}

// Оновлюємо lightsaberEffect()
void lightsaberEffect() {
  static unsigned long lastUpdate = 0;
  unsigned long now = millis();

  if (now - lastUpdate < SABER_ANIMATION_SPEED) return;
  lastUpdate = now;

  // Анімація запалювання/затухання
  if (saberPowering) {
    if (saberActive) {
      // Запалювання
      saberBrightness = min(255, saberBrightness + POWER_ANIMATION_STEP);
      if (saberBrightness == 255) saberPowering = false;
    } else {
      // Затухання
      saberBrightness = max(0, saberBrightness - POWER_ANIMATION_STEP);
      if (saberBrightness == 0) saberPowering = false;
    }
  }

  // Очищаємо стрічку
  strip.clear();

  // Малюємо меч, якщо він не вимкнений
  if (saberBrightness > 0) {
    // Рухаємо позицію меча
    saberPosition = (saberPosition + 1) % LED_COUNT;
    
    // Малюємо меч з градієнтом
    for (int i = 0; i < SABER_LENGTH; i++) {
      uint8_t pos = (saberPosition + i) % LED_COUNT;
      uint8_t brightness = saberBrightness * (SABER_LENGTH - i) / SABER_LENGTH;
      strip.setPixelColor(pos, strip.ColorHSV(saberHue, 255, brightness));
    }
    
    // Яскравий кінчик
    strip.setPixelColor(saberPosition, strip.ColorHSV(saberHue, 200, saberBrightness));
    
    // Випадкові звукові ефекти
    if (random(10) < 2) {
      playTone(LIGHTSABER_HUM_FREQ + random(-30, 30), 50 + random(50));
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
  if (now - lastChange > 15) { // Замість 4ms для повільнішого руху
    lastChange = now;
    phase += 0.12; // Зменшений крок для повільнішої зміни
    
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
    case 1: mirrorFlow(); break;
    case 2: lightningEffect(); break;
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

void mirrorFlow() {
  const int center = 15; // Центр стрічки (30 пікселів складено пополам)
  static float pos = 0;
  
  // Очищення стрічки (аналог fadeToBlackBy)
  for(int i = 0; i < LED_COUNT; i++) {
    strip.setPixelColor(i, strip.Color(0, 0, 0));
  }
  
  // Генеруємо кольори, що рухаються від центру
  for(int i = 0; i < center; i++) {
    // Конвертуємо HSV в RGB (H - відтінок, S=255 - насиченість, V=255 - яскравість)
    uint32_t color = strip.ColorHSV((uint32_t)(pos * 100 + i * 50), 255, 255);
    
    // Кольори для першої половини
    strip.setPixelColor(center - 1 - i, color);
    // Дзеркальне відображення для другої половини
    strip.setPixelColor(center + i, color);
  }
  
  pos += 0.5; // Швидкість анімації
  if(pos > 25) pos = 0;
  
  strip.show();
  delay(30);
}

void resetMeteor(int index) {
  meteors[index].position = random(-20, -5);
  meteors[index].brightness = random(150, 255);
  meteors[index].speed = random(2, 5);
  switch (random(3)) {
    case 0: meteors[index].color = strip.Color(100, 100, 255); break;
    case 1: meteors[index].color = strip.Color(150, 50, 255); break;
    case 2: meteors[index].color = strip.Color(200, 200, 255); break;
  }
}

void lightningEffect() {
  static unsigned long lastUpdate = 0;
  static bool initialized = false;

  if (!initialized) {
    for (int i = 0; i < 5; i++) {
      resetMeteor(i);
    }
    initialized = true;
  }

  unsigned long now = millis();
  
  if (now - lastUpdate > 30) {
    lastUpdate = now;
    strip.clear();

    for (int i = 0; i < 5; i++) {
      meteors[i].position += meteors[i].speed;
      
      if (meteors[i].position >= LED_COUNT + 10) {
        resetMeteor(i);
        continue;
      }

      for (int j = 0; j < 8; j++) {
        int pos = meteors[i].position - j;
        if (pos >= 0 && pos < LED_COUNT) {
          uint8_t tailBrightness = meteors[i].brightness * (8 - j) / 8;
          uint32_t color = meteors[i].color;
          uint8_t r = (color >> 16) & 0xFF;
          uint8_t g = (color >> 8) & 0xFF;
          uint8_t b = color & 0xFF;
          r = r * tailBrightness / 255;
          g = g * tailBrightness / 255;
          b = b * tailBrightness / 255;
          strip.setPixelColor(pos, strip.Color(r, g, b));
        }
      }
    }
    strip.show();
  }
}
