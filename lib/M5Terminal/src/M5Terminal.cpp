#include "M5Terminal.h"

M5Terminal::M5Terminal()
    : display(), canvas(&display), currentLine(0), displayStartLine(0), cursorX(0), cursorY(0), lineHeight(10) {
    // Configuração inicial
    display.begin();  // Inicializa o display

    if (display.isEPD()) {
        display.setEpdMode(epd_mode_t::epd_fastest);
        display.invertDisplay(true);
        display.clear(TFT_BLACK);
    }
    if (display.width() < display.height()) {
        display.setRotation(display.getRotation() ^ 1);
    }

    canvas.setColorDepth(1);  // mono color
    canvas.createSprite(display.width(), display.height());
    canvas.setTextFont(&fonts::DejaVu9);

    canvas.setTextScroll(true);
}

// Implementar os métodos de M5Terminal aqui
void M5Terminal::print(const char* text) {
    wrapText(text);
    updateDisplay();
}

void M5Terminal::println(const char* text) {
    print(text);
    newLine();
}

void M5Terminal::printf(const char* format, ...) {
    char buffer[MAX_LINE_LENGTH];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    print(buffer);
}

void M5Terminal::scrollUp() {
    if (displayStartLine > 0) {
        displayStartLine--;
        updateDisplay();
    }
}

void M5Terminal::scrollDown() {
    if (displayStartLine < MAX_LINES - 1) {
        displayStartLine++;
        updateDisplay();
    }
}

void M5Terminal::clear() {
    for (int i = 0; i < MAX_LINES; i++) {
        lineBuffer[i] = "";
    }
    currentLine = 0;
    displayStartLine = 0;
    updateDisplay();
}

void M5Terminal::updateDisplay() {
    canvas.fillScreen(BLACK);  // Limpa a tela
    for (int i = 0; i < MAX_LINES; i++) {
        int lineIndex = (displayStartLine + i) % MAX_LINES;
        canvas.setCursor(cursorX, cursorY + i * lineHeight);
        canvas.print(lineBuffer[lineIndex]);
    }
    canvas.pushSprite(0, 0);  // Atualiza a tela
}

void M5Terminal::newLine() {
    currentLine++;
    if (currentLine >= MAX_LINES) {
        currentLine = MAX_LINES - 1;
        scrollUp();
    }
}

void M5Terminal::wrapText(const char* text) {
    String inputText = String(text);
    int length = inputText.length();
    int index = 0;
    while (index < length) {
        int lineLength = min(MAX_LINE_LENGTH, length - index);
        lineBuffer[currentLine] = inputText.substring(index, index + lineLength);
        newLine();
        index += lineLength;
    }
}
