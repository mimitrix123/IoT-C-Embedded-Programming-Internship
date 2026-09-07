# Week 2 Mini Project — Multi-Sensor Data Logger

A C embedded-systems project that reads **temperature, distance, and light level** and sends timestamped measurements to a serial terminal through UART. Thresholds trigger a buzzer alert when readings move outside configured limits.

## Sensors

- **Temperature:** LM35 (analog) or DHT11 (digital)
- **Distance:** HC-SR04 ultrasonic sensor
- **Light:** LDR connected to an ADC input
- **Output:** UART serial terminal
- **Alert:** Piezo buzzer

## Features

1. Periodically sample all three sensors.
2. Convert raw sensor measurements into useful units.
3. Generate a timestamp/counter for each sample.
4. Log readings over UART in a readable CSV-like format.
5. Compare measurements against configurable thresholds.
6. Produce a buzzer alert when a threshold is exceeded.

## Suggested UART Output

```text
Time(s),Temperature(C),Distance(cm),Light(ADC),Alert
0,27.4,48.2,731,NORMAL
1,27.5,47.9,745,NORMAL
2,27.6,8.3,752,DISTANCE
3,42.1,51.0,198,TEMPERATURE
```

## Suggested Thresholds

The values below are examples and should be adjusted for the hardware/environment:

| Measurement | Example alert condition |
|---|---|
| Temperature | > 40 °C |
| Distance | < 10 cm |
| Light | < 200 ADC counts (dark) |

## Hardware Connections

The exact GPIO/ADC/UART pins depend on the microcontroller board.

```text
LM35 / DHT11  ──> MCU temperature input
HC-SR04 TRIG  ──> MCU digital output
HC-SR04 ECHO  ──> MCU digital input
LDR           ──> MCU ADC input
Buzzer        ──> MCU digital/PWM output
UART TX       ──> USB-UART RX / serial terminal
GND           ──> Common ground
```

> **Important:** Check the HC-SR04 ECHO voltage level against the microcontroller's GPIO limits. Use appropriate level shifting when required.

## Implementation

`multi_sensor_logger.c` contains portable C application logic with hardware abstraction placeholders. Replace the sensor, GPIO, ADC, timer, UART, and delay functions with the APIs/register code for the selected microcontroller.

The implementation uses an **LM35-style analog temperature input** by default because it keeps the example focused on ADC, ultrasonic timing, UART logging, and threshold handling. The temperature function can be replaced by a DHT11 driver without changing the logger structure.

## Learning Outcomes

- Sensor interfacing in embedded C
- ADC-based sensor measurement
- Ultrasonic distance measurement using timing
- UART serial communication
- Timestamped data logging
- Threshold-based event/alert handling
- Hardware abstraction and modular embedded software

## Repository Structure

```text
Week-2/
└── Multi-Sensor-Data-Logger/
    ├── README.md
    └── multi_sensor_logger.c
```

## Internship

**IoT & C Embedded Programming Internship — Week 2 Mini Project**
