#include "input.h"

volatile uint8_t key_state;
volatile uint8_t key_press;

ISR(RTC_PIT_vect)
{
    RTC.PITINTFLAGS |= RTC_PI_bm;

    static uint8_t ct0 = 0xFF, ct1 = 0xFF;
    uint8_t i;

    i = key_state ^ ~VPORTA.IN;
    ct0 = ~(ct0 & i);
    ct1 = ct0 ^ (ct1 & i);
    i &= ct0 & ct1;

    key_state ^= i;
    key_press |= key_state & i;
}

uint8_t input_get_key_press(uint8_t key_mask)
{
    cli();
    key_mask &= key_press;
    key_press ^= key_mask;
    sei();
    return key_mask;
}

uint8_t input_get_key_state(uint8_t key_mask)
{
    key_mask &= key_state;
    return key_mask;
}


void input_init(void)
{
    // Set ACK, INPUT0, INPUT1, INPUT2, INPUT3 as input
    VPORTA.DIR &= ~(RESET_PIN | ACK_PIN | INPUT0_PIN | INPUT1_PIN | INPUT2_PIN | INPUT3_PIN);

    // Enable pull-up resistors
    PORTA.PIN2CTRL = PORT_PULLUPEN_bm;
    PORTA.PIN3CTRL = PORT_PULLUPEN_bm;
    PORTA.PIN4CTRL = PORT_PULLUPEN_bm;
    PORTA.PIN5CTRL = PORT_PULLUPEN_bm;
    PORTA.PIN6CTRL = PORT_PULLUPEN_bm;
    PORTA.PIN7CTRL = PORT_PULLUPEN_bm;
}
