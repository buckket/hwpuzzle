#ifndef INPUT_H_
#define INPUT_H_

#include <avr/io.h>
#include <avr/interrupt.h>

// PORTA
#define RESET_PIN		PIN2_bm
#define ACK_PIN			PIN3_bm
#define INPUT0_PIN		PIN4_bm
#define INPUT1_PIN		PIN5_bm
#define INPUT2_PIN		PIN6_bm
#define INPUT3_PIN		PIN7_bm

void input_init(void);
uint8_t input_get_key_press(uint8_t key_mask);
uint8_t input_get_key_state(uint8_t key_mask);

#endif /* INPUT_H_ */