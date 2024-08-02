
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

struct Flags {
    uint8_t fnFlag : 1;     // 1 bit para fnFlag
    uint8_t altFlag : 1;    // 1 bit para altFlag
    uint8_t shiftFlag : 1;  // 1 bit para capsFlag
    uint8_t ctrlFlag : 1;   //
    uint8_t optFlag : 1;
};

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
    void handleKeyboardInput();

   private:
    M5Display* _display;  // Adjust to M5Display
    M5Canvas* _canvas;
    std::deque<std::string> _outBuffer;
    std::deque<std::string> _inBuffer;
    int _maxLines = 100;
    int _lineHeight;
    int _startLine;
    int _scrollX = 0;
    Flags _flags;
    bool _fnFlag = false;
    bool _altFlag = false;
    bool _capsFlag = false;
    bool _ctrlFlag = false;
    uint16_t _textColor = Color::toRgb565(50, 255, 50);
    uint16_t _backgroundColor = Color::toRgb565(0, 0, 0);
    uint16_t _borderColor = Color::toRgb565(192, 192, 192);
    uint16_t _scrollColor = Color::toRgb565(0, 200, 150);
    SemaphoreHandle_t _canvasMutex;

    void
    updateOutputWindow();
    void updateInputWindow();
    void autoScroll();
    void drawScrollIndicator();
    void drawFnIndicator();
};

#endif  // M5TERMINAL_H