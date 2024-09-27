#include <WiFi.h>
#include <PubSubClient.h>

const char *ssid = "Zip_Guest";
const char *pass = "Bondvagen46!";

const int ledPin = 18;

//MQTT BROKER CREDENTIALS
const char *mqtt_broker = "broker.emqx.io";
const int mqtt_port = 1883;
const char *topic = "emqx/esp32/led";
const char *mqtt_username = "emqx";
const char *mqtt_password = "123abc";

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);

  //sätter led pin till output
  pinMode(ledPin, OUTPUT);

  Serial.println("Starting LED control...");

  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("connecting to wifi...");
  }
  Serial.println("Connected to the Wi-fi network");


  //connecting to a mqtt broker
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);
  while (!client.connected()) {
    String client_id = "mqttx_cc626830";
    client_id += String(WiFi.macAddress());
    Serial.printf("The client %s is connecting to the MQTT broker\n", client_id.c_str());
    if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("EMQX MQTT broker connected");
    } else {
      Serial.print("Failed with state");
      Serial.print(client.state());
      delay(2000);
    }
  }
  client.publish(topic, "HI I'm esp32");
  client.subscribe(topic);
}

void callback(char *topic, byte *payload, unsigned int length) {
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  Serial.print("Message:");

  String message;
  for (int i = 0; i < length; i++) {
        message += (char) payload[i];
  }
  Serial.println(message);
  Serial.println("-----------------------");

  if (message.equalsIgnoreCase("on")) {
    digitalWrite(ledPin, HIGH);
    Serial.println("LED turned ON");
  } else if (message.equalsIgnoreCase("off")) {
    digitalWrite(ledPin, LOW);
    Serial.println("LED turned OFF");
  } else {
    Serial.println("Unknown command");
  }
}

void loop() {
  client.loop();
}
