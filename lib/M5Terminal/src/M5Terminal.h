#ifndef M5TERMINAL_H
#define M5TERMINAL_H

#include <M5GFX.h>

class M5Terminal {
   public:
    M5Terminal();
    void begin();
    void print(const char* text);
    void println(const char* text);
    void clear();
    void scrollUp();
    void scrollDown();

   private:
    M5GFX display;
    M5Canvas canvas;
    String buffer[100];
    int bufferIndex;
    int startLine;
    int lineHeight;
    void updateCanvas();
};

#endif  // M5TERMINAL_H
