#include "HX711.h"// Loadcell libary
#include "soc/rtc.h"
#include <WiFi.h>
#include <PubSubClient.h>

// Connection settings (Wi-Fi and MQTT)
const char *ssid = "TN-WH2983";
const char *pass = "SyufjufDeub5";
const char *mqtt_broker = "broker.emqx.io";
const int mqtt_port = 1883;
const char *mqtt_topic_weight = "esp32/scale";
const char *mqtt_topic_distance = "esp32/distance";
const char *mqtt_username = "emqx";
const char *mqtt_password = "123abc";

WiFiClient espClient;
PubSubClient client(espClient);

// Load Cell HX711 Configurations
const int LOADCELL_DOUT_PIN = 4;
const int LOADCELL_SCK_PIN = 19;
HX711 scale;
float totalWeight=0;// In the bin
float previousWeight = 0;
float threshold = 1.0;
float calibration = 1100.38987;

// Ultrasonic Sensor Configurations
const int trigPin = 18;
const int echoPin = 5;
#define SOUND_SPEED 0.034
long duration;
float distanceCm;
float previousDistance = 0;
float distanceThreshold = 1.0;

// Timing variables for Measurement Intervals
unsigned long previousMillisScale = 0;
unsigned long previousMillisUltra = 0;
const long ultrasonicInterval = 1000;
const long scaleInterval = 10000;
const long powerDownInterval = 30000;

// Setup Function
void setup() {
  Serial.begin(115200);
  initializeScale();// Initialize scale with debug prints

  connectToWIFI();  
  connectToMQTT();

  // Initialize ultrasonic sensor pins
  pinMode(trigPin,OUTPUT);
  pinMode(echoPin,INPUT);
}

// Main Loop
void loop() {
  //getCalibration();
  unsigned long currentMillis = millis();

  // Handle distance measurement and publishing
  if(currentMillis - previousMillisUltra >= ultrasonicInterval){
    previousMillisUltra = currentMillis;
    handleDistanceMeasurement();
  }

  // Handle weight measurement and publishing
  if(currentMillis - previousMillisScale >= scaleInterval){
    previousMillisScale = currentMillis;
    handleWeightMeasurement();
  }

  //Loop for connection
  client.loop();
}

// WiFi Conection Function
void connectToWIFI(){
  WiFi.begin(ssid,pass);
  while (WiFi.status() !=WL_CONNECTED){
    delay(1000);
    Serial.println("connecting to wifi...");
  }
   Serial.println("Connected to the Wi-fi network");
  
}

//MQTT Connection Function
void connectToMQTT(){
   client.setServer(mqtt_broker,mqtt_port);
   while(!client.connected()){
    String client_id = "";
    client_id += String(WiFi.macAddress());
    Serial.printf("The client %s is connecting to the MQTT broker\n",client_id.c_str());
    if(client.connect(client_id.c_str(),mqtt_username,mqtt_password)){
      Serial.println("Connected to the EMQX MQTT broker.");
    }else{
      Serial.print("Failed to connect, state: ");
      Serial.print(client.state());
      delay(2000);
   }
  }
 }

// Ultrasonic Sensor Measurment and Publishing function
void handleDistanceMeasurement(){
    // Send a trigger pulse to the ultrasonic sensor
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    duration = pulseIn(echoPin,HIGH,20000);

    // Debugging: printing the raw duration to check if any signal is detected
    Serial.print("Echo Duration (us): ");
    Serial.println(duration);
    
    if (duration > 0) { // Only caluculate distance if duration is not zero or less
        distanceCm = duration * SOUND_SPEED / 2;
        
        if (abs(distanceCm - previousDistance) > distanceThreshold) {
            Serial.print("Distance change detected. Distance (cm): ");
            Serial.println(distanceCm, 1);
            
            // Convert distance to string and publish
            client.publish(mqtt_topic_distance, String(distanceCm,1).c_str());
            previousDistance = distanceCm;
        }
    } else {
        Serial.println("No echo detected (timeout).");
    }
}

// Load Cell (HX711) Measurement and publishing function
void handleWeightMeasurement(){
  static int stableCount = 0;
  static float stableWeight = 0;

  if(scale.is_ready()){
    // Get the current weight reading
    float currentWeight = scale.get_units(5);  // Average of 5 readings for stability

    if(currentWeight < 0) {
    currentWeight = 0;
  } else if (currentWeight > 1000) {
    currentWeight = 1000;
  }

  // Stability check for consistent readings
  if (abs(currentWeight - stableWeight) < threshold){
    stableCount++;
  } else{
    stableCount = 0;
    stableWeight = currentWeight;
  }

  // Process weight change if stable for 2 readings
  if (stableCount >= 2 && abs(currentWeight - previousWeight) > threshold) {
    float weightDifference = currentWeight - previousWeight;

    totalWeight +=weightDifference;

    if (weightDifference > 0) {
        Serial.print("Added weight to bin: ");
    } else {
        Serial.print("Removed weight from bin: ");
    }

    Serial.println(abs(weightDifference), 1);
    Serial.print("Updated total weight in bin: ");
    Serial.println(totalWeight, 1);

    client.publish(mqtt_topic_weight,String(totalWeight,1).c_str());

    // Update the previous weight to currentWeight.
    previousWeight = currentWeight;
    stableCount = 0;

    } 
  } else{
    Serial.println("HX711 not ready");
  }

    // Put the ADC in sleep mode to save power
    static unsigned long lastReadingTime = 0;
    unsigned long currentMillis = millis();
    if (currentMillis - lastReadingTime >= powerDownInterval){
      lastReadingTime = currentMillis;
      scale.power_down();
      delay(10);
      scale.power_up();
   }    
}

// Utility Functions getCalibration and initializeScale
void getCalibration(){
  if(scale.is_ready()){
    scale.set_scale();

    Serial.println("Tare.. remove any weights from scale");
    delay(5000);

    scale.tare();
    Serial.println("Tare done");

    Serial.print("Place a known weight on the scale");
    delay(5000);

    long reading = scale.get_units(10);
    Serial.print("Result: ");
    Serial.println(reading);

  }else {
   Serial.println("HX711 NOT FOUND.");
  }

  delay(1000);
}

void initializeScale(){
  Serial.println("HX711 TEST_SCALE");
  Serial.println("Initializing the scale");

  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

  Serial.println("Before setting up the scale");
  Serial.print("read: \t\t");
  Serial.println(scale.read());// print a raw reading from the ADC
  Serial.print("read average: \t\t");
  Serial.println(scale.read_average(20));// print the average of 20 readings from the ADC
  Serial.print("get value: \t\t");
  Serial.println(scale.get_value(5)); // print the average of 5 readings from the ADC minus the tare weight (not set yet)
  Serial.print("get units: \t\t");
  Serial.println(scale.get_units(5),1);// print the average of 5 readings from the ADC minus tare weight (not set) divided
  
  scale.set_scale(calibration);// Set callibration
  delay(1000);

  scale.tare();// reset the scale to 0
  Serial.print("After setting up the scale");
  Serial.print("read: \t\t");
  Serial.println(scale.read());
  Serial.print("read average: \t\t");
  Serial.println(scale.read_average(20));
  Serial.print("get value: \t\t");
  Serial.println(scale.get_value(5));
  Serial.print("get units: \t\t");
  Serial.println(scale.get_units(5),1);

  Serial.println("Readings:");
}
