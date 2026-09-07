# System Design and Test Plan

## 1. Architecture

```text
Soil Sensor ----\
DS18B20 ---------+--> ESP32 --> OLED
LDR ------------/      |
                       +--> Relay --> DC Pump
                       |
                       +--> Wi-Fi --> MQTT Broker --> Cloud Dashboard
                       |
                       +--> OTA service
                       |
                       +--> Deep Sleep (battery mode)
```

The ESP32 is the edge controller. Sensor acquisition, irrigation decisions, display refresh, MQTT networking, and OTA handling are split into FreeRTOS tasks. A mutex protects shared state.

## 2. Operating Modes

### Continuous / development mode

- Sensors sampled every 5 seconds.
- OLED refreshed every second.
- MQTT telemetry published every 10 seconds.
- OTA remains available.
- Irrigation uses hysteresis and a maximum runtime.

### Battery / deep-sleep mode

- Wake on a timer.
- Read sensors.
- Apply irrigation policy.
- Publish telemetry if network is available.
- Ensure pump is OFF before sleep.
- Turn off Wi-Fi and enter timer deep sleep.

Deep sleep trades continuous OTA availability for lower power consumption. OTA should be performed during a maintenance wake window.

## 3. Irrigation State Machine

```text
          soil <= DRY
      +----------------+
      |                v
    [IDLE] --------> [PUMP ON]
      ^                 |
      |                 | soil >= WET
      |                 | OR max runtime
      +-----------------+
             cooldown
```

The dry and wet thresholds must be calibrated experimentally. The cooldown prevents excessive relay cycling.

## 4. Fault Handling

- Wi-Fi loss: reconnect without stopping local sensing/irrigation logic.
- MQTT loss: reconnect and continue local operation.
- Invalid DS18B20 reading: keep the value visible as invalid and avoid treating it as a valid control input.
- Pump runtime timeout: force pump OFF.
- OTA start: force pump OFF before updating.
- Mutex failure: skip the affected shared-state operation rather than corrupting state.
- Power loss: relay defaults to OFF at boot.

For a production system, add watchdog supervision, persistent fault/event logging, sensor-disconnect detection, brownout handling, and a hardware-level pump fail-safe.

## 5. MQTT Contract

### Telemetry

Topic: `smart-agriculture/telemetry`

Payload:

```json
{
  "soil_moisture": 38.4,
  "temperature": 28.1,
  "light": 742,
  "pump": 0
}
```

### Irrigation state

Topic: `smart-agriculture/irrigation`

Payload: `ON` or `OFF`

### Device status

Topic: `smart-agriculture/status`

Payload: `online` / `offline` using MQTT last-will behavior.

## 6. Security Checklist

- Do not commit Wi-Fi or MQTT secrets.
- Use MQTT over TLS in production.
- Use broker authentication and least-privilege credentials.
- Protect OTA with a strong password and preferably signed/controlled firmware deployment.
- Keep the broker off the public internet unless appropriately secured.
- Rotate credentials if they are accidentally exposed.

## 7. Validation Procedure

### Sensor validation

1. Record raw soil ADC with the probe dry.
2. Record raw ADC in thoroughly wet soil.
3. Update `SOIL_DRY_ADC` and `SOIL_WET_ADC`.
4. Compare reported moisture against several known soil conditions.
5. Verify DS18B20 temperature against a reference thermometer.
6. Observe LDR readings in darkness, indoor light, and direct bright light.

### Pump validation

1. Test relay output with the pump disconnected.
2. Verify default boot state is OFF.
3. Connect the pump through the correctly rated driver/supply.
4. Verify dry threshold starts the pump.
5. Verify wet threshold stops the pump.
6. Verify maximum runtime forces a stop.
7. Verify cooldown prevents rapid restart.

### Network validation

1. Confirm Wi-Fi connection and IP address.
2. Confirm MQTT telemetry arrives at the broker.
3. Disconnect Wi-Fi and verify local control continues.
4. Restore Wi-Fi and verify reconnection.
5. Perform OTA from a trusted development network.

### Power validation

Measure current in:

- boot
- Wi-Fi connected idle
- MQTT publishing
- pump active
- deep sleep

Use these measurements to size the battery, regulator, and solar/charging system rather than relying on nominal ESP32 figures.

## 8. Acceptance Criteria

- Soil, temperature, and light values update reliably.
- OLED shows current values and connectivity state.
- Pump switches only according to calibrated thresholds and safety timeout.
- MQTT receives valid JSON telemetry.
- Device recovers from Wi-Fi/MQTT interruption.
- OTA update completes without leaving the pump active.
- Deep-sleep wake/publish cycle works when enabled.
- PCB and enclosure dimensions match the selected hardware before fabrication.
