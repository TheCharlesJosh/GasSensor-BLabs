//  ########      ###     ##    ##      ###     ##    ##   ####  ##     ##     ###     ##    ##        ##            ###     ########    ######    
//  ##     ##    ## ##     ##  ##      ## ##    ###   ##    ##   ##     ##    ## ##    ###   ##        ##           ## ##    ##     ##  ##    ##   
//  ##     ##   ##   ##     ####      ##   ##   ####  ##    ##   ##     ##   ##   ##   ####  ##        ##          ##   ##   ##     ##  ##         
//  ########   ##     ##     ##      ##     ##  ## ## ##    ##   #########  ##     ##  ## ## ##        ##         ##     ##  ########    ######    
//  ##     ##  #########     ##      #########  ##  ####    ##   ##     ##  #########  ##  ####        ##         #########  ##     ##        ##   
//  ##     ##  ##     ##     ##      ##     ##  ##   ###    ##   ##     ##  ##     ##  ##   ###        ##         ##     ##  ##     ##  ##    ##   
//  ########   ##     ##     ##      ##     ##  ##    ##   ####  ##     ##  ##     ##  ##    ##        ########   ##     ##  ########    ######    

/**
 * Define values for the device.
 */

//Pins
#define STEPPER_C1    9
#define STEPPER_C2    10
#define WIFLY_RX      6
#define WIFLY_TX      7
#define LCD_D4        5
#define LCD_D5        4
#define LCD_D6        3
#define LCD_D7        2
#define LCD_E         11
#define LCD_RS        12

//Constants
#define SENSOR_PIN                  0       // Analog pin used for sensor
#define RLOAD                       10      // Load resistance (kohms)
#define RO_CLEAN_AIR                10      // RS in clean air / RO (from datasheet in kohms)
#define CALIBRATION_SAMPLES         50      // Number of samples taken during calibration
#define CALIBRATION_SAMPLING_TIME   500     // Time interval between samples during calibration
#define READ_SAMPLES                50      // Number of samples taken during normal operation
#define READ_SAMPLING_TIME          5       // Time interval between samples during normal operation

#define USB_SPD       9600        //Baudrate for USB Communication
#define WIFLY_SPD     9600        //Baudrate for WiFly Communication; change if needed
#define WIFLY_DELAY   500         //Delay before executing another event. Recommended at 250 ms or higher.
#define MAX_RESPONSE_LENGTH 1000  //The number of characters the response string can handle.   
#define STEPPER_REVO  200         //Steps per revolution -- needs calibration
#define STEPPER_SPD   70          //Stepper's RPM -- needs calibration
//#define MQTT_SERVER   "q.m2m.io"  //The DNS name of the server. If using an IP address, comment out this line and uncomment one of the lines below starting with "byte MQTT_SERVER"
#define MQTT_SERVER   "test.mosquitto.org"  //The DNS name of the server. If using an IP address, comment out this line and uncomment one of the lines below starting with "byte MQTT_SERVER"

/**
 * Libraries needed for the device.
 */
#include <Ethernet.h>
#include <PubSubClient.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <WiFly.h>
#include <LiquidCrystal.h>
#include <Stepper.h>

/**
 * Pre-operation initializations.
 */

//Constants
//byte MQTT_SERVER[] = { 192, 168, 1, 100 };       //Lifted from GasSensorWifi/wiflyshield.h; SQUAT
//byte MQTT_SERVER[] = { 172, 16, 2, 224 };        //Lifted from GasSensorWifi/wiflyshield.h; DILNET
float lpgCurve[3] = { 3, 0, -0.4};                // Data format: {x, y, slope}; point1: (lg1000, lg1), point2: (lg10000, lg0.4); Assumed linear
float Ro = 10;                                    //Ro is initialized to 10 kilo ohms
int ppmSensorValue;                               //Holds raw value from analog input
char ppmSensorValueMessage[6];                    //Holds data sent via MQTT
char wiFlyResponse[MAX_RESPONSE_LENGTH] = "";     //Holds the data from the module, concatenation of data from charToString
char charToString[2] = {' '};                     //Holds the characters grabbed from read; to be added to string wiFlyResponse

//Functions
// // void initializeAll();
// float getResistance(int raw_adc);
// float calibrateSensor(int sensorPin);
// float readSensor(int sensorPin);

//Library-defined initializations
LiquidCrystal lcd(LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7); //Initialize 16x2 LCD Module
Stepper stepper(STEPPER_REVO, STEPPER_C1, STEPPER_C2);            //Initialize Bipolar Stepper Motor
SoftwareSerial serialWiFly = SoftwareSerial(WIFLY_RX, WIFLY_TX);  //Utilize software serial for WiFly
WiFlyClient wifly;                                                //Initialize WiFly module (comparable to EthernetClient)
PubSubClient mqttClient(MQTT_SERVER, 1883, 0, wifly);             //Initialize the PubSub client (server, port, subscription callback, client); 1883 is the standard port for MQTT; callback set to 0 because we don't need to read new messages.

/**
 * Initialize All function
 *    Begins by setting the baudrate of the WiFly and the Serial to the set constants (chunk 1), 
 *    then tells the library to use the SoftwareSerial defined above (serialWiFly) instead of the default serial (chunk 2), 
 *    then waits for outgoing streams to finish sending data (chunk 3).
 *    LCD initializes and clears data (chunk 4).
 *    Stepper speed has been set using the STEPPER_SPD constant (chunk 5).
 */

void initializeAll(){
  Serial.begin(USB_SPD);
  serialWiFly.begin(WIFLY_SPD);

  WiFly.setUart(&serialWiFly);

  //Serial.flush();
  //serialWiFly.flush();

  lcd.begin(16,2);
  lcd.clear();

  stepper.setSpeed(STEPPER_SPD);

  delay(WIFLY_DELAY);
}

/**
 * Get MQ-6 Sensor Resistance function (return value is in kiloohms)
 *    The sensor and the load resistor forms a voltage divider.
 *    Given the voltage across the load resistor and its resistance,
 *    the resistance of the sensor could be derived.
 */

float getResistance(int raw_adc){
    return (((float)RLOAD*(1023-raw_adc)/raw_adc));
}

/**
 * Calibrate MQ-6 Sensor function
 *    Calculates the sensor resistance in clean air (ie. no presence of leaking LPG).
 */
float calibrateSensor(int sensorPin){
  int i = 0;
  int rawSensorValue = 0;
  float averageSensorValue = 0;
  
  while(i<CALIBRATION_SAMPLES) {
    averageSensorValue += getResistance(analogRead(SENSOR_PIN));
    i++;
    delay(CALIBRATION_SAMPLING_TIME);
  }
  
  averageSensorValue = averageSensorValue / CALIBRATION_SAMPLES;
  averageSensorValue = averageSensorValue / RO_CLEAN_AIR;
  
  return averageSensorValue;
}

/**
 * Read MQ-6 Sensor Raw Data function
 *    Calculates the sensor resistance during normal operation (average of READ_SAMPLES)
 */
float readSensor(int sensorPin){
    int i;
    float sensorResistance = 0;

    for (i=0;i<READ_SAMPLES;i++) {
        sensorResistance += getResistance(analogRead(sensorPin));
        delay(READ_SAMPLING_TIME);
    }

    sensorResistance = sensorResistance/READ_SAMPLES;

    return sensorResistance;  
}

/**
 * Get Response function
 *    Gets the data sent back by the WiFly and stores it into wiFlyResponse.
 */

void getResponse(){
  while(serialWiFly.available() > 0) {
    charToString[0] = serialWiFly.read();
    strcat(wiFlyResponse, charToString);
  } 
  Serial.println(wiFlyResponse);
}

/**
 * Compare and Reset function
 *    Using the strstr function, find if receivedResponse has expectedResponse in it (usually "<4.00>", "EXIT", "CMD").
 *    If the string has the expectedResponse, reset the receivedResponse back to an empty string.
 */

boolean compareAndReset(char *receivedResponse, const char *expectedResponse){
  char *stringPointer;
  stringPointer = strstr(receivedResponse, expectedResponse);
  
  if(stringPointer != NULL){
    strcpy(receivedResponse, "");
    return true;
  } else return false;
}

/**
 * Is In Command Mode function
 *    Determines if the WiFly module is in Command Mode.
 *    There are three scenarios that may happen during this event.
 *    1. The WiFly module is inaccessible (unplugged, not responding, bricked). The function will return false.
 *    2. The WiFly module has been recently booted. The function will return true.
 *    3. The WiFly module has been already booted. The function will return true.
 */

boolean isInCommandMode(){
  boolean inCommandMode = false;
  delay(WIFLY_DELAY);
  serialWiFly.write("$$$");
  delay(WIFLY_DELAY);
  getResponse();
  if(compareAndReset(wiFlyResponse, "CMD")){
    Serial.println("WiFly Command Mode.");
    inCommandMode = true;
  } else if(compareAndReset(wiFlyResponse, "<4.00>")){
    Serial.println("Warning: Already in Command Mode. Was the device resetted prematurely?");
    inCommandMode = true;
  } else {
    Serial.println("wat.");
  }
  return inCommandMode;
}

void setup(){
  initializeAll();
  lcd.print("Hello!");
  lcd.setCursor(0,1);
  //WiFly.begin();
  //lcd.print("1");
  boolean tryHard = mqttClient.connect("arduinoLPG");
  if(mqttClient.connected()){
    lcd.print("YAY");
  } else {
    lcd.print("NAY");
  }
  lcd.print(tryHard);
  mqttClient.publish("test/hub", "helloworlde");
}

void loop(){

}