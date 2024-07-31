#include "Color.h"

uint16_t Color::toRgb565(int R, int G, int B) {
    int red565 = (R >> 3) & 0x1F;
    int green565 = (G >> 2) & 0x3F;
    int blue565 = (B >> 3) & 0x1F;
    return (red565 << 11) | (green565 << 5) | blue565;
}
