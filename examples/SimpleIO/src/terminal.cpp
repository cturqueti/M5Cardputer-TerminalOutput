#include <Arduino.h>
#include <M5Cardputer.h>

#include "M5Terminal.h"

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
    static bool fnFlag = false;
    static int counter = 0;
    if (counter < 30) {
        terminal.print("Contador: ");
        terminal.println(String(counter).c_str());
        counter++;
        if (counter == 29) {
            terminal.println("testando o tamanho da linha e o left e right");
        }
        delay(50);
    }
    if (M5Cardputer.Keyboard.isChange()) {
        if (M5Cardputer.Keyboard.isPressed()) {
            Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();
            if (status.fn) {
                fnFlag = !fnFlag;
                delay(50);

            } else if (status.enter) {
                terminal.print("Enviando:");
                String output = terminal.sendInput();
                terminal.println(output.c_str());

            } else if (status.del) {
                terminal.backSpaceInput();
            } else if (!fnFlag) {
                for (auto key : status.word) {
                    terminal.refreshInput(String(key).c_str());
                }
            }
        }
    } else if (M5Cardputer.Keyboard.isPressed() && fnFlag) {
        Keyboard_Class::KeysState fn = M5Cardputer.Keyboard.keysState();
        for (auto key : fn.word) {
            if (key == KEY_UP) {
                terminal.scrollUp();
            } else if (key == KEY_DOWN) {
                terminal.scrollDown();
            } else if (key == KEY_LEFT) {
                terminal.scrollLeft();
            } else if (key == KEY_RIGHT) {
                terminal.scrollRight();
            }
        }
    }

    M5Cardputer.update();
    delay(50);
}
