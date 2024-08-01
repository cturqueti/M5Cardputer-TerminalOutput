
#ifndef M5TERMINAL_H
#define M5TERMINAL_H

#include <M5Cardputer.h>

#include <deque>
#include <string>

#include "Color.h"

#define KEY_UP ';'
#define KEY_DOWN '.'
#define KEY_LEFT ','
#define KEY_RIGHT '/'

// Verify the correct type; it might be M5GFX or another appropriate type
typedef M5GFX M5Display;  // Adjust as necessary

class M5Terminal {
   public:
    M5Terminal(M5Display* display);
    void begin();
    void print(const char* text);
    void println(const char* text);
    void refreshInput(const char* text);
    String sendInput();
    void clearinput();
    void backSpaceInput();
    void clear();
    void scrollUp();
    void scrollDown();
    void scrollLeft();
    void scrollRight();
    void drawFrame();
    void updateCanvas();

   private:
    M5Display* _display;  // Adjust to M5Display
    M5Canvas* _canvas;
    std::deque<std::string> _outBuffer;
    std::deque<std::string> _inBuffer;
    int _maxLines = 100;
    int _lineHeight;
    int _startLine;
    int _scrollX = 0;
    void updateOutputWindow();
    void updateInputWindow();

    void autoScroll();
    void drawScrollIndicator();
};

#endif  // M5TERMINAL_H