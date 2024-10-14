#include "HX711.h"//This is the libary we are using for all our funtions and they are
                  //explained more in libary.
#include "soc/rtc.h"
const int LOADCELL_DOUT_PIN = 4;
const int LOADCELL_SCK_PIN = 19;

HX711 scale;

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

  scale.set_scale(1126.5);//change this callabration to ur own value and callabariation = reading/weight.
  
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
}
void loop() {
 Serial.print("one reading:\t");
  Serial.print(scale.get_units(), 1);
  Serial.print("\t| average:\t");
  Serial.println(scale.get_units(100), 5);

  scale.power_down();             // put the ADC in sleep mode
  delay(5000);
  scale.power_up();
}
//Callobration är reading/vikten vi vet och reading det kommer vi få när vi gör Serial.println(reading);
