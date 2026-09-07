# Week 1 Mini Project — Digital Dice

A C-based embedded systems mini project that simulates a physical dice using **7 LEDs arranged in a standard dice pattern**, a push button, and a piezo buzzer.

## Objective

Press the push button to roll the dice. The program generates a random value from **1 to 6** and displays the corresponding dice face using seven LEDs. A piezo buzzer provides audio feedback for each roll.

## Components

- 7 × LEDs
- 7 × current-limiting resistors (typically 220Ω–330Ω)
- 1 × push button
- 1 × piezo buzzer
- Microcontroller/development board
- Breadboard and jumper wires
- Suitable power supply

## LED Arrangement

```text
LED positions:

A       B

C   D   E

F       G
```

The LED patterns represent the six dice faces:

```text
1: D
2: A + G
3: A + D + G
4: A + B + F + G
5: A + B + D + F + G
6: A + B + C + E + F + G
```

## Program Logic

1. Initialize the seven LED outputs, button input, and buzzer.
2. Wait for the push button to be pressed.
3. Seed/use the C random-number generator.
4. Generate a number from 1–6 using `rand() % 6 + 1`.
5. Turn on the LEDs corresponding to the generated dice value.
6. Activate the buzzer briefly as roll feedback.
7. Wait for the button to be released before accepting another roll.

## C Implementation

The `digital_dice.c` file contains a portable C implementation of the dice-face logic. The hardware-specific GPIO, delay, and buzzer functions can be mapped to the target microcontroller board.

## Learning Outcomes

- Digital GPIO input/output
- LED control and bit/pattern mapping
- Push-button handling and debouncing
- C random-number generation
- Functions and arrays in embedded C
- Basic hardware/software integration

## Repository Structure

```text
Week-1/
└── Digital-Dice/
    ├── README.md
    └── digital_dice.c
```

## Internship

**IoT & C Embedded Programming Internship — Week 1 Mini Project**
