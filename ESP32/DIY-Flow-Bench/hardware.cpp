/***********************************************************
 * The DIY Flow Bench project
 * https://diyflowbench.com
 * 
 * Hardware.cpp - Hardware class
 *
 * Open source flow bench project to measure and display volumetric air flow using an ESP32 / Arduino.
 * 
 * For more information please visit the WIKI on our GitHub project page: https://github.com/DeeEmm/DIY-Flow-Bench/wiki
 * Or join our support forums: https://github.com/DeeEmm/DIY-Flow-Bench/discussions 
 * You can also visit our Facebook community: https://www.facebook.com/groups/diyflowbench/
 * 
 * This project and all associated files are provided for use under the GNU GPL3 license:
 * https://github.com/DeeEmm/DIY-Flow-Bench/blob/master/LICENSE
 * 
 * 
 ***/

#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <Arduino.h>
#include "hardware.h"
#include "configuration.h"
#include "constants.h"
#include "structs.h"
#include "pins.h"
#include "maths.h"
#include "messages.h"
#include LANGUAGE_FILE



/***********************************************************
* CONSTRUCTOR
*/
Hardware::Hardware() {
  
}



/***********************************************************
* Configure pins
*
***/
void Hardware::configurePins () {
 
  // Inputs
  pinMode(MAF_PIN, INPUT); 
  pinMode(VOLTAGE_PIN, INPUT); 
  pinMode(SPEED_SENSOR_PIN, INPUT); 
  pinMode(REF_PRESSURE_PIN, INPUT); 
  pinMode(DIFF_PRESSURE_PIN, INPUT); 
  pinMode(PITOT_PIN, INPUT); 
  #if defined TEMP_SENSOR_TYPE_LINEAR_ANALOG || defined TEMP_SENSOR_TYPE_SIMPLE_TEMP_DHT11
    pinMode(TEMPERATURE_PIN, INPUT); 
  #endif
  #if defined RELH_SENSOR_TYPE_LINEAR_ANALOG || defined RELH_SENSOR_TYPE_SIMPLE_RELH_DHT11
    pinMode(HUMIDITY_PIN, INPUT);  
  #endif
  #if defined BARO_SENSOR_TYPE_LINEAR_ANALOG || defined BARO_SENSOR_TYPE_MPX4115
    pinMode(REF_BARO_PIN, INPUT);
  #endif
  

  // Outputs
  pinMode(VAC_BANK_1, OUTPUT);
  pinMode(VAC_BANK_2, OUTPUT);
  pinMode(VAC_BANK_2, OUTPUT);
  pinMode(VAC_SPEED_PIN, OUTPUT);
  pinMode(VAC_BLEED_VALVE_PIN, OUTPUT);
  pinMode(AVO_ENBL, OUTPUT);
  pinMode(AVO_STEP, OUTPUT);
  pinMode(AVO_DIR, OUTPUT);
  pinMode(VAC_BANK_1, OUTPUT);

}


/***********************************************************
* INITIALISE HARDWARE
* 
* TODO: Need to move ALL hardware initialisation into here
*
***/
void Hardware::begin () {

  Messages _message;
  
  configurePins();
  
  this->getI2CList(); // TODO: move into status dialog instead of printing to serial monitor
  
  
  
  
// Support for [Adafruit 1015] (12bit) ADC
// https://github.com/adafruit/Adafruit_ADS1X15
//  #ifdef PREF_SRC_ADC_1015 
//    Adafruit_ADS1015 ads1015; 
//    ads1015.begin(ADC_I2C_ADDR);
//  #endif


// Support for [Adafruit 1115] (16bit) ADC
// https://github.com/adafruit/Adafruit_ADS1X15
//  #ifdef PREF_SRC_ADC_1115
//    Adafruit_ADS1115 ads1115;
//    ads1115.begin(ADC_I2C_ADDR);
//  #endif




// Support for DHT11 humidity / temperature sensors
// https://github.com/winlinvip/SimpleDHT
// #if RELH_SENSOR_SIMPLE_RELH_DHT11 || TEMP_SENSOR_SIMPLE_TEMP_DHT11
//   #include <SimpleDHT.h>  
//   SimpleDHT11 dht11(HUMIDITY_PIN);    
// #endif

}



/***********************************************************
* Get list of I2C devices 
*
* Based on: https://www.esp32.com/viewtopic.php?t=4742
***/
//int * Hardware::getI2CList() {   
void Hardware::getI2CList() {   
  
  Messages _message;
  // TODO: Detect I2C devices on bus and return addresses in array to be displayed in status dialog

  // ESP32 I2C Scanner
  // Based on code of Nick Gammon  http://www.gammon.com.au/forum/?id=10896
  // ESP32 DevKit - Arduino IDE 1.8.5
  // Device tested PCF8574 - Use pullup resistors 3K3 ohms !
  // PCF8574 Default Freq 100 KHz 
  
  Wire.begin (SCA_PIN, SCL_PIN); 

  Serial.println ();
  Serial.println ("Scanning for I2C devices...");
  byte count = 0;

  Wire.begin();
  for (byte i = 8; i < 120; i++)   {
    Wire.beginTransmission (i);          // Begin I2C transmission Address (i)
    if (Wire.endTransmission () == 0)  { // Receive 0 = success (ACK response) 
    
      Serial.print ("Found address: ");
      Serial.print (i, DEC);
      Serial.print (" (0x");
      Serial.print (i, HEX);     // PCF8574 7 bit address
      Serial.println (")");
      count++;
    }
  }
  Serial.print ("Found ");      
  Serial.print (count, DEC);        // numbers of devices
  Serial.println (" device(s).");
  Serial.println (" ");
}



/***********************************************************
* GET ADS1015 ADC value
*
* Based on: https://github.com/sparkfun/SparkFun_ADS1015_Arduino_Library/blob/master/src/SparkFun_ADS1015_Arduino_Library.cpp
* and :https://github.com/adafruit/Adafruit_ADS1X15/blob/master/Adafruit_ADS1X15.cpp
*
 ***/
int Hardware::getADCRawData(int channel) {

  if (channel > 3) {
    return 0;
  }
  
  uint16_t mode = ((4 + channel) << 12);
    
  uint16_t config = 0x8000;
  config |= mode;
  config |= 0X0000; // Gain (+/- 6.144v) NOTE: we are assuming all sensors have same FSD, else we need to scale each sensor within each sensor->function in sensors.cpp
  config |= 0x0100; // Mode = Single
  config |= 4; // data rate = 4 (default)
  config |= 0x0000; // comparator mode traditional
  config |= 0x0000; // polarity low
  config |= 0x0000; // alert is non-latching
  config |= 3;
  
  Wire.beginTransmission(ADC_I2C_ADDR);  
  
  Wire.write((uint8_t)0x01);
  Wire.write((uint8_t)(config >> 8));
  Wire.write((uint8_t)(config & 0xFF));
  Wire.endTransmission();
  
  delay(10); // TODO: This is a hack, need to test for transmission complete. (Delays are bad Mmmmnnkay) [might not even need it, screen refresh rate might be enough!]
  
  Wire.beginTransmission(ADC_I2C_ADDR);
  Wire.write(0x00);
  Wire.endTransmission();

  int rv = Wire.requestFrom(ADC_I2C_ADDR, (uint8_t) 2);
  int raw;
  if (rv == 2) {
    raw = Wire.read() << 8;
    raw += Wire.read();
  }
  
  #ifdef ADC_TYPE_ADS1015
    raw >>= 4;  // Shift 12-bit results
  #endif
  
  return int(raw);
  
}


/***********************************************************
* GET BOARD VOLTAGE
* Measures 5v supply buck power to ESP32 via voltage divider
* We use a 10k-10k divider on the official shield
* This gives a max of 2.5v which is fine for the ESP32's 3.3v logic
* Use a 0.1uf cap on input to help filter noise
*
* NOTE: ESP32 has 12 bit ADC (0-3.3v = 0-4095)
***/
int Hardware::getSupplyMillivolts() {   

  int rawVoltageValue = analogRead(VOLTAGE_PIN);
  
  float supplyMillivolts = (2 * (rawVoltageValue * (3.3 / 4095.0) * 1000)) + SUPPLY_MV_TRIMPOT;

  return supplyMillivolts;
  
}



/***********************************************************
* BENCH IS RUNNING
*/
bool Hardware::benchIsRunning() {
    
  Messages _message;
  Maths _maths;
  
  extern struct ConfigSettings config;
  
  // TODO: Check scope of these...
  float refPressure = _maths.calculateRefPressure(INWG);
  float mafFlowRateCFM = _maths.calculateFlowCFM();

  if ((refPressure > config.min_bench_pressure) && (mafFlowRateCFM > config.min_flow_rate))
  {
	  _message.Handler(LANG_BENCH_RUNNING);
	  return true;
  } else {
    _message.Handler(LANG_NO_ERROR);
	  return false;
  }
}


/***********************************************************
* CHECK REFERENCE PRESSURE
* Make sure that reference pressure is within limits
***/
void Hardware::checkRefPressure() {
  
  Messages _message;
  Maths _maths;
  
  extern struct ConfigSettings config;
  
  float refPressure = _maths.calculateRefPressure(INWG);
    
  // Check that pressure does not fall below limit set by MIN_TEST_PRESSURE_PERCENTAGE when bench is running
  // note alarm commented out in alarm function as 'nag' can get quite annoying
  // Is this a redundant check? Maybe a different alert would be more appropriate
  if ((refPressure < (config.cal_ref_press * (MIN_TEST_PRESSURE_PERCENTAGE / 100))) && (Hardware::benchIsRunning()))
  {
    _message.Handler(LANG_REF_PRESS_LOW);
  }
}
