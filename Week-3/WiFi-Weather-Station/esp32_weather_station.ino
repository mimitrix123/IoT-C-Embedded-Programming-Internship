/*
 * Week 3 Mini Project: WiFi Weather Station
 * IoT & C Embedded Programming Internship
 *
 * ESP32 + BME280 + SSD1306 OLED + MQTT
 *
 * Libraries:
 *   - WiFi (ESP32 core)
 *   - Wire (ESP32 core)
 *   - Adafruit_BME280
 *   - Adafruit_GFX
 *   - Adafruit_SSD1306
 *   - PubSubClient
 *
 * Configure WiFi and MQTT values before uploading.
 */

#include <WiFi.h>
#include <Wire.h>
#include <PubSubClient.h>
#include <Adafruit_BME280.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_WIDTH       128
#define OLED_HEIGHT       64
#define OLED_RESET        -1
#define OLED_ADDRESS    0x3C
#define BME_ADDRESS     0x76

const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
const char* MQTT_SERVER = "YOUR_MQTT_BROKER";
const int MQTT_PORT = 1883;
const char* MQTT_TOPIC = "weatherstation/telemetry";

const unsigned long PUBLISH_INTERVAL_MS = 5000;

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
Adafruit_BME280 bme;
Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, OLED_RESET);

unsigned long lastPublish = 0;

void connectWiFi()
{
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }
}

void connectMQTT()
{
    while (!mqttClient.connected()) {
        String clientId = "ESP32-Weather-" + String((uint32_t)ESP.getEfuseMac(), HEX);

        if (mqttClient.connect(clientId.c_str())) {
            // Connected. Publish a status message if desired.
            mqttClient.publish("weatherstation/status", "online", true);
        } else {
            delay(2000);
        }
    }
}

void updateOLED(float temperature, float humidity, float pressure)
{
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);

    display.println("WEATHER STATION");
    display.println();
    display.print("Temp: ");
    display.print(temperature, 1);
    display.println(" C");

    display.print("Hum : ");
    display.print(humidity, 1);
    display.println(" %");

    display.print("Pres: ");
    display.print(pressure, 1);
    display.println(" hPa");

    display.print("WiFi: ");
    display.println(WiFi.status() == WL_CONNECTED ? "OK" : "OFF");

    display.print("MQTT: ");
    display.println(mqttClient.connected() ? "OK" : "OFF");

    display.display();
}

void publishWeather(float temperature, float humidity, float pressure)
{
    char payload[160];

    snprintf(payload, sizeof(payload),
             "{\"temperature\":%.2f,\"humidity\":%.2f,\"pressure\":%.2f}",
             temperature, humidity, pressure);

    mqttClient.publish(MQTT_TOPIC, payload);
}

void setup()
{
    Serial.begin(115200);
    Wire.begin();

    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
        while (true) {
            delay(1000);
        }
    }

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Starting weather station...");
    display.display();

    if (!bme.begin(BME_ADDRESS)) {
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("BME280 not found!");
        display.display();

        while (true) {
            delay(1000);
        }
    }

    mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
    connectWiFi();
    connectMQTT();
}

void loop()
{
    if (WiFi.status() != WL_CONNECTED) {
        connectWiFi();
    }

    if (!mqttClient.connected()) {
        connectMQTT();
    }

    mqttClient.loop();

    unsigned long now = millis();
    if (now - lastPublish >= PUBLISH_INTERVAL_MS) {
        lastPublish = now;

        float temperature = bme.readTemperature();
        float humidity = bme.readHumidity();
        float pressure = bme.readPressure() / 100.0F;

        updateOLED(temperature, humidity, pressure);
        publishWeather(temperature, humidity, pressure);

        Serial.printf("Temperature: %.2f C | Humidity: %.2f %% | Pressure: %.2f hPa\n",
                      temperature, humidity, pressure);
    }
}
