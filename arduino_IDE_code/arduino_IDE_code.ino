#include "HX711.h"//This is the libary we are using for all our funtions and they are
                  //explained more in libary.
#include "soc/rtc.h"
#include <WiFi.h>
#include <PubSubClient.h>

const char *ssid = "TN-WH2983";
const char *pass = "SyufjufDeub5";

const int LOADCELL_DOUT_PIN = 4;
const int LOADCELL_SCK_PIN = 19;

const char *mqtt_broker = "broker.emqx.io";
const int mqtt_port = 1883;
const char *mqtt_topic = "esp32/scale";
const char *mqtt_username = "emqx";
const char *mqtt_password = "123abc";

WiFiClient espClient;
PubSubClient client(espClient);

HX711 scale;

float previousWeight = 0;
float threshold = 1.0;
float calibration = 1100.38987;

const int trigPin = 7;
const int echoPin = 8;

#define SOUND_SPEED 0.034

long duration;
float distanceCm;

unsigned long previousMillisScale = 0;
unsigned long previousMillisDistande =0;
const long ultrasonicInterval = 1000;
const long scaleInterval = 5000;

void setup() {
  Serial.begin(115200);

  Serial.println("HX711 TEST_SCALE");

  Serial.println("Initializing the scale");
  //RTC if the freq for esp is to fast.
  //
  //
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
            // by the SCALE parameter (not set yet)

  scale.set_scale(calibration);//change this callabration to ur own value and callabariation = reading/weight.
  
  delay(1000);
  scale.tare();// reset the scale to 0

  Serial.print("After setting up the scale");

  Serial.print("read: \t\t");
  Serial.println(scale.read());// print a raw reading from the ADC

  Serial.print("read average: \t\t");
  Serial.println(scale.read_average(20));// print the average of 20 readings from the ADC

  Serial.print("get value: \t\t");
  Serial.println(scale.get_value(5));// print the average of 5 readings from the ADC minus the tare weight, set with tare()

  Serial.print("get units: \t\t");
  Serial.println(scale.get_units(5),1);// print the average of 5 readings from the ADC minus tare weight, divided
            // by the SCALE parameter set with set_scale

  Serial.println("Readings:");  

  //Connection to wifi
  WiFi.begin(ssid,pass);
  while (WiFi.status() !=WL_CONNECTED){
    delay(1000);
    Serial.println("connecting to wifi...");
  }
   Serial.println("Connected to the Wi-fi network");

   //Connection to mqtt broker
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
   pinMode(trigPin,OUTPUT);
   pinMode(trigPin,INPUT);
}
void loop() {
  //Raw reeading value because reading-tare/factor and we dont have factor,
  //we will then get different raw reading values depending on the weight of the object
  //that we can use for reading/object weight to get callobaration factor, to then put it in
  //set_scale(callobarion factor) to convert data to human readable.
  /*void loop() {
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
  }else{
    Serial.println("HX711 NOT FOUND.");
  }
  delay(1000);*/

 // Get the current weight reading
  float currentWeight = scale.get_units(20);  // Average of 10 readings for stability

   if (currentWeight < 0) {
    currentWeight = 0;
  } else if (currentWeight > 1000) {
    currentWeight = 1000;
  }

  // Check if currentweight minus previousweight is greater then threshold.
  if (abs(currentWeight - previousWeight) > threshold) {
    Serial.print("Change detected. Weight:");
    Serial.print(currentWeight, 1);  // Print current weight with 1 decimal
    Serial.print(" g | Previous weight: "); 
    Serial.print(previousWeight, 1);  // same as line 83
    Serial.println(" g");

    int weightInt = (int)currentWeight;
    client.publish(mqtt_topic,String(weightInt).c_str());

    // Update the previous weight to currentWeight.
    previousWeight = currentWeight;
  }
  client.loop();

  scale.power_down();  // Put the ADC in sleep mode to save power
  delay(5000);  // Wait for 5 seconds
  scale.power_up();  // Power up the ADC
}
//Callobration är reading/vikten vi vet och reading det kommer vi få när vi gör Serial.println(reading);
