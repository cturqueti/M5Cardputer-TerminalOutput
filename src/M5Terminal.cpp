#include "M5Terminal.h"

void terminalEvent(void) __attribute__((weak));
void terminalEvent(void) {}

M5Terminal::M5Terminal(M5Display* display) : _display(display), _startLine(0), _lineHeight(12) {
    _display->setRotation(1);
    if (_display->isEPD()) {
        _display->setEpdMode(epd_mode_t::epd_fastest);
        _display->invertDisplay(true);
        _display->clear(_backgroundColor);
    }
    if (_display->width() < _display->height()) {
        _display->setRotation(_display->getRotation() ^ 1);
    }
    _canvas = new M5Canvas(_display);
    _lock = xSemaphoreCreateMutex();  // Create a mutex
    if (_lock == NULL) {
        std::cout
            << "Failed to create mutex" << std::endl;

        // Handle the error
    }
}

M5Terminal::~M5Terminal() {
    // end();  // explicit Full UART termination
#if !CONFIG_DISABLE_HAL_LOCKS
    if (_lock != NULL) {
        vSemaphoreDelete(_lock);
    }
#endif
}

void M5Terminal::begin() {
    if (xSemaphoreTake(_lock, portMAX_DELAY) == pdTRUE) {
        _canvas->fillSprite(_backgroundColor);
        _canvas->createSprite(_display->width(), _display->height());
        _canvas->setTextColor(_textColor, _backgroundColor);
        _canvas->setFont(&fonts::DejaVu12);
        _canvas->setTextScroll(true);

        xSemaphoreGive(_lock);
    }
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
    if (xSemaphoreTake(_lock, portMAX_DELAY) == pdTRUE) {
        _canvas->fillScreen(_backgroundColor);
        xSemaphoreGive(_lock);
    }
    updateOutputWindow();
    updateInputWindow();
    drawFnIndicator();
    drawFrame();

    if (xSemaphoreTake(_lock, portMAX_DELAY) == pdTRUE) {
        _canvas->pushSprite(0, 0);
        xSemaphoreGive(_lock);
    }
}

void M5Terminal::updateInputWindow() {
    const int leftMargin = 5;

    // Calculate the bottom line Y position of the display
    int y = _canvas->height() - _lineHeight;
    if (xSemaphoreTake(_lock, portMAX_DELAY) == pdTRUE) {
        if (!_inBuffer.empty()) {
            _canvas->drawString(_inBuffer[0].c_str(), leftMargin - _scrollX, y);
        } else {
            _inBuffer.push_back(">");
        }
        xSemaphoreGive(_lock);
    }
}

void M5Terminal::updateOutputWindow() {
    const int leftMargin = 5;
    int line = _startLine;
    int y = 0;

    int numVisibleLines = _canvas->height() / _lineHeight;
    if (xSemaphoreTake(_lock, portMAX_DELAY) == pdTRUE) {
        for (int i = 0; i < (numVisibleLines - 1) && line < _outBuffer.size(); i++) {
            if (_outBuffer[line].length() > 0) {
                _canvas->drawString(_outBuffer[line].c_str(), leftMargin - _scrollX, y);  // Apply horizontal scrolling
            }
            line++;
            y += _lineHeight;
        }
        xSemaphoreGive(_lock);
    }
    drawScrollIndicator();
}

void M5Terminal::drawFrame() {
    if (xSemaphoreTake(_lock, portMAX_DELAY) == pdTRUE) {
        // Desenhar a moldura ao redor da área de saída de texto
        _canvas->drawRect(0, 0, _canvas->width(), _canvas->height() - _lineHeight, _borderColor);

        // Desenhar uma linha separadora entre a área de saída de texto e a linha de entrada de texto
        _canvas->drawLine(0, _canvas->height() - _lineHeight, _canvas->width(), _canvas->height() - _lineHeight, _borderColor);
        xSemaphoreGive(_lock);
    }
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
    if (xSemaphoreTake(_lock, portMAX_DELAY) == pdTRUE) {
        // Clear the scroll bar area
        _canvas->fillRect(_canvas->width() - 5, 0, 5, _canvas->height() - _lineHeight, TFT_BLACK);

        // Draw the scroll bar
        _canvas->fillRect(_canvas->width() - 5, indicatorY, 5, indicatorHeight, _scrollColor);

        // Add a border for visibility
        _canvas->drawRect(_canvas->width() - 5, 0, 5, _canvas->height() - _lineHeight, _borderColor);
        _canvas->drawRect(_canvas->width() - 5, indicatorY, 5, indicatorHeight, Color::toRgb565(0, 150, 150));
        xSemaphoreGive(_lock);
    }
}

void M5Terminal::handleKeyboardInput() {
    if (M5Cardputer.Keyboard.isChange()) {
        if (M5Cardputer.Keyboard.isPressed()) {
            Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();
            // std::vector<Point2D_t> _keyCoord = M5Cardputer.Keyboard.keyList();
            if (status.fn) {
                _flags.fnFlag = !_flags.fnFlag;
                _flags.altFlag = false;
                _flags.shiftFlag = false;
                _flags.optFlag = false;
                _flags.ctrlFlag = false;
                delay(50);
            } else if (status.alt) {
                _flags.fnFlag = false;
                _flags.altFlag = !_flags.altFlag;
                _flags.shiftFlag = false;
                _flags.optFlag = false;
                _flags.ctrlFlag = false;
                delay(50);
            } else if (status.shift) {
                _flags.fnFlag = false;
                _flags.altFlag = false;
                _flags.shiftFlag = !_flags.shiftFlag;
                M5Cardputer.Keyboard.setCapsLocked(_flags.shiftFlag);
                _flags.optFlag = false;
                _flags.ctrlFlag = false;
                delay(50);
            } else if (status.opt) {
                _flags.fnFlag = false;
                _flags.altFlag = false;
                _flags.shiftFlag = false;
                _flags.optFlag = !_flags.optFlag;
                _flags.ctrlFlag = false;
                delay(50);
            } else if (status.ctrl) {
                _flags.fnFlag = false;
                _flags.altFlag = false;
                _flags.shiftFlag = false;
                _flags.optFlag = false;
                _flags.ctrlFlag = !_flags.ctrlFlag;
                delay(50);
            } else if (status.enter) {
                // Handle enter key
                String output = sendInput();
                println(output.c_str());
            } else if (status.del) {
                // Handle backspace/delete key
                backSpaceInput();
            } else if (!_flags.altFlag && !_flags.ctrlFlag && !_flags.fnFlag && !_flags.optFlag) {
                // Se a flag shift estiver ativada, converte a entrada para maiúsculas

                for (auto key : status.word) {
                    refreshInput(String(key).c_str());
                }
            }
        }
    } else if (M5Cardputer.Keyboard.isPressed() && _flags.fnFlag) {
        Keyboard_Class::KeysState fn = M5Cardputer.Keyboard.keysState();
        for (auto key : fn.word) {
            if (key == KEY_UP) {
                scrollUp();
            } else if (key == KEY_DOWN) {
                scrollDown();
            } else if (key == KEY_LEFT) {
                scrollLeft();
            } else if (key == KEY_RIGHT) {
                scrollRight();
            }
        }
    }
    M5Cardputer.update();
    updateCanvas();
}

void M5Terminal::drawFnIndicator() {
    // Define the position and size of the indicator
    int indicatorWidth = 40;
    int indicatorHeight = 15;
    int x = _canvas->width() - indicatorWidth - 10;  // 5 pixels padding from the right edge
    int y = 5;                                       // 5 pixels padding from the top edge
    if (_flags.fnFlag) {
        if (xSemaphoreTake(_lock, portMAX_DELAY) == pdTRUE) {
            // Draw the background rectangle
            _canvas->fillRect(x, y, indicatorWidth, indicatorHeight, TFT_ORANGE);

            // Draw the border of the rectangle
            _canvas->drawRect(x, y, indicatorWidth, indicatorHeight, TFT_WHITE);

            // Draw the "FN" text inside the rectangle

            _canvas->setTextColor(WHITE);
            _canvas->setTextSize(1);
            _canvas->drawString("FN", x + 5, y + 3);  // Position the text inside the rectangle
            _canvas->setTextColor(_textColor);
            xSemaphoreGive(_lock);
        }
    } else if (_flags.altFlag) {
        if (xSemaphoreTake(_lock, portMAX_DELAY) == pdTRUE) {
            // Draw the background rectangle
            //_canvas->fillRect(x, y, indicatorWidth, indicatorHeight, TFT_ORANGE);

            // Draw the border of the rectangle
            _canvas->drawRect(x, y, indicatorWidth, indicatorHeight, TFT_WHITE);

            // Draw the "FN" text inside the rectangle

            _canvas->setTextColor(WHITE);
            _canvas->setTextSize(1);
            _canvas->drawString("ALT", x + 5, y + 3);  // Position the text inside the rectangle
            _canvas->setTextColor(_textColor);
            xSemaphoreGive(_lock);
        }

    } else if (_flags.ctrlFlag) {
        if (xSemaphoreTake(_lock, portMAX_DELAY) == pdTRUE) {
            // Draw the background rectangle
            //_canvas->fillRect(x, y, indicatorWidth, indicatorHeight, TFT_ORANGE);

            // Draw the border of the rectangle
            _canvas->drawRect(x, y, indicatorWidth, indicatorHeight, TFT_WHITE);

            // Draw the "FN" text inside the rectangle

            _canvas->setTextColor(WHITE);
            _canvas->setTextSize(1);
            _canvas->drawString("CTRL", x + 5, y + 3);  // Position the text inside the rectangle
            _canvas->setTextColor(_textColor);
            xSemaphoreGive(_lock);
        }

    } else if (_flags.optFlag) {
        if (xSemaphoreTake(_lock, portMAX_DELAY) == pdTRUE) {
            // Draw the background rectangle
            _canvas->fillRect(x, y, indicatorWidth, indicatorHeight, TFT_DARKGREEN);

            // Draw the border of the rectangle
            _canvas->drawRect(x, y, indicatorWidth, indicatorHeight, TFT_WHITE);

            // Draw the "FN" text inside the rectangle

            _canvas->setTextColor(WHITE);
            _canvas->setTextSize(1);
            _canvas->drawString("OPT", x + 5, y + 3);  // Position the text inside the rectangle
            _canvas->setTextColor(_textColor);
            xSemaphoreGive(_lock);
        }

    } else if (_flags.shiftFlag) {
        if (xSemaphoreTake(_lock, portMAX_DELAY) == pdTRUE) {
            // Draw the background rectangle
            _canvas->fillRect(x, y, indicatorWidth, indicatorHeight, TFT_BLUE);

            // Draw the border of the rectangle
            _canvas->drawRect(x, y, indicatorWidth, indicatorHeight, TFT_WHITE);

            // Draw the "FN" text inside the rectangle

            _canvas->setTextColor(WHITE);
            _canvas->setTextSize(1);
            _canvas->drawString("SH", x + 5, y + 3);  // Position the text inside the rectangle
            _canvas->setTextColor(_textColor);
            xSemaphoreGive(_lock);
        }
    }
}

int M5Terminal::available(void) {
    return 0;
}

void M5Terminal::read() {
}