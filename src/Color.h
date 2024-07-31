#ifndef COLOR_H
#define COLOR_H

#include <stdint.h>

class Color {
   public:
    // Method to convert RGB to RGB565
    static uint16_t toRgb565(int R, int G, int B);
};

#endif  // COLOR_H
