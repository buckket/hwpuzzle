#include "led.h"

void LED_init(void)
{
    // Set LED0, LED1, LED2, LED3 pins as output
    VPORTC.DIR |= LED0_PIN | LED1_PIN | LED2_PIN | LED3_PIN;
}

void LED_set(uint8_t out)
{
    uint8_t x = 0;
    switch (out) {
    case 0:
        x = LED0_PIN;
        break;
    case 1:
        x = LED1_PIN;
        break;
    case 2:
        x = LED2_PIN;
        break;
    case 3:
        x = LED3_PIN;
        break;
    case 4:
        x = LED0_PIN | LED1_PIN | LED2_PIN | LED3_PIN;
        break;
    case 5:
        x = LED0_PIN | LED2_PIN;
        break;
    case 6:
        x = LED1_PIN | LED3_PIN;
        break;
    default:
        x = 0;
    }
    VPORTC.OUT = ~x;
}
