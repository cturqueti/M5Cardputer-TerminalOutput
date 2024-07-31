#include "M5Terminal.h"

M5Terminal::M5Terminal(M5Display* display) : _display(display), _startLine(0), _lineHeight(12) {
    _display->setRotation(1);
    if (_display->isEPD()) {
        _display->setEpdMode(epd_mode_t::epd_fastest);
        _display->invertDisplay(true);
        _display->clear(TFT_BLACK);
    }
    if (_display->width() < _display->height()) {
        _display->setRotation(_display->getRotation() ^ 1);
    }
    _canvas = new M5Canvas(_display);
}

void M5Terminal::begin() {
    // _canvas->setColorDepth(1);  // mono color
    _canvas->fillSprite(TFT_BLACK);
    _canvas->createSprite(_display->width(), _display->height());
    _canvas->setTextColor(TFT_GREEN, TFT_BLACK);
    _canvas->setTextFont(&fonts::DejaVu12);
    //_canvas->setTextColor(TFT_GREEN);
    _canvas->setTextScroll(true);

    clear();
}

void M5Terminal::print(const char* str) {
    if (_buffer.size() >= _maxLines) {
        _buffer.pop_front();
    }

    if (_buffer.empty()) {
        _buffer.push_back(std::string(str));
    } else {
        _buffer.back() += std::string(str);
    }

    updateCanvas();
}

void M5Terminal::println(const char* str) {
    print(str);
    if (_buffer.size() >= _maxLines) {
        _buffer.pop_front();
    }
    _buffer.push_back("");
    autoScroll();
    updateCanvas();
}

void M5Terminal::clear() {
    _buffer.clear();
    updateCanvas();
}

void M5Terminal::scrollUp() {
    if (_startLine > 0) {
        _startLine--;
        updateCanvas();
    }
}

void M5Terminal::scrollDown() {
    if (_startLine < _buffer.size() - 1) {
        _startLine++;
        updateCanvas();
    }
}

void M5Terminal::scrollLeft() {
    if (_scrollX > 0) {
        _scrollX -= 5;
        updateCanvas();
    } else if (_scrollX < 0) {
        _scrollX = 0;
    }
}

void M5Terminal::scrollRight() {
    int maxScrollX = _canvas->width() - _canvas->textWidth(" ");  // Ajuste conforme necessário
    if (_scrollX < maxScrollX) {
        _scrollX += 5;
        updateCanvas();
    }
}

void M5Terminal::autoScroll() {
    int numVisibleLines = _canvas->height() / _lineHeight;
    if (_buffer.size() > numVisibleLines) {
        _startLine = _buffer.size() - numVisibleLines;
    }
}

void M5Terminal::updateCanvas() {
    _canvas->fillScreen(TFT_BLACK);
    int line = _startLine;
    int y = 0;

    int numVisibleLines = _canvas->height() / _lineHeight;

    for (int i = 0; i < numVisibleLines && line < _buffer.size(); i++) {
        if (_buffer[line].length() > 0) {
            _canvas->drawString(_buffer[line].c_str(), -_scrollX, y);  // Aplicar rolagem horizontal
        }
        line++;
        y += _lineHeight;
    }

    _canvas->pushSprite(0, 0);
}
