#include <Arduino.h>
#include <M5Cardputer.h>

#include "M5Terminal.h"

#define KEY_UP ';'
#define KEY_DOWN '.'
#define KEY_LEFT ','
#define KEY_RIGHT '/'

M5Terminal terminal(&M5Cardputer.Display);

void setup() {
    auto cfg = M5.config();
    M5Cardputer.begin(cfg);

    Serial.begin(115200);
    terminal.begin();
    terminal.clear();
    terminal.println("Teste de terminal");
}

void loop() {
    static int counter = 0;
    if (counter < 30) {
        terminal.print("Contador: ");
        terminal.println(String(counter).c_str());
        counter++;
        if (counter == 29) {
            terminal.println("testando o tamanho da linha e o left e right");
        }
        delay(100);
    }
    if (M5Cardputer.Keyboard.isKeyPressed(KEY_FN) && M5Cardputer.Keyboard.isKeyPressed(KEY_UP)) {
        terminal.scrollUp();
    } else if (M5Cardputer.Keyboard.isKeyPressed(KEY_FN) && M5Cardputer.Keyboard.isKeyPressed(KEY_DOWN)) {
        terminal.scrollDown();
    } else if (M5Cardputer.Keyboard.isKeyPressed(KEY_FN) && M5Cardputer.Keyboard.isKeyPressed(KEY_LEFT)) {
        terminal.scrollLeft();
    } else if (M5Cardputer.Keyboard.isKeyPressed(KEY_FN) && M5Cardputer.Keyboard.isKeyPressed(KEY_RIGHT)) {
        terminal.scrollRight();
    }
    M5Cardputer.update();
    delay(50);
}
