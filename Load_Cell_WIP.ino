#include <HX711_ADC.h>
#if defined(ESP8266) || defined(ESP32) || defined(AVR)
#include <EEPROM.h>
#endif

//our ooo is:
//power on
//start serial and opamp
//wait 2s and tare
//check for timeout
//run calibration - empty scale, type t, zero scale - place known mass, type mass value - calculate calibration factor
//if we can, save cal factor to EEPROM
//enter loop()
//Update HX711, get calibrated data, print value, repeat.

//variables
/////////////////////////////////////////////////////

char calReadyChar;

// data out from HX711 amplifier
const int HX711_dout = 4;
//microcontroller serial clock
const int HX711_sck = 5;

float newCalibrationValue;

float known_mass;

float experimental_mass;

HX711_ADC LoadCell(HX711_dout, HX711_sck);

// calibration #
const int calVal_eepromAdress = 0;
//time between printing
unsigned long t = 100;
////////////////////////////////////////////////////////

void calibrate(){
  Serial.println("Type and enter t once scale is empty to begin calibration.");
  while(calReadyChar != 't'){
  	if (Serial.available()>0){  
      calReadyChar = Serial.read();
    }
      delay(1);
  }
      LoadCell.tare();

    Serial.println("Place object of known weight on gauge, then type and enter that object's weight in grams (tenths place for kilograms).");
  while(known_mass==0){
    known_mass = Serial.parseFloat();
  }
  LoadCell.refreshDataSet();
  newCalibrationValue = LoadCell.getNewCalibration(known_mass);
  LoadCell.setCalFactor(newCalibrationValue);
  //TODO: save calibrationvalue to EEPROM if we can
  Serial.print("New calibration factor is ");
  Serial.println(newCalibrationValue);
  delay(1000);
  
//calibration value == post tare counts/known mass
//thus mass == post tare count/calibration value
}



void setup() {
Serial.begin(57600);
LoadCell.begin();

unsigned long stabilizingtime = 2000;
//tare is basically "zeroing"
boolean _tare = true;
LoadCell.start(stabilizingtime, _tare);
Serial.println("at tare");
if (LoadCell.getTareTimeoutFlag() || LoadCell.getSignalTimeoutFlag()){
  Serial.print("Tare or HX711 Signal timed out. Wuh oh.");
  //prevents further action until reset
  while (1);
} 


LoadCell.setCalFactor(1.0);
while (!LoadCell.update());
calibrate();

}

void loop() {
  LoadCell.update();
  Serial.println(LoadCell.getData());
  delay(t);
}
