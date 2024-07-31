#include <Arduino.h>
#include <M5Cardputer.h>

#include "M5Terminal.h"

#define KEY_UP ';'
#define KEY_DOWN '.'
#define KEY_LEFT ','
#define KEY_RIGHT '/'

M5Terminal terminal(&M5Cardputer.Display);
String userInput = "> ";

void handleBackspace();

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

            } else if (!fnFlag) {
                for (auto key : status.word) {
                    userInput += key;
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

void handleBackspace() {
    // Check if there is text to delete
    if (userInput.length() > 2) {  // "> " length is 2
        // Remove the last character from userInput
        userInput.remove(userInput.length() - 1);
        M5Cardputer.Display.fillScreen(BLACK);

        // Display updated user input
        // displayUserInput();
    }
}
