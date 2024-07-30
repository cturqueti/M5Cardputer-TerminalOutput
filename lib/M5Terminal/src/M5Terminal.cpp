#include "M5Terminal.h"

M5Terminal::M5Terminal() : bufferIndex(0), startLine(0), lineHeight(20), canvas(&display) {
    for (int i = 0; i < 100; i++) {
        buffer[i] = "";  // Inicializa o buffer com linhas vazias
    }
}

void M5Terminal::begin() {
    display.begin();
    display.setRotation(1);
    display.fillScreen(TFT_BLACK);
    display.setTextColor(TFT_WHITE, TFT_BLACK);
    display.setTextSize(2);
    canvas.createSprite(display.width(), display.height());
    canvas.fillSprite(TFT_BLACK);
    canvas.setTextColor(TFT_GREEN, TFT_BLACK);
    canvas.setTextFont(&fonts::DejaVu12);
}

void M5Terminal::print(const char* text) {
    buffer[bufferIndex] += text;
    updateCanvas();
}

void M5Terminal::println(const char* text) {
    // Adiciona o texto e pula para a próxima linha do buffer
    buffer[bufferIndex] += text;
    bufferIndex++;
    // bufferIndex = (bufferIndex + 1) % 100;
    if (bufferIndex > 6) {
        scrollUp();  // Rolagem para cima se o buffer estiver cheio
    }
    // Atualiza o canvas para refletir as mudanças
    updateCanvas();
}

void M5Terminal::clear() {
    canvas.fillSprite(TFT_BLACK);
    bufferIndex = 0;
    startLine = 0;
    // Limpa o buffer
    for (int i = 0; i < 100; i++) {
        buffer[i] = "";
    }
    updateCanvas();
}

void M5Terminal::scrollUp() {
    startLine = (startLine + 1) % 100;
    updateCanvas();
}

void M5Terminal::scrollDown() {
    startLine = (startLine - 1 + 100) % 100;
    updateCanvas();
}

void M5Terminal::updateCanvas() {
    canvas.fillSprite(TFT_BLACK);
    int line = startLine;
    int y = 0;  // Inicializa a posição vertical para o texto

    for (int i = 0; i < 7; i++) {
        if (buffer[line].length() > 0) {
            canvas.drawString(buffer[line], 0, y);
        }
        line = (line + 1) % 100;
        y += lineHeight;  // Atualiza a posição vertical para a próxima linha
    }
    canvas.pushSprite(0, 0);
}
