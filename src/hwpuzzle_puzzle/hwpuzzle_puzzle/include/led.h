#ifndef LED_H_
#define LED_H_

#include <avr/io.h>

// PORTC
#define LED0_PIN		PIN0_bm
#define LED1_PIN		PIN1_bm
#define LED2_PIN		PIN2_bm
#define LED3_PIN		PIN3_bm

void LED_init(void);
void LED_set(uint8_t out);

#endif /* LED_H_ */
