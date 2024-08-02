#include <Arduino.h>
#include <M5Cardputer.h>

#include "M5Terminal.h"

M5Terminal terminal(&M5Cardputer.Display);

// Task function that will run in a separate thread
void handleKeyboardTask(void *pvParameters) {
    while (true) {
        terminal.handleKeyboardInput();  // Update keyboard input
        vTaskDelay(pdMS_TO_TICKS(50));   // Wait for 50 ms
    }
}

void setup() {
    auto cfg = M5.config();
    M5Cardputer.begin(cfg);

    terminal.begin();
    terminal.clear();
    terminal.println("Terminal test");

    // Create the FreeRTOS task
    xTaskCreate(handleKeyboardTask, "HandleKeyboardTask", 4096 / sizeof(StackType_t), NULL, 1, NULL);
}

void loop() {
    static bool fnFlag = false;
    static int counter = 0;
    if (counter < 30) {
        terminal.print("Counter: ");
        terminal.println(String(counter).c_str());
        counter++;
        std::cout
            << "Olá, mundo!" << std::endl;

        if (counter == 29) {
            terminal.println("Testing line length and left and right");
        }
        delay(50);
    }
}
