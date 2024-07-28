#include <WiFi.h>
#include <PubSubClient.h>
#include <ESP32Servo.h>
#include <NewPing.h>

const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* mqtt_server = "broker.hivemq.com";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];
int value = 0;
bool statservo;
bool statcar;
Servo myservo;

#define TRIGGER_PIN 22
#define ECHO_PIN 23
#define MAX_DISTANCE 300  // Maximum distance we want to measure (in centimeters)
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(String topic, byte* payload, unsigned int length) {
  String fullpayload;
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    fullpayload += (char)payload[i];
  }
  Serial.println();
  if (topic == "servo1khair")
  {
    if (fullpayload == "open" && statservo == false)
    {
      myservo.write(0);
      statservo = true;
    }
  }
  if (topic == "GatePark1")
  {
    if (statcar == true && fullpayload == "open" && statservo == false)
    {
      myservo.write(0);
      statservo = true;
    }
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      client.subscribe("servo1khair");
      client.subscribe("GatePark1");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  myservo.attach(19);
  myservo.write(90);
  pinMode(18, INPUT_PULLUP);
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  unsigned int distance = sonar.ping_cm();

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  // Check distance to control the servo and publish messages
  if (statcar == false && statservo == true && distance < 50) {
    myservo.write(90); // Open the gate
    statservo = false;
    client.publish("GateParking1", "open");
    statcar = true;
  }

  if (statcar == true && statservo == true && distance > 200)
  {
    myservo.write(90);
    statservo = false;
    statcar = false;
  }


  if (!client.connected()) {
    reconnect();
  }
  client.loop();


  unsigned long now = millis();
}
