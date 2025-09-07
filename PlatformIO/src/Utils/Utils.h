#include <Arduino.h>
void debounceDelay(uint32_t ms)
{
    uint32_t start = millis();
    while (millis() - start < ms)
    {
        // Just wait
        vTaskDelay(1);
    }
}