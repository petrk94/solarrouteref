/*
 * Arduino Programm  für Solarrouter Projekt
 * 
 * Noch Notwendige Implentierungen:
 *    - Netwerkshield Programmcode
 *    - GP2Y1010AU0F - Sensor für PM2.5 Messwerte
 * 	  - ACS712 - Strom vom Solarmodul zum Akku
 * 	  - ACS712 - Stromverbrauch von Hardware
 *    - Lux-Meter
 * 
 * 
 * 
 */

#include <SPI.h>
#include <SD.h>

#include "DHT.h"
#define DHTPIN 2
#define DHTTYPE DHT22

#include <SoftwareSerial.h>
#include <Wire.h>
#include <Time.h>
#include <DS1307RTC.h>

const int chipSelect = 10;

// Confi für Sensoren
const int analogInPinLDR = A0;  // LDR
const int analogInPinUakku = A1;  // Spannungsabgriff für Akku
const int analogInPinUsolar = A2;  // Spannungsabgriff für Akku
int sensorValueLDR = 0;
int sensorValueUakku = 0;
int sensorValueUsolar = 0;

const int threshold = 570;   // Muss angespasst werden

float vinakku = 0.0;
float voutakku = 0.0;
float vinsolar = 0.0;
float voutsolar = 0.0;
float R1 = 100000.0; // 100K
float R2 = 10000.0;  // 10K

// Timer Confi
const long oneSecond = 1000;
const long oneMinute = oneSecond*60;
const long fiveMinutes = oneMinute*5;

// These constants, define values needed for the LDR readings and ADC
#define LDR_PIN                   0
#define MAX_ADC_READING           1023
#define ADC_REF_VOLTAGE           5.0
#define REF_RESISTANCE            5030  // measure this for best results
#define LUX_CALC_SCALAR           12518931
#define LUX_CALC_EXPONENT         -1.405



DHT dht(DHTPIN, DHTTYPE);

void setup(){
  Serial.begin(9600);
  dht.begin();
  

 // SD Karten Confi
  Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("card initialized.");

}


    
void loop()
{
  int   ldrRawData;
  float resistorVoltage, ldrVoltage;
  float ldrResistance;
  float ldrLux;

  
  // Misst den Helligkeitswert vom LDR
  int sensorValueLDR = analogRead(analogInPinLDR);

  // Misst Luftfeuchtigkeit in Prozent
  float h = dht.readHumidity();
  // Misst die Temperatur in Celsius
  float t = dht.readTemperature();

  // Spannung Messung für Akku
  // Spannungsteiler R1= 100K R2= 10K
  
  int sensorValueUakku = analogRead(analogInPinUakku);
  // Berechnung der Akku Spannung; vin ist die reale Akkuspannnung
  voutakku = (sensorValueUakku * 5.0)/1024.0;
  vinakku = voutakku/(R2/(R1+R2));

  // Spannung Messung für Solarmodul
  // Spannungsteiler R1= 100K R2= 10K
  
  int sensorValueUsolar = analogRead(analogInPinUsolar);
  // Berechnung der Solarmodul Spannung; vin ist die reale Solarmodul Spannung
  voutsolar = (sensorValueUsolar * 5.0)/1024.0;
  vinsolar = voutsolar/(R2/(R1+R2));
  


  // Berechnung Lux Werte von dem LDR Spannungsteiler 5V -- LDR --- Ax --- 10kOhm --- GND
  // Programmcode stammt von http://www.allaboutcircuits.com/projects/design-a-luxmeter-using-a-light-dependent-resistor/
  // Perform the analog to digital conversion  
  ldrRawData = analogRead(analogInPinLDR);
  
  // RESISTOR VOLTAGE_CONVERSION
  // Convert the raw digital data back to the voltage that was measured on the analog pin
  resistorVoltage = (float)ldrRawData / MAX_ADC_READING * ADC_REF_VOLTAGE;

  // voltage across the LDR is the 5V supply minus the 5k resistor voltage
  ldrVoltage = ADC_REF_VOLTAGE - resistorVoltage;
  
  // LDR_RESISTANCE_CONVERSION
  // resistance that the LDR would have for that voltage  
  ldrResistance = ldrVoltage/resistorVoltage * REF_RESISTANCE;
  
  // LDR_LUX
  // Change the code below to the proper conversion from ldrResistance to
  // ldrLux
  ldrLux = LUX_CALC_SCALAR * pow(ldrResistance, LUX_CALC_EXPONENT);





  // Serielle Datenausgabe
  // Ausgabe LDR
  Serial.print("SensorValue LDR: ");
  Serial.println(sensorValueLDR);
  // Ausgabe Luftfeuchtigkeit
  Serial.print("SensorValue Humidity: ");
  Serial.println(t);
  // Ausgabe Temperatur
  Serial.print("SensorValue Temperature: ");
  Serial.println(t);
  // Ausgabe Spannung Akku
  Serial.print("SensorValue U Akku: ");
  Serial.println(vinakku);
  // Ausgabe Spannung Solarmodul
  Serial.print("SensorValue U Solar: ");
  Serial.println(vinsolar);



  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open("datalog.txt", FILE_WRITE);

  // if the file is available, write to it:
    tmElements_t tm;


    


  if (RTC.read(tm)) {
    dataFile.print(";");
    dataFile.print(tm.Hour);
    dataFile.print(':');
    dataFile.print(tm.Minute);
    dataFile.print(':');
    dataFile.print(tm.Second);
    dataFile.print(";");
    dataFile.print(tm.Month);
    dataFile.print('-');
    dataFile.print(tm.Day);
    dataFile.print('-');
    dataFile.print(tmYearToCalendar(tm.Year));
    dataFile.println();
  }
  
    dataFile.close();
    
  }




void print2digits(int number) {
  if (number >= 0 && number < 10) {
    Serial.write('0');
  }
}

