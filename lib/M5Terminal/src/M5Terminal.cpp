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
    _canvas->fillSprite(TFT_BLACK);
    _canvas->createSprite(_display->width(), _display->height());
    _canvas->setTextColor(Color::toRgb565(50, 255, 50), TFT_BLACK);
    _canvas->setFont(&fonts::DejaVu12);
    _canvas->setTextScroll(true);

    clear();
}

void M5Terminal::print(const char* text) {
    if (_outBuffer.size() >= _maxLines) {
        _outBuffer.pop_front();
    }

    if (_outBuffer.empty()) {
        _outBuffer.push_back(std::string(text));
    } else {
        _outBuffer.back() += std::string(text);
    }

    updateCanvas();
}

void M5Terminal::println(const char* text) {
    print(text);
    if (_outBuffer.size() >= _maxLines) {
        _outBuffer.pop_front();
    }
    _outBuffer.push_back("");
    autoScroll();
    updateCanvas();
}

void M5Terminal::refreshInput(const char* text) {
    if (_inBuffer.empty()) {
        _inBuffer.push_back(std::string(text));
    } else {
        _inBuffer.back() += std::string(text);
    }
    updateCanvas();
}

String M5Terminal::sendInput() {
    if (!_inBuffer.empty()) {
        String output = _inBuffer.front().c_str();
        clearinput();
        // Remove o primeiro caractere
        if (output.length() > 0) {
            output = output.substring(1);
        }
        return output;
    }
    return "";
}

void M5Terminal::clearinput() {
    _inBuffer.clear();
    _inBuffer.push_back(">");
    _scrollX = 0;
    updateCanvas();
}
void M5Terminal::backSpaceInput() {
    if (!_inBuffer.empty()) {
        if (!_inBuffer.back().empty()) {
            _inBuffer.back().pop_back();
        }
        if (_inBuffer.back().empty()) {
            _inBuffer.pop_back();
        }
        updateCanvas();
    }
}

void M5Terminal::clear() {
    _outBuffer.clear();
    _scrollX = 0;
    updateCanvas();
}

void M5Terminal::scrollUp() {
    if (_startLine > 0) {
        _startLine--;
        updateCanvas();
    }
}

void M5Terminal::scrollDown() {
    // Calculate the total number of visible lines
    int numVisibleLines = _canvas->height() / _lineHeight;

    // Check if scrolling has reached the lower limit
    if (_startLine < std::max(0, static_cast<int>(_outBuffer.size()) - numVisibleLines)) {
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
    int maxScrollX = _canvas->width() - _canvas->textWidth(" ");  // Adjust as needed
    if (_scrollX < maxScrollX) {
        _scrollX += 5;
        updateCanvas();
    }
}

void M5Terminal::autoScroll() {
    int numVisibleLines = _canvas->height() / _lineHeight;
    if (_outBuffer.size() > numVisibleLines) {
        _startLine = _outBuffer.size() - numVisibleLines;
    }
}

void M5Terminal::updateCanvas() {
    _canvas->fillScreen(TFT_BLACK);
    updateOutputWindow();
    updateInputWindow();
    drawFrame();

    _canvas->pushSprite(0, 0);
}

void M5Terminal::updateInputWindow() {
    const int leftMargin = 5;

    // Calculate the bottom line Y position of the display
    int y = _canvas->height() - _lineHeight;

    if (!_inBuffer.empty()) {
        _canvas->drawString(_inBuffer[0].c_str(), leftMargin - _scrollX, y);
    } else {
        _inBuffer.push_back(">");
    }
}

void M5Terminal::updateOutputWindow() {
    const int leftMargin = 5;
    int line = _startLine;
    int y = 0;

    int numVisibleLines = _canvas->height() / _lineHeight;

    for (int i = 0; i < (numVisibleLines - 1) && line < _outBuffer.size(); i++) {
        if (_outBuffer[line].length() > 0) {
            _canvas->drawString(_outBuffer[line].c_str(), leftMargin - _scrollX, y);  // Apply horizontal scrolling
        }
        line++;
        y += _lineHeight;
    }
    drawScrollIndicator();
}

void M5Terminal::drawFrame() {
    const uint16_t WHITE = Color::toRgb565(192, 192, 192);  // Cor branca no formato RGB565

    // Desenhar a moldura ao redor da área de saída de texto
    _canvas->drawRect(0, 0, _canvas->width(), _canvas->height() - _lineHeight, WHITE);

    // Desenhar uma linha separadora entre a área de saída de texto e a linha de entrada de texto
    _canvas->drawLine(0, _canvas->height() - _lineHeight, _canvas->width(), _canvas->height() - _lineHeight, WHITE);
}

void M5Terminal::drawScrollIndicator() {
    // Calculate the total number of lines and the height of the scroll bar
    int totalLines = _outBuffer.size();
    int numVisibleLines = (_canvas->height() - _lineHeight) / _lineHeight;
    int visibleStartLine = _startLine;
    int visibleEndLine = visibleStartLine + numVisibleLines - 1;

    // Adjust the height of the scroll bar
    int indicatorHeight = map(numVisibleLines, 0, totalLines, 0, _canvas->height() - _lineHeight);
    int indicatorY = map(visibleStartLine, 0, totalLines - numVisibleLines, 0, _canvas->height() - _lineHeight - indicatorHeight);

    // Clear the scroll bar area
    _canvas->fillRect(_canvas->width() - 5, 0, 5, _canvas->height() - _lineHeight, TFT_BLACK);

    // Draw the scroll bar
    _canvas->fillRect(_canvas->width() - 5, indicatorY, 5, indicatorHeight, Color::toRgb565(0, 200, 150));

    // Add a border for visibility
    _canvas->drawRect(_canvas->width() - 5, 0, 5, _canvas->height() - _lineHeight, Color::toRgb565(150, 150, 150));
    _canvas->drawRect(_canvas->width() - 5, indicatorY, 5, indicatorHeight, Color::toRgb565(0, 150, 150));
}