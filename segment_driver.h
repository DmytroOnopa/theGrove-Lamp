#ifndef SEGMENT_DRIVER_H
#define SEGMENT_DRIVER_H

#include <Arduino.h>

struct Segment;

extern Segment segments[24];

void segment_init();
void segment_clear();
void segment_update();
void segment_activate(int n);
void segment_showChar(uint8_t digitIndex, char c);
void segment_showCustom(uint8_t digitIndex, const uint8_t pattern[7]);
void showBrightnessBar(uint8_t brightness);
void segment_showNumber(uint16_t number);
void testIndicator();
void segment_clearDigit(uint8_t digitIndex);
void segment_showGradient(uint8_t level); // 0-9

#endif