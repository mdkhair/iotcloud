#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Servo.h>
#include <NewPingESP8266.h>

const char* ssid = "5G_CPE";
const char* password = "123456789";
const char* mqtt_server = "broker.hivemq.com";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;
bool statservo;
bool statcar;
Servo myservo;

#define TRIGGER_PIN D6
#define ECHO_PIN D5
#define MAX_DISTANCE 300  // Maximum distance we want to measure (in centimeters)
NewPingESP8266 sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

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
  if (topic == "Zafri1ABC")
  {
    if (fullpayload == "masuk" && statservo == false)
    {
      myservo.write(0);
      statservo = true;
    }
  }
  if (topic == "GateZafriPark1")
  {
    if (statcar == true && fullpayload == "masuk" && statservo == false)
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
      client.subscribe("Zafri1ABC");
      client.subscribe("GateZafriPark1");
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
  myservo.attach(D3, 500, 2400);
  myservo.write(90);
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
  delay(200);
  // Check distance to control the servo and publish messages
  if (statcar == false && statservo == true && distance < 6) {
    myservo.write(90); // Open the gate
    statservo = false;
    client.publish("GateZapPark1", "masuk");
    statcar = true;
  }

  if (statcar == true && statservo == true && distance > 15)
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
