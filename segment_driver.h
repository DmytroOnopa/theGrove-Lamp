#ifndef SEGMENT_DRIVER_H
#define SEGMENT_DRIVER_H

#include <Arduino.h>

struct Segment;

extern Segment segments[24];

void segment_init();
void segment_clear();
void segment_update();
void segment_activate(int n);
void segment_showDigit(uint8_t digitIndex, char c);
void segment_showNumber(uint16_t number);
void displayBrightness(uint8_t val);
void testIndicator();

#endif