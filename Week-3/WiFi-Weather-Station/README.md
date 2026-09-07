# Week 3 Mini Project — WiFi Weather Station

An ESP32-based IoT weather station that reads **temperature, humidity, and atmospheric pressure**, displays the measurements on an OLED, publishes telemetry over **MQTT**, and provides a browser dashboard for live charts.

## Architecture

```text
Temperature + Humidity Sensor ─┐
                               ├──> ESP32 ──> OLED Display
Pressure Sensor ───────────────┘       │
                                       │ WiFi
                                       ▼
                                  MQTT Broker
                                       │
                                       ▼
                               Web Dashboard
                                  Live Graphs
```

## Hardware

- ESP32 development board
- Temperature/humidity sensor (e.g. BME280 or DHT11/DHT22)
- Pressure sensor (BME280 is recommended because it combines temperature, humidity and pressure)
- 0.96-inch I2C OLED display (SSD1306)
- Breadboard and jumper wires
- USB power/programming cable

## Software Stack

- ESP32 Arduino framework / C++
- WiFi connectivity
- MQTT (PubSubClient or equivalent)
- I2C sensor/display drivers
- HTML/CSS/JavaScript web dashboard
- Chart.js for live graph rendering

## MQTT Topic

Default topic:

```text
weatherstation/telemetry
```

Example payload:

```json
{
  "temperature": 27.4,
  "humidity": 63.2,
  "pressure": 1008.6
}
```

## MQTT Broker Configuration

Before uploading the ESP32 sketch, configure:

```cpp
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
const char* MQTT_SERVER = "YOUR_MQTT_BROKER";
const int MQTT_PORT = 1883;
const char* MQTT_TOPIC = "weatherstation/telemetry";
```

Do **not** commit real WiFi passwords, broker credentials, API keys, or private certificates to GitHub.

## OLED Display

The OLED cycles/displays:

```text
WEATHER STATION
Temp: 27.4 C
Hum : 63.2 %
Pres: 1008.6 hPa
WiFi: OK
MQTT: OK
```

## Web Dashboard

`dashboard/index.html` is a self-contained browser dashboard. It connects to an MQTT broker through **MQTT over WebSockets** and plots incoming temperature, humidity, and pressure readings in real time.

Configure the broker WebSocket endpoint near the top of the script before use:

```javascript
const MQTT_WS_URL = "ws://YOUR_BROKER:8083/mqtt";
```

For production deployments, prefer a secure `wss://` endpoint and broker authentication.

## Repository Structure

```text
Week-3/
└── WiFi-Weather-Station/
    ├── README.md
    ├── esp32_weather_station.ino
    └── dashboard/
        └── index.html
```

## Learning Outcomes

- ESP32 WiFi networking
- Sensor interfacing over I2C
- OLED display programming
- MQTT publish/subscribe communication
- JSON telemetry formatting
- Browser-based IoT dashboards
- Real-time data visualization
- Basic IoT security practices

## Internship

**IoT & C Embedded Programming Internship — Week 3 Mini Project**
