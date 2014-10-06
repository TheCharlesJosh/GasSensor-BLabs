#include <Ethernet.h>
#include <LiquidCrystal.h>
#include <PubSubClient.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <WiFly.h>

#include "wiflyshield.h"

#define SENSOR_PIN					0		// Analog pin used for sensor
#define	RLOAD						10	    // Load resistance (kohms)
#define	RO_CLEAN_AIR				10	    // RS in clean air / RO (from datasheet in kohms)
#define CALIBRATION_SAMPLES			50		// Number of samples taken during calibration
#define CALIBRATION_SAMPLING_TIME	500		// Time interval between samples during calibration
#define READ_SAMPLES				50      // Number of samples taken during normal operation
#define READ_SAMPLING_TIME			5       // Time interval between samples during normal operation

float lpgCurve[3] = { 3, 0, -0.4};		// Data format:{ x, y, slope}; point1: (lg1000, lg1), point2: (lg10000, lg0.4)
										// Assumed linear
										
float Ro = 10;	//Ro is initialized to 10 kilo ohms

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

int ppmSensorValue;

char ppmSensorValueMessage[6];

void setup() {
    
    initWifly();

	lcd.begin(16, 2);
	lcd.print("LPG Sensor");
	Serial.println("LPG Sensor");
	
	lcd.setCursor(0, 1);
	lcd.print("Calibrating...");
	Serial.println("Calibrating...");
	
    // Get sensor resistance in clean air.
	Ro = calibrateSensor(SENSOR_PIN);   
	
	lcd.clear();                                
	lcd.print("Calibrated."); 
	Serial.println("Calibrated."); 
	lcd.setCursor(0, 1);
	lcd.print("Ro = ");
	lcd.print(Ro);
	lcd.print(" kohms");
    Serial.print("Ro = ");
	Serial.print(Ro);
	Serial.println(" kohms");
  
}

void loop() {
	
    // Check if Arduino Sensor is connected to the MQTT Server.
    if(wiFlyMqttClient.connected()) {
        // Convert sensor value (int) to string.
		strcpy(ppmSensorValueMessage, itoa(ppmSensorValue, ppmSensorValueMessage, 10));
		// Send sensor value to MQTT server.
        // wiFlyMqttClient.publish(char *topic, char *message);
        wiFlyMqttClient.publish("arduinoSensorValues", ppmSensorValueMessage);
	} else {
        // Connect to WiFi network.
        connectToNetworkUsingWiFly();
        // Connect to MQTT server.
		wiFlyMqttClient.connect("arduinoSensorClient");	
	}
    
	lcd.clear();
    
    // Get sensor value from analog pin.
	ppmSensorValue = pow(10, ((((log(calculateSensorResistance(analogRead(SENSOR_PIN)) / Ro)) - lpgCurve[1]) / lpgCurve[2]) + lpgCurve[0]));
	
	lcd.setCursor(0, 0);
	lcd.print("LPG Sensor");
	lcd.setCursor(0, 1);
	lcd.print("Value: ");
	lcd.print(ppmSensorValue);
	lcd.print(" ppm");
    
    Serial.print("Value: ");
	Serial.print(ppmSensorValue);
	Serial.println(" ppm");
    
	delay(750);
	
}

//*******************************************************************
//  The sensor and the load resistor forms a voltage divider.
//  Given the voltage across the load resistor and its resistance,
//  the resistance of the sensor could be derived.
//*******************************************************************
float calculateSensorResistance(int raw_adc) {
    return (((float)RLOAD*(1023-raw_adc)/raw_adc));
}

//*******************************************************************
//  Calculates the sensor resistance in clean air.
//*******************************************************************
float calibrateSensor(int sensorPin) {
	
	int i = 0;
	int rawSensorValue = 0;
	float averageSensorValue = 0;
	
	while(i<CALIBRATION_SAMPLES) {
		averageSensorValue += calculateSensorResistance(analogRead(SENSOR_PIN));
		i++;
		delay(CALIBRATION_SAMPLING_TIME);
	}
	
	averageSensorValue = averageSensorValue / CALIBRATION_SAMPLES;
	averageSensorValue = averageSensorValue / RO_CLEAN_AIR;
	
	return averageSensorValue;

}

//*******************************************************************
//  Calculates the sensor resistance during normal operation
//*******************************************************************
float readSensor(int sensorPin) {
  
    int i;
    float sensorResistance = 0;

    for (i=0;i<READ_SAMPLES;i++) {
        sensorResistance += calculateSensorResistance(analogRead(sensorPin));
        delay(READ_SAMPLING_TIME);
    }

    sensorResistance = sensorResistance/READ_SAMPLES;

    return sensorResistance;  
  
}
