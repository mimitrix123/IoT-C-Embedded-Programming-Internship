# Week 4 — IoT Smart Agriculture System

A complete ESP32-based smart agriculture prototype for monitoring soil moisture, temperature, and light, automatically controlling irrigation, displaying local status, reporting telemetry over MQTT, and supporting FreeRTOS multitasking, deep sleep, and OTA firmware updates.

## Features

- Soil-moisture monitoring using an analog capacitive sensor
- Temperature monitoring using a DS18B20
- Ambient-light monitoring using an LDR
- Automatic irrigation through a relay-controlled DC water pump
- OLED status display (SSD1306, I2C)
- Wi-Fi connectivity
- MQTT telemetry and irrigation-state reporting
- FreeRTOS tasks for sensing, display, MQTT, and irrigation logic
- Deep-sleep support for battery-oriented deployments
- OTA firmware update support
- PCB design reference files and wiring plan
- 3D-printed enclosure plan with ventilation, cable glands, and mounting points

## Hardware

| Part | Purpose |
|---|---|
| ESP32 DevKit | Main controller, Wi-Fi, FreeRTOS, OTA, deep sleep |
| Capacitive soil-moisture sensor | Soil moisture measurement |
| DS18B20 | Temperature measurement |
| LDR + resistor | Ambient light measurement |
| 0.96-inch SSD1306 OLED | Local status display |
| 1-channel 3.3-V compatible relay module | Pump switching |
| DC water pump | Irrigation actuator |
| External pump supply | Powers pump; do not power pump from ESP32 3.3 V |
| Flyback protection | Required when driving a bare DC pump with a transistor/MOSFET |

> **Safety:** Keep the pump power path separate from the ESP32 supply. Use a properly rated relay/MOSFET, fuse, flyback protection where applicable, common ground where required by the driver design, and suitable waterproofing. Do not switch mains voltage with this prototype.

## Suggested GPIO Map

| Function | ESP32 GPIO | Notes |
|---|---:|---|
| Soil moisture ADC | GPIO34 | Input-only ADC pin |
| LDR ADC | GPIO35 | Input-only ADC pin |
| DS18B20 data | GPIO4 | Add 4.7 kΩ pull-up to 3.3 V |
| OLED SDA | GPIO21 | I2C |
| OLED SCL | GPIO22 | I2C |
| Pump relay | GPIO26 | Active-high by default; verify module logic |
| Battery/wake status | GPIO33 | Optional battery-divider ADC / wake input |

Change the pin definitions in `smart_agriculture.ino` to match the final PCB.

## Irrigation Logic

The firmware uses hysteresis to avoid rapid relay cycling:

- Pump starts when soil moisture is at or below `SOIL_DRY_THRESHOLD`.
- Pump stops when soil moisture reaches `SOIL_WET_THRESHOLD`.
- Pump is limited by `MAX_PUMP_RUNTIME_MS` as a safety timeout.
- A cooldown period prevents immediate repeated activation.
- MQTT reports both sensor telemetry and irrigation state.

The exact moisture thresholds must be calibrated for the actual soil, sensor, crop, and installation depth.

## MQTT

Default topics:

- `smart-agriculture/telemetry`
- `smart-agriculture/irrigation`
- `smart-agriculture/status`

Example telemetry:

```json
{
  "soil_moisture": 38.4,
  "temperature": 28.1,
  "light": 742,
  "pump": 1
}
```

Set Wi-Fi and MQTT configuration in the firmware or a secure provisioning mechanism. **Never commit real passwords, broker credentials, private keys, or certificates.**

## FreeRTOS Architecture

The ESP32 Arduino environment provides FreeRTOS. The implementation separates responsibilities into tasks:

1. **Sensor task** — periodically samples sensors and updates shared state.
2. **Irrigation task** — evaluates moisture thresholds and drives the relay.
3. **Display task** — refreshes the OLED with the latest state.
4. **MQTT task** — maintains the connection and publishes telemetry.

A mutex protects shared sensor/state data. Delays use FreeRTOS task delays so the CPU can service other tasks.

## Deep Sleep Strategy

Deep sleep is intended for a battery deployment where continuous Wi-Fi/MQTT connectivity is not required. The controller can:

1. Wake on a timer.
2. Sample sensors and update local state.
3. Run irrigation logic if required.
4. Publish a telemetry packet when Wi-Fi is available.
5. Disconnect Wi-Fi and enter deep sleep.

For a real pump system, do not enter sleep while irrigation is active. Use an appropriate external latch/controller if the pump must remain on while the ESP32 sleeps.

The default firmware keeps continuous FreeRTOS operation for demonstration. Set `ENABLE_DEEP_SLEEP` to `true` for the battery workflow and validate the power budget first.

## OTA Updates

OTA uses the ESP32 Arduino OTA service. During development, the board can be updated over the local network after its first USB flash. OTA should be protected with a password and/or a controlled network in real deployments.

## PCB Design

`pcb/README.md` contains the schematic-level netlist, connector plan, routing guidance, and manufacturing checklist. The PCB should provide:

- ESP32 module/header footprint
- Sensor connectors with labeled power/data pins
- OLED I2C connector
- Relay/driver output connector
- Separate pump power input and protected switching path
- Decoupling capacitors near the ESP32 and sensor supply
- Test points for 3.3 V, GND, ADC inputs, and relay control
- Mounting holes
- Clear separation between low-voltage logic and pump power wiring

For the first hardware revision, validate the circuit on a breadboard before fabrication.

## 3D-Printed Enclosure

`enclosure/README.md` contains the mechanical plan. The enclosure should include:

- ESP32/PCB mounting bosses
- Front OLED window
- Cable-gland openings for sensor and pump wiring
- Ventilation slots around the ESP32
- Drainage path / drip protection
- Removable lid with four screws
- Separation between electronics and any wet tubing
- Optional transparent or gasketed sensor/display cover

Print in a durable material suitable for the expected outdoor temperature and humidity. The enclosure is not automatically weatherproof; sealing and cable glands must be validated independently.

## Firmware Setup

Install these Arduino libraries:

- OneWire
- DallasTemperature
- Adafruit GFX Library
- Adafruit SSD1306
- PubSubClient

Then:

1. Open `esp32/smart_agriculture.ino` in Arduino IDE or PlatformIO.
2. Select the ESP32 board.
3. Enter local Wi-Fi and MQTT settings.
4. Flash once over USB.
5. Calibrate the soil sensor's dry/wet readings.
6. Verify the pump driver with the pump disconnected.
7. Test MQTT and OTA on a trusted network.
8. Enable deep sleep only after validating wake/publish/irrigation behavior.

## Repository Layout

```text
Week-4/
└── Smart-Agriculture-System/
    ├── README.md
    ├── esp32/
    │   └── smart_agriculture.ino
    ├── pcb/
    │   └── README.md
    ├── enclosure/
    │   └── README.md
    └── docs/
        └── system-design.md
```

## Learning Outcomes

- ESP32 embedded IoT development
- ADC and digital sensor interfacing
- Relay/actuator control
- FreeRTOS task-based firmware architecture
- Wi-Fi and MQTT telemetry
- OLED user interfaces
- OTA firmware deployment
- Deep-sleep power management
- PCB/system hardware planning
- Enclosure and environmental design
- Safe separation of logic and actuator power
