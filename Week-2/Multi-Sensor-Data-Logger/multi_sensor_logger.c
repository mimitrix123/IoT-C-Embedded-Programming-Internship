/*
 * Week 2 Mini Project: Multi-Sensor Data Logger
 * IoT & C Embedded Programming Internship
 *
 * Sensors:
 *   - LM35 temperature sensor (ADC)
 *   - HC-SR04 ultrasonic distance sensor
 *   - LDR light sensor (ADC)
 *
 * Output:
 *   - Timestamped UART log
 *   - Threshold-based buzzer alert
 *
 * This file is portable C application logic. Hardware-specific GPIO,
 * ADC, UART, timer and delay functions should be implemented for the
 * target microcontroller.
 */

#include <stdio.h>
#include <stdint.h>

#define ADC_MAX_VALUE       1023.0f
#define ADC_REFERENCE_V     5.0f
#define LM35_MV_PER_DEG_C   10.0f

/* Example thresholds; tune these for the actual application. */
#define TEMP_HIGH_C         40.0f
#define DISTANCE_LOW_CM     10.0f
#define LIGHT_LOW_ADC       200

#define LOG_INTERVAL_MS     1000U

/* ---------- Hardware abstraction layer ---------- */

static void hardware_init(void)
{
    /* Initialize GPIO, ADC, UART, timer and buzzer hardware. */
}

static uint16_t adc_read(uint8_t channel)
{
    /* Replace with the MCU ADC implementation. */
    (void)channel;
    return 0;
}

static void hcsr04_trigger(void)
{
    /* Drive TRIG high for approximately 10 us, then low. */
}

static uint32_t hcsr04_echo_time_us(void)
{
    /* Measure the HIGH duration of the ECHO signal in microseconds. */
    return 0;
}

static void buzzer_set(uint8_t enabled)
{
    /* Drive the buzzer GPIO or PWM output. */
    (void)enabled;
}

static void delay_ms(uint32_t milliseconds)
{
    /* Replace with MCU-specific delay/timer code. */
    (void)milliseconds;
}

static void uart_write(const char *text)
{
    /* Replace with MCU UART transmit routine. */
    (void)text;
}

/* ---------- Sensor conversion functions ---------- */

static float lm35_read_celsius(void)
{
    const uint16_t raw = adc_read(0);
    const float voltage = ((float)raw / ADC_MAX_VALUE) * ADC_REFERENCE_V;
    const float millivolts = voltage * 1000.0f;

    /* LM35 output is approximately 10 mV per degree Celsius. */
    return millivolts / LM35_MV_PER_DEG_C;
}

static uint16_t ldr_read(void)
{
    return adc_read(1);
}

static float hcsr04_read_cm(void)
{
    const uint32_t echo_us_per_trigger = 58U;
    uint32_t echo_time_us;

    hcsr04_trigger();
    echo_time_us = hcsr04_echo_time_us();

    /* Approximation: distance(cm) = echo time(us) / 58. */
    return (float)echo_time_us / (float)echo_us_per_trigger;
}

/* ---------- Logging and alert logic ---------- */

static const char *get_alert(float temperature_c,
                             float distance_cm,
                             uint16_t light_adc)
{
    if (temperature_c > TEMP_HIGH_C) {
        return "TEMPERATURE";
    }

    if (distance_cm < DISTANCE_LOW_CM) {
        return "DISTANCE";
    }

    if (light_adc < LIGHT_LOW_ADC) {
        return "LIGHT";
    }

    return "NORMAL";
}

static void log_readings(uint32_t timestamp_s,
                         float temperature_c,
                         float distance_cm,
                         uint16_t light_adc,
                         const char *alert)
{
    char buffer[128];

    (void)snprintf(buffer, sizeof(buffer),
                   "%lu,%.2f,%.2f,%u,%s\r\n",
                   (unsigned long)timestamp_s,
                   temperature_c,
                   distance_cm,
                   (unsigned int)light_adc,
                   alert);

    uart_write(buffer);
}

static uint8_t alert_active(float temperature_c,
                            float distance_cm,
                            uint16_t light_adc)
{
    return (temperature_c > TEMP_HIGH_C) ||
           (distance_cm < DISTANCE_LOW_CM) ||
           (light_adc < LIGHT_LOW_ADC);
}

int main(void)
{
    uint32_t timestamp_s = 0;

    hardware_init();

    uart_write("Time(s),Temperature(C),Distance(cm),Light(ADC),Alert\r\n");

    while (1) {
        const float temperature_c = lm35_read_celsius();
        const float distance_cm = hcsr04_read_cm();
        const uint16_t light_adc = ldr_read();
        const char *alert = get_alert(temperature_c, distance_cm, light_adc);

        log_readings(timestamp_s,
                     temperature_c,
                     distance_cm,
                     light_adc,
                     alert);

        /* Beep when any configured threshold is exceeded. */
        if (alert_active(temperature_c, distance_cm, light_adc)) {
            buzzer_set(1);
            delay_ms(200);
            buzzer_set(0);
        }

        delay_ms(LOG_INTERVAL_MS);
        ++timestamp_s;
    }

    return 0;
}
