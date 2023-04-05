#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/cpufunc.h>

#include "led.h"
#include "rtc.h"
#include "input.h"

#define STATE_PLAY	0
#define STATE_PLAY_DONE	1
#define STATE_CHECK	2

#define BUFFER_SIZE 100


void base_4(char input, uint8_t* output)
{
    for(uint8_t i = 0; i < 4; i++) {
        output[i] = input % 4;
        input /= 4;
    }
}

void toggle_led(uint8_t *toggle)
{
    if (*toggle > 0) {
        LED_set(5);
        *toggle = 0;
    } else {
        LED_set(6);
        *toggle = 1;
    }
}

int main(void)
{
    // Disable watchdog during setup phase
    wdt_disable();

    // Disable prescaler
    ccp_write_io((void *)&(CLKCTRL.MCLKCTRLB), 0);

    // Enable RTC and PIT interrupt
    RTC_init();
    RTC_PIT_enable();

    // Enable inputs and debounce logic
    input_init();

    // Enable outputs and turn all LEDs off
    LED_set(-1);
    LED_init();

    // Enable again
    wdt_enable(WDTO_15MS);

    // Default state after reset
    uint8_t state = STATE_PLAY;

    char signal_buffer[BUFFER_SIZE] = "Klabauter";
    uint8_t signal_position = 0;

    uint8_t quaternary[4];
    uint8_t quaternary_position = -1;

    uint8_t play_position = 0;
    uint8_t replay_position = 0;

    uint8_t toggle = 0;

    // Reset all detected key events
    input_get_key_press(0xFF);

    while (1) {
        wdt_reset();

        if(input_get_key_press(RESET_PIN)) {
            state = STATE_PLAY;
            signal_position = 0;
            quaternary_position = -1;
            play_position = 0;
            replay_position = 0;
            LED_set(-1);
        }

        switch (state) {
        case STATE_PLAY:
            if (input_get_key_press(ACK_PIN)) {
                // Ask user to replay information
                if (play_position > replay_position) {
                    input_get_key_press(0xFF);
                    state = STATE_CHECK;
                    signal_position = 0;
                    quaternary_position = -1;
                    replay_position = 0;
                    LED_set(-1);
                    break;
                }

                // End of signal reached, we are finished
                if (quaternary_position > 3 && (replay_position/4 > BUFFER_SIZE - 1 || signal_buffer[replay_position/4] == 0)) {
                    state = STATE_PLAY_DONE;
                    toggle_led(&toggle);
                    break;
                }

                // Generate quaternary when needed
                if (quaternary_position > 3) {
                    base_4(signal_buffer[signal_position++], quaternary);
                    quaternary_position = 0;
                }

                LED_set(quaternary[quaternary_position++]);
                play_position++;
            }
            break;
        case STATE_CHECK:
            ;
            uint8_t input = input_get_key_press(ACK_PIN | INPUT0_PIN | INPUT1_PIN | INPUT2_PIN | INPUT3_PIN);

            uint8_t x;
            switch(input) {
            case INPUT0_PIN:
                x = 0;
                break;
            case INPUT1_PIN:
                x = 1;
                break;
            case INPUT2_PIN:
                x = 2;
                break;
            case INPUT3_PIN:
                x = 3;
                break;
            default:
                x = -1;
            }

            // Invalid key press
            if (x > 3) {
                break;
            }

            // Generate quaternary when needed
            if (quaternary_position > 3) {
                base_4(signal_buffer[signal_position++], quaternary);
                quaternary_position = 0;
            }

            // Compare user input
            if (x == quaternary[quaternary_position]) {
                LED_set(quaternary[quaternary_position++]);
                replay_position++;
            } else {
                // Wrong input, display error and reset
                state = STATE_PLAY;
                signal_position = 0;
                quaternary_position = -1;
                play_position = 0;
                replay_position = 0;
                LED_set(4);
                break;
            }

            // Everything entered correctly, continue with sequence
            if (replay_position == play_position) {
                state = STATE_PLAY;
                signal_position = 0;
                quaternary_position = -1;
                play_position = 0;
                LED_set(-1);
                break;
            }
            break;
        case STATE_PLAY_DONE:
            if (input_get_key_press(ACK_PIN)) {
                toggle_led(&toggle);
            }
            break;
        }

        sleep_cpu();
    }
}
