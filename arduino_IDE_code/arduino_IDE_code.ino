#include "HX711.h"
#include "soc/rtc.h"
const int LOADCELL_DOUT_PIN = 4;
const int LOADCELL_SCK_PIN = 19;

HX711 scale;

void setup() {
  Serial.begin(115200);

  //RTC if the freq for esp is to fast.

  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  
}
void loop() {
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
  delay(1000);
}
//Callobration är reading/vikten vi vet och reading det kommer vi få när vi gör Serial.println(reading);
