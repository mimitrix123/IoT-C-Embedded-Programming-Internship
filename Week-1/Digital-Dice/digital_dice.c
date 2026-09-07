/*
 * Week 1 Mini Project: Digital Dice
 * IoT & C Embedded Programming Internship
 *
 * Seven LEDs are arranged in a dice pattern. Pressing the button
 * generates a random value from 1 to 6 and displays the matching face.
 * The buzzer gives short audio feedback after each roll.
 *
 * Hardware-specific GPIO/delay functions are represented as placeholders
 * so the logic can be adapted to Arduino, AVR, PIC, STM32, 8051, etc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define LED_COUNT 7

/* LED positions: A B C D E F G
 *
 *     A       B
 *
 *     C   D   E
 *
 *     F       G
 */
enum {
    A = 0,
    B,
    C,
    D,
    E,
    F,
    G
};

/* Dice faces represented as LED bit masks. */
static const unsigned char dice_pattern[7] = {
    0x00,                         /* unused */
    (1u << D),                    /* 1 */
    (1u << A) | (1u << G),        /* 2 */
    (1u << A) | (1u << D) | (1u << G), /* 3 */
    (1u << A) | (1u << B) | (1u << F) | (1u << G), /* 4 */
    (1u << A) | (1u << B) | (1u << D) | (1u << F) | (1u << G), /* 5 */
    (1u << A) | (1u << B) | (1u << C) | (1u << E) |
        (1u << F) | (1u << G) /* 6 */
};

/* Replace these functions with the target MCU's GPIO implementation. */
static void leds_init(void)
{
    /* Configure seven GPIO pins as outputs. */
}

static void button_init(void)
{
    /* Configure button GPIO as input, preferably with pull-up/down. */
}

static void buzzer_init(void)
{
    /* Configure buzzer GPIO/PWM output. */
}

static int button_pressed(void)
{
    /* Return non-zero when the push button is pressed. */
    return 0;
}

static void delay_ms(unsigned int milliseconds)
{
    /* Replace with MCU-specific delay function. */
    (void)milliseconds;
}

static void write_leds(unsigned char pattern)
{
    int i;

    for (i = 0; i < LED_COUNT; ++i) {
        int on = (pattern >> i) & 1u;
        /* Write 'on' to the GPIO pin corresponding to LED i. */
        (void)on;
    }
}

static void buzzer_beep(void)
{
    /* Generate a short tone using GPIO or PWM on the target board. */
    delay_ms(100);
}

static int roll_dice(void)
{
    return (rand() % 6) + 1;
}

static void display_dice(int value)
{
    if (value >= 1 && value <= 6) {
        write_leds(dice_pattern[value]);
    }
}

int main(void)
{
    int dice_value;

    leds_init();
    button_init();
    buzzer_init();

    /* Seed the C pseudo-random number generator once at startup. */
    srand((unsigned int)time(NULL));

    while (1) {
        if (button_pressed()) {
            /* Simple debounce. Replace with a robust debounce routine on hardware. */
            delay_ms(30);

            if (button_pressed()) {
                dice_value = roll_dice();
                display_dice(dice_value);
                buzzer_beep();

                /* Wait for release to avoid multiple rolls from one press. */
                while (button_pressed()) {
                    delay_ms(10);
                }
            }
        }
    }

    return 0;
}
