#ifndef M5TERMINAL_H
#define M5TERMINAL_H

#include <M5Cardputer.h>

#include <deque>
#include <string>

// Verifique o tipo correto, pode ser M5GFX ou outro tipo apropriado
typedef M5GFX M5Display;  // Ajuste conforme necessário

class M5Terminal {
   public:
    M5Terminal(M5Display* display);
    void begin();
    void print(const char* text);
    void println(const char* text);
    void clear();
    void scrollUp();
    void scrollDown();

   private:
    M5Display* _display;  // Ajuste para M5Display
    M5Canvas* _canvas;
    std::deque<std::string> _buffer;
    int _maxLines = 100;
    int _lineHeight;
    int _startLine;
    void updateCanvas();
    void autoScroll();
};

#endif  // M5TERMINAL_H
