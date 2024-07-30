#include <Arduino.h>
#include <M5Cardputer.h>

#include "M5Terminal.h"

M5Terminal terminal;

void setup() {
    // M5Cardputer.begin(true);
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
        delay(1000);
    }
    // if (M5Cardputer.Keyboard.isKeyPressed(';')) {
    //     // canvas.println("------>passou por aqui<-------");
    //     // delay(2000);
    //     terminal.print(";");
    // }
    // M5Cardputer.update();
    delay(200);
}
