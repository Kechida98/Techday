#include <WiFi.h>
const char *ssid = "";
const char *pass = "";

const int ledPin = 18;

void setup() {
  Serial.begin(115200);

  //sätter led pin till output
  pinMode(ledPin, OUTPUT);
  
  Serial.println("Starting LED control...");

  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED){
    delay (1000);
    Serial.println("connecting to wifi...");
  }
  Serial.println("connected to wifi");
}

void loop() {
   // Turn the LED ON
  digitalWrite(ledPin, HIGH);
  Serial.println("LED is ON");
  
  // Wait for 1 second (1000 milliseconds)
  delay(1000);

  // Turn the LED OFF
  digitalWrite(ledPin, LOW);
  Serial.println("LED is OFF");

  // Wait for 1 second
  delay(1000);

}
//test push
