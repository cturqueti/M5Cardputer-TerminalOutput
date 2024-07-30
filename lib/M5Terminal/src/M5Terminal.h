#ifndef M5TERMINAL_H
#define M5TERMINAL_H

#include <Arduino.h>
#include <M5GFX.h>

#include "M5Cardputer.h"

#define MAX_LINES 100
#define MAX_LINE_LENGTH 27
#define BLACK 0x0000

class M5Terminal {
   public:
    M5Terminal();

    void print(const char* text);
    void println(const char* text);
    void printf(const char* format, ...);  // Adicione esta linha
    void scrollUp();
    void scrollDown();
    void clear();

   private:
    void updateDisplay();
    void newLine();
    void wrapText(const char* text);

    M5GFX display;
    M5Canvas canvas;

    String lineBuffer[MAX_LINES];
    int currentLine;
    int displayStartLine;
    int cursorX;
    int cursorY;
    int lineHeight;
};

#endif  // DISPLAY_H
