#include <LiquidCrystal.h>

#define SENSOR_PIN					0		// Analog pin used for sensor
#define	RLOAD						10	    // Load resistance (kohms)
#define	RO_CLEAN_AIR				10	    // RS in clean air / RO (from datasheet in kohms)
#define CALIBRATION_SAMPLES			50		// Number of samples taken during calibration
#define CALIBRATION_SAMPLING_TIME	500		// Time interval between samples during calibration
#define READ_SAMPLES				50
#define READ_SAMPLING_TIME			5

float lpgCurve[3] = { 3, 0, -0.4};		// Data format:{ x, y, slope}; point1: (lg1000, lg1), point2: (lg10000, lg0.4)
										// Assumed linear
										
float Ro = 10;	//Ro is initialized to 10 kilo ohms

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

float ppmSensorValue;
int val;

void setup() {

	lcd.begin(16, 2);
	lcd.print("LPG Sensor");
	
	lcd.setCursor(0, 1);
	lcd.print("Calibrating...");
	
	Ro = calibrateSensor(SENSOR_PIN);   
	
	lcd.clear();                                
	lcd.print("Calibrated."); 
	lcd.setCursor(0, 1);
	lcd.print("Ro = ");
	lcd.print(Ro);
	lcd.print(" kohms");
	
	delay(2000);
  
}

void loop() {
	val = analogRead(SENSOR_PIN);
	lcd.clear();
	ppmSensorValue = pow(10, ((((log(calculateSensorResistance(val) / Ro)) - lpgCurve[1]) / lpgCurve[2]) + lpgCurve[0]));
	
	lcd.setCursor(0, 0);
	lcd.print("LPG ");
        lcd.print(val);
	lcd.setCursor(0, 1);
	lcd.print("Value: ");
	lcd.print(ppmSensorValue);
	lcd.print(" ppm");
	
	delay(750);
	
}

float calculateSensorResistance(int raw_adc) {
  return (((float)RLOAD*(1023-raw_adc)/raw_adc));
}

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
