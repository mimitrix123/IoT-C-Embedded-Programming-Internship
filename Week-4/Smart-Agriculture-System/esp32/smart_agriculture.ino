#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoOTA.h>
#include "esp_sleep.h"

// ========================= Configuration =========================
#define ENABLE_DEEP_SLEEP false

static const char *WIFI_SSID = "YOUR_WIFI_SSID";
static const char *WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
static const char *MQTT_SERVER = "YOUR_MQTT_BROKER";
static const uint16_t MQTT_PORT = 1883;
static const char *MQTT_USER = "";
static const char *MQTT_PASSWORD = "";
static const char *MQTT_CLIENT_PREFIX = "smart-agri-";

static const char *TOPIC_TELEMETRY = "smart-agriculture/telemetry";
static const char *TOPIC_IRRIGATION = "smart-agriculture/irrigation";
static const char *TOPIC_STATUS = "smart-agriculture/status";

// ============================= Pins ===============================
static const int SOIL_PIN = 34;
static const int LDR_PIN = 35;
static const int DS18B20_PIN = 4;
static const int RELAY_PIN = 26;
static const int OLED_SDA = 21;
static const int OLED_SCL = 22;

static const uint8_t OLED_ADDRESS = 0x3C;
static const int SCREEN_WIDTH = 128;
static const int SCREEN_HEIGHT = 64;

// Calibrate these values with the installed soil sensor.
static const int SOIL_DRY_ADC = 3000;
static const int SOIL_WET_ADC = 1200;
static const float SOIL_DRY_THRESHOLD = 30.0f;
static const float SOIL_WET_THRESHOLD = 55.0f;

static const uint32_t SENSOR_PERIOD_MS = 5000;
static const uint32_t DISPLAY_PERIOD_MS = 1000;
static const uint32_t MQTT_PERIOD_MS = 10000;
static const uint32_t MAX_PUMP_RUNTIME_MS = 30000;
static const uint32_t PUMP_COOLDOWN_MS = 60000;
static const uint64_t DEEP_SLEEP_SECONDS = 300;

// Most relay modules are active HIGH; change to LOW if required.
static const int RELAY_ON = HIGH;
static const int RELAY_OFF = LOW;

// ============================ Objects =============================
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
OneWire oneWire(DS18B20_PIN);
DallasTemperature tempSensor(&oneWire);

// =========================== Shared State =========================
struct SensorState {
  float soilPercent;
  float temperatureC;
  int lightAdc;
  bool pumpOn;
  uint32_t pumpStartedAt;
  uint32_t lastPumpStoppedAt;
  uint32_t sampleNumber;
};

SensorState state = {0.0f, NAN, 0, false, 0, 0, 0};
SemaphoreHandle_t stateMutex;

// ============================= Helpers ============================
float soilPercentFromAdc(int raw) {
  float pct = 100.0f * (float)(SOIL_DRY_ADC - raw) /
              (float)(SOIL_DRY_ADC - SOIL_WET_ADC);
  return constrain(pct, 0.0f, 100.0f);
}

bool copyState(SensorState &out) {
  if (xSemaphoreTake(stateMutex, pdMS_TO_TICKS(100)) != pdTRUE) return false;
  out = state;
  xSemaphoreGive(stateMutex);
  return true;
}

void setPump(bool on) {
  if (xSemaphoreTake(stateMutex, pdMS_TO_TICKS(100)) != pdTRUE) return;
  if (state.pumpOn == on) {
    xSemaphoreGive(stateMutex);
    return;
  }

  digitalWrite(RELAY_PIN, on ? RELAY_ON : RELAY_OFF);
  state.pumpOn = on;
  if (on) {
    state.pumpStartedAt = millis();
  } else {
    state.lastPumpStoppedAt = millis();
  }
  xSemaphoreGive(stateMutex);

  Serial.printf("Pump %s\n", on ? "ON" : "OFF");
}

void connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");

  uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 20000) {
    delay(250);
    Serial.print('.');
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Wi-Fi IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("Wi-Fi connection timed out");
  }
}

void connectMQTT() {
  if (WiFi.status() != WL_CONNECTED || mqttClient.connected()) return;

  String clientId = String(MQTT_CLIENT_PREFIX) + String((uint32_t)(ESP.getEfuseMac() & 0xFFFFFFFF), HEX);
  Serial.print("Connecting MQTT...");

  bool connected;
  if (strlen(MQTT_USER) > 0) {
    connected = mqttClient.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD,
                                   TOPIC_STATUS, 0, true, "offline");
  } else {
    connected = mqttClient.connect(clientId.c_str(), TOPIC_STATUS, 0, true, "offline");
  }

  if (connected) {
    Serial.println("connected");
    mqttClient.publish(TOPIC_STATUS, "online", true);
  } else {
    Serial.printf("failed, rc=%d\n", mqttClient.state());
  }
}

void readSensors() {
  const int soilRaw = analogRead(SOIL_PIN);
  const int lightRaw = analogRead(LDR_PIN);

  tempSensor.requestTemperatures();
  const float tempC = tempSensor.getTempCByIndex(0);

  if (xSemaphoreTake(stateMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
    state.soilPercent = soilPercentFromAdc(soilRaw);
    state.temperatureC = tempC;
    state.lightAdc = lightRaw;
    state.sampleNumber++;
    xSemaphoreGive(stateMutex);
  }

  Serial.printf("Sensors: soil=%.1f%% temp=%.2fC light=%d\n",
                soilPercentFromAdc(soilRaw), tempC, lightRaw);
}

void publishTelemetry() {
  if (!mqttClient.connected()) return;

  SensorState s;
  if (!copyState(s)) return;

  char payload[256];
  const int written = snprintf(payload, sizeof(payload),
    "{\"soil_moisture\":%.1f,\"temperature\":%.2f,\"light\":%d,\"pump\":%d}",
    s.soilPercent, s.temperatureC, s.lightAdc, s.pumpOn ? 1 : 0);

  if (written > 0 && written < (int)sizeof(payload)) {
    mqttClient.publish(TOPIC_TELEMETRY, payload, false);
  }
}

void publishPumpState(bool on) {
  if (mqttClient.connected()) {
    mqttClient.publish(TOPIC_IRRIGATION, on ? "ON" : "OFF", true);
  }
}

// ============================ FreeRTOS ============================
void sensorTask(void *parameter) {
  (void)parameter;
  for (;;) {
    readSensors();
    vTaskDelay(pdMS_TO_TICKS(SENSOR_PERIOD_MS));
  }
}

void irrigationTask(void *parameter) {
  (void)parameter;
  bool lastPumpState = false;

  for (;;) {
    SensorState s;
    if (copyState(s)) {
      const uint32_t now = millis();
      bool requested = s.pumpOn;

      if (s.pumpOn) {
        // Hard safety timeout prevents a stuck relay/pump from running forever.
        if (now - s.pumpStartedAt >= MAX_PUMP_RUNTIME_MS ||
            s.soilPercent >= SOIL_WET_THRESHOLD) {
          requested = false;
        }
      } else {
        const bool cooldownComplete = (now - s.lastPumpStoppedAt >= PUMP_COOLDOWN_MS) ||
                                      (s.lastPumpStoppedAt == 0);
        if (s.soilPercent <= SOIL_DRY_THRESHOLD && cooldownComplete) {
          requested = true;
        }
      }

      if (requested != s.pumpOn) {
        setPump(requested);
      }

      if (requested != lastPumpState) {
        publishPumpState(requested);
        lastPumpState = requested;
      }
    }
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

void displayTask(void *parameter) {
  (void)parameter;
  for (;;) {
    SensorState s;
    if (copyState(s)) {
      display.clearDisplay();
      display.setTextColor(SSD1306_WHITE);
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.println("SMART AGRICULTURE");
      display.printf("Soil : %5.1f %%\n", s.soilPercent);
      display.printf("Temp : %5.1f C\n", s.temperatureC);
      display.printf("Light: %5d\n", s.lightAdc);
      display.printf("Pump : %s\n", s.pumpOn ? "ON" : "OFF");
      display.printf("WiFi : %s\n", WiFi.status() == WL_CONNECTED ? "OK" : "--");
      display.printf("MQTT : %s\n", mqttClient.connected() ? "OK" : "--");
      display.display();
    }
    vTaskDelay(pdMS_TO_TICKS(DISPLAY_PERIOD_MS));
  }
}

void mqttTask(void *parameter) {
  (void)parameter;
  uint32_t lastPublish = 0;

  for (;;) {
    if (WiFi.status() != WL_CONNECTED) connectWiFi();
    if (!mqttClient.connected()) connectMQTT();
    mqttClient.loop();

    if (millis() - lastPublish >= MQTT_PERIOD_MS) {
      publishTelemetry();
      lastPublish = millis();
    }

    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

void otaTask(void *parameter) {
  (void)parameter;
  for (;;) {
    ArduinoOTA.handle();
    vTaskDelay(pdMS_TO_TICKS(20));
  }
}

void enterDeepSleepIfEnabled() {
  if (!ENABLE_DEEP_SLEEP) return;

  SensorState s;
  if (!copyState(s)) return;
  if (s.pumpOn) return; // Never sleep while the pump is active.

  if (mqttClient.connected()) {
    publishTelemetry();
    mqttClient.loop();
    delay(250);
    mqttClient.disconnect();
  }
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);

  Serial.printf("Deep sleep for %llu seconds\n", DEEP_SLEEP_SECONDS);
  esp_sleep_enable_timer_wakeup(DEEP_SLEEP_SECONDS * 1000000ULL);
  esp_deep_sleep_start();
}

// ============================== Setup =============================
void setup() {
  Serial.begin(115200);
  delay(300);

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, RELAY_OFF);
  analogReadResolution(12);

  stateMutex = xSemaphoreCreateMutex();
  if (stateMutex == nullptr) {
    Serial.println("ERROR: state mutex allocation failed");
    while (true) delay(1000);
  }

  Wire.begin(OLED_SDA, OLED_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    Serial.println("OLED initialization failed");
  } else {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("Smart Agriculture");
    display.println("Starting...");
    display.display();
  }

  tempSensor.begin();
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  mqttClient.setBufferSize(512);

  connectWiFi();
  connectMQTT();

  ArduinoOTA.setHostname("smart-agriculture-esp32");
  ArduinoOTA.setPassword("CHANGE_OTA_PASSWORD");
  ArduinoOTA.onStart([]() {
    Serial.println("OTA update started");
    setPump(false);
  });
  ArduinoOTA.onEnd([]() { Serial.println("OTA update complete"); });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("OTA error: %u\n", error);
  });
  ArduinoOTA.begin();

  // Pin tasks to cores to make responsibilities explicit. Arduino's
  // networking stack continues to run alongside these application tasks.
  xTaskCreatePinnedToCore(sensorTask, "SensorTask", 4096, nullptr, 2, nullptr, 1);
  xTaskCreatePinnedToCore(irrigationTask, "IrrigationTask", 4096, nullptr, 3, nullptr, 1);
  xTaskCreatePinnedToCore(displayTask, "DisplayTask", 4096, nullptr, 1, nullptr, 0);
  xTaskCreatePinnedToCore(mqttTask, "MQTTTask", 6144, nullptr, 2, nullptr, 0);
  xTaskCreatePinnedToCore(otaTask, "OTATask", 4096, nullptr, 1, nullptr, 0);

  // In a battery build, call this only after the required sample/publish cycle.
  // It is intentionally disabled by default because continuous OTA/MQTT is not
  // compatible with long uninterrupted deep-sleep periods.
}

void loop() {
  // Main loop remains lightweight; application work is handled by FreeRTOS tasks.
  static uint32_t lastSleepCheck = 0;
  if (ENABLE_DEEP_SLEEP && millis() - lastSleepCheck > 60000) {
    lastSleepCheck = millis();
    enterDeepSleepIfEnabled();
  }
  delay(10);
}
