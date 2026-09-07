# PCB Design Plan

## Electrical Block Diagram

```text
                 +-------------------+
                 |      ESP32        |
                 | Wi-Fi / FreeRTOS  |
                 +---------+---------+
                           |
          +----------------+----------------+
          |                |                |
       I2C bus           ADC              GPIO
          |                |                |
      +---+---+      +-----+-----+      +---+---+
      | OLED  |      | Soil / LDR |      | Relay |
      +-------+      +-----------+      +---+---+
                                             |
                                          Pump OUT

DS18B20 -------------------------------> ESP32 GPIO4
```

## Netlist / Connector Plan

| Ref | Connection | Notes |
|---|---|---|
| J1 | 5 V / GND power input | Regulated low-voltage supply |
| J2 | Soil: 3V3/GND/ADC | Keep analog trace away from relay switching |
| J3 | LDR: 3V3/GND/ADC | Use resistor divider and test point |
| J4 | DS18B20: 3V3/GND/DATA | 4.7 kΩ pull-up on DATA |
| J5 | OLED: 3V3/GND/SDA/SCL | I2C, GPIO21/22 |
| J6 | Relay: VCC/GND/IN | Use a 3.3-V-compatible module/driver |
| J7 | Pump supply / switched output | Separate high-current path; voltage/current rating depends on pump |
| TP1 | 3V3 | Test point |
| TP2 | GND | Test point |
| TP3 | Soil ADC | Test point |
| TP4 | LDR ADC | Test point |
| TP5 | Relay control | Test point |

## Recommended Protection

- 100 nF ceramic decoupler near each sensor/interface IC.
- Bulk capacitor near the ESP32 supply (for example 10–47 µF, sized after regulator/transient testing).
- Fuse sized for the selected pump and wiring.
- Reverse-polarity protection on the low-voltage supply.
- Flyback diode if a bare DC pump is switched by a transistor/MOSFET. A relay coil module may already include its own suppression.
- If using a MOSFET driver, choose a logic-level MOSFET fully enhanced at the available gate voltage and size it for the pump's stall current.

## Layout Guidance

1. Put the ESP32 antenna at the board edge with a keep-out under and around the antenna area according to the module manufacturer's layout guidance.
2. Keep the relay/pump switching path physically separated from analog sensor traces.
3. Use wider copper for the pump-current path based on the expected current and PCB copper thickness.
4. Place sensor connectors on the perimeter for easy field wiring.
5. Add mounting holes aligned with the enclosure bosses.
6. Label every field connector with signal names and polarity.
7. Provide a programming/header access point for USB/UART or an ESP32 DevKit socket.
8. Provide test points for debugging before enclosure assembly.

## Manufacturing Checklist

- [ ] ERC/DRC passes.
- [ ] Footprints verified against physical components.
- [ ] ESP32 antenna keep-out checked.
- [ ] Connector polarity checked.
- [ ] Pump current and fuse rating verified.
- [ ] Relay contact rating verified for the selected pump.
- [ ] Sensor ADC ranges verified against ESP32 input limits.
- [ ] PCB mounting-hole dimensions matched to enclosure.
- [ ] Prototype tested before ordering a production quantity.

This document is a PCB implementation plan rather than a fabricated Gerber set. A final board should be captured in KiCad/EasyEDA after the exact ESP32 module, regulator, relay/driver, pump, connectors, and enclosure dimensions are selected.