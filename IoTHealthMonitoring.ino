#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "MAX30105.h"

// WiFi & MQTT Configuration
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const char* mqtt_server = "broker.hivemq.com";

WiFiClient espClient;
PubSubClient client(espClient);

// SH110X Display Configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// Create SH110X display object (128x64). The reset pin is set to -1 if not used.
Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// DS18B20 Temperature Sensor Configuration
#define ONE_WIRE_BUS 4
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// MAX30102 Heart Rate Sensor Configuration
MAX30105 particleSensor;

void setup() {
  Serial.begin(115200);
  
  // 1. Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");

  // 2. Connect to MQTT Broker
  client.setServer(mqtt_server, 1883);
  reconnect();

  // 3. Initialize SH110X Display at I2C address 0x3C
  //    'true' enables internal charge pump
  if (!display.begin(0x3C, true)) {
    Serial.println("SH110X allocation failed!");
    while (1);
  }
  display.clearDisplay();
  display.setTextSize(1);
  // Use SH110X_WHITE instead of WHITE
  display.setTextColor(SH110X_WHITE);

  // 4. Initialize Temperature Sensor
  sensors.begin();

  // 5. Initialize MAX30102 Heart Rate Sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_STANDARD)) {
    Serial.println("MAX30102 not found");
    while (1);
  }
  particleSensor.setup();
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");
    if (client.connect("ESP32_HealthMonitor")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" retrying...");
      delay(2000);
    }
  }
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  // 1. Read Temperature
  sensors.requestTemperatures();
  float bodyTemp = sensors.getTempCByIndex(0);

  // 2. Read Heart Rate (Simulated)
  long irValue = particleSensor.getIR();
  int heartRate = (irValue > 50000) ? (irValue % 100 + 60) : 0;

  Serial.printf("Heart Rate: %d BPM, Temp: %.2f°C\n", heartRate, bodyTemp);

  // 3. Publish to MQTT
  char heartRateStr[10], tempStr[10];
  dtostrf(heartRate, 6, 2, heartRateStr);
  dtostrf(bodyTemp, 6, 2, tempStr);

  client.publish("/health/heart_rate", heartRateStr);
  client.publish("/health/temperature", tempStr);

  // 4. Display Data on SH110X OLED
  display.clearDisplay();
  display.setCursor(0, 0);
  // Use printf-style printing from Adafruit_GFX
  display.printf("Heart Rate: %d BPM\n", heartRate);
  display.printf("Temp: %.2f C\n", bodyTemp);
  display.display();

  delay(5000);  // Send data every 5 seconds
}
