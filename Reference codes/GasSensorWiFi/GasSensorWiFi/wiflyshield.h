#include "Arduino.h"

#define DEBUG_ON
#define DEBUG_PRINT(receivedResponse) if(strcmp(receivedResponse, "") != 0){ Serial.print("\nWiFly Response: "); Serial.println(receivedResponse);}

#define BAUDRATE 19200
#define MAX_RESPONSE_LENGTH 250
#define NUMBER_SAVED_SSID 1
#define NETWORKS_SCAN_TIMEOUT 6000

#define WIFLY_RX 6
#define WIFLY_TX 7

// Functions
void initWifly();
boolean waitResponseFromWiFly();
boolean compareResponseFromWiFly(char *receivedResponse, const char *expectedResponse);
boolean enterWiFlyCommandMode();
boolean exitWiFlyCommandMode();
void sendSetCommandToWiFly(char *command);
void joinNetworkUsingWiFly(char *network);
boolean checkWiFlyConnection();
//boolean scanNetworksUsingWiFly();
void connectToNetworkUsingWiFly();

void receiveMqttMessageUsingWiFly(char* topic, byte* payload, unsigned int length) {
  // handle message arrived
}

// DILNET
//byte mqttServer[] = { 172, 16, 2, 224 };

// Squat
byte mqttServer[] = { 192, 168, 1, 100 };

WiFlyClient wiFlyModule;
PubSubClient wiFlyMqttClient(mqttServer, 1883, receiveMqttMessageUsingWiFly, wiFlyModule);

SoftwareSerial serialWiFly = SoftwareSerial(WIFLY_RX, WIFLY_TX);

char wiFlyResponse[MAX_RESPONSE_LENGTH] = "";

//char scannedNetwork[25];

//char SAVED_SSID[NUMBER_SAVED_SSID][25] = { "DILNET" };
//char SAVED_PASSWORD[NUMBER_SAVED_SSID][15] = {""};

char SAVED_SSID[NUMBER_SAVED_SSID][25] = { "Squat" };
char SAVED_PASSWORD[NUMBER_SAVED_SSID][15] = { "Jer291113" };

//char matchedPASSWORD[15];

char tempCommand[50];

char charToString[2] = { ' ', '\0'};

boolean isConnected = false;


void initWifly() {

	// Arduino Hardware UART
	Serial.begin(BAUDRATE); 
	
	// WiFly Software UART
	serialWiFly.begin(BAUDRATE);
	WiFly.setUart(&serialWiFly);
	
	Serial.flush();
	serialWiFly.flush();
	
}


boolean waitResponseFromWiFly() {

	while(serialWiFly.available() > 0) {
		
		charToString[0] = serialWiFly.read();
		strcat(wiFlyResponse, charToString);
		
		
		#ifdef DEBUG_ON
		Serial.println("Waiting response from WiFly...");
		#endif
		
	}	

}


boolean compareResponseFromWiFly(char *receivedResponse, const char *expectedResponse) {
	
	char *stringPointer;

	stringPointer = strstr(receivedResponse, expectedResponse);
	
	if(stringPointer != NULL)
		return true;
	else
		return false;

}


boolean enterWiFlyCommandMode() {

	boolean inWiFlyCommandMode = false;
	
	serialWiFly.write("$$$");
	delay(300);
	
	serialWiFly.write("ver\r");
	delay(300);
	
	waitResponseFromWiFly();
	
	#ifdef DEBUG_ON
	DEBUG_PRINT(wiFlyResponse);
	#endif

	inWiFlyCommandMode = compareResponseFromWiFly(wiFlyResponse, "<4.00>");
	strcpy(wiFlyResponse, "");
	
	#ifdef DEBUG_ON
	if(inWiFlyCommandMode)
		Serial.println("WiFly Command Mode\n");
	#endif
	
	return inWiFlyCommandMode;

}


boolean exitWiFlyCommandMode() {

	boolean inWiFlyDataMode = false;

	serialWiFly.write("exit\r");
	delay(500);
	
	waitResponseFromWiFly();
	
	#ifdef DEBUG_ON
	DEBUG_PRINT(wiFlyResponse);
	#endif
	
	inWiFlyDataMode = compareResponseFromWiFly(wiFlyResponse, "EXIT");
	strcpy(wiFlyResponse, "");
	
	#ifdef DEBUG_ON
	if(inWiFlyDataMode)
		Serial.println("WiFly Data Mode\n");
	#endif
	
	return inWiFlyDataMode;

}


void sendSetCommandToWiFly(char *setCommand) {

	boolean setCommandFinished = false;

	while(!enterWiFlyCommandMode());
	
	while(!setCommandFinished) {
	
		#ifdef DEBUG_ON
		Serial.print("Command: ");
		Serial.println(setCommand);
		#endif
		
		serialWiFly.write(setCommand);
		delay(500);
	
		waitResponseFromWiFly();
	
		#ifdef DEBUG_ON
		DEBUG_PRINT(wiFlyResponse);
		#endif
		
		setCommandFinished = compareResponseFromWiFly(wiFlyResponse, "AOK");
		strcpy(wiFlyResponse, "");
		
	}
	
	setCommandFinished = false;
	
	#ifdef DEBUG_ON
	Serial.println("Saving...\n");
	#endif
	
	while(!setCommandFinished) {
	
		serialWiFly.write("save\r");
		delay(300);
	
		waitResponseFromWiFly();
	
		#ifdef DEBUG_ON
		DEBUG_PRINT(wiFlyResponse);
		#endif
		
		setCommandFinished = compareResponseFromWiFly(wiFlyResponse, "Storing");
		strcpy(wiFlyResponse, "");
		
	}
	
	while(!exitWiFlyCommandMode());

}


void joinNetworkUsingWiFly(char *network) {

	while(!enterWiFlyCommandMode());
	
	strcpy(tempCommand, "join ");
	strcat(tempCommand, network);
	strcat(tempCommand, "\r");
	
	#ifdef DEBUG_ON
	Serial.print("Command: ");
	Serial.println(tempCommand);
	#endif
	
	serialWiFly.write(tempCommand);
	delay(300);
		
	waitResponseFromWiFly();
		
	#ifdef DEBUG_ON
	DEBUG_PRINT(wiFlyResponse);
	Serial.println("Joining network...");
	#endif
	
	strcpy(wiFlyResponse, "");

	while(!exitWiFlyCommandMode());
	
}


boolean checkWiFlyConnection() {

	boolean wiFlyConnected = false;
	
	while(!enterWiFlyCommandMode());
		
	serialWiFly.write("show c\r");
	delay(300);
		
	waitResponseFromWiFly();
	
	#ifdef DEBUG_ON
	DEBUG_PRINT(wiFlyResponse);
	#endif
		
	if(wiFlyResponse[2] == '3') {
		wiFlyConnected = true;
		#ifdef DEBUG_ON
		Serial.println("WiFly is associated!\n");
		#endif
	}	
		
	strcpy(wiFlyResponse, "");	
	
	while(!exitWiFlyCommandMode());
	
	return wiFlyConnected;

}


/*boolean scanNetworksUsingWiFly() {

	unsigned short networksCompareCount = 0;
	unsigned short tokenCount = 0;
	unsigned short savedNetworksCount = 0;
	
	char *stringPointer;
	char *stringPointer1;
	char *stringPointer2;
	
	unsigned long waitStartTime;
	unsigned long waitDurationTime;
	
	unsigned int scannedNetworksCount = 0;
	
	while(!enterWiFlyCommandMode());
	
	serialWiFly.write("scan\r");
	delay(500);
	
	waitStartTime = millis();

	while(1) { 
	
		if(strlen(wiFlyResponse) < MAX_RESPONSE_LENGTH) {
			if(serialWiFly.available() > 0) {
				charToString[0] = serialWiFly.read();
				strcat(wiFlyResponse, charToString);
			}
			
			waitDurationTime = millis() - waitStartTime;
			
			if(waitDurationTime > NETWORKS_SCAN_TIMEOUT) {
				#ifdef DEBUG_ON
				DEBUG_PRINT(wiFlyResponse);
				#endif
				break;
			}
		} else {
			#ifdef DEBUG_ON
			DEBUG_PRINT(wiFlyResponse);
			#endif
			break;
		}
	}
	
	if(wiFlyResponse[23] == '\r')
		scannedNetworksCount = wiFlyResponse[22] - '0';
	else
		scannedNetworksCount = ((wiFlyResponse[22] - '0') * 10 ) + (wiFlyResponse[23] - '0');
	
	#ifdef DEBUG_ON
	Serial.print("Number of Networks Found: ");
	Serial.println(scannedNetworksCount);
	Serial.println();
	#endif
	
	if(scannedNetworksCount == 0) {
		#ifdef DEBUG_ON
		Serial.println("No network match found.");
		Serial.println("Networks Scan Finished\n");
		#endif
		strcpy(wiFlyResponse, "");
		while(!exitWiFlyCommandMode());
		return false;
	}
	
	stringPointer = strstr(wiFlyResponse, "SCAN");
	stringPointer1 = stringPointer;
	stringPointer = strchr((char *)(stringPointer1), '\n');
	if(stringPointer != NULL) {
		*stringPointer = 0;
	}
	
	// Access first line of scanned networks.
	while(networksCompareCount < scannedNetworksCount) {
		
		stringPointer1 = stringPointer + 1;
		stringPointer = strchr((char *)(stringPointer1), '\n');
		if(stringPointer != NULL) {
			*stringPointer = 0;
			
			// Access SSID of each scanned network.
			stringPointer2 = strtok((char *)stringPointer1,",\r\n");
			while (stringPointer2 != NULL) {
				tokenCount++;
				if(tokenCount == 9) {
					//scannedNetwork = stringPointer2;
					strcpy(scannedNetwork, stringPointer2);
					#ifdef DEBUG_ON
					Serial.print("Scanned Network ");
					Serial.print(networksCompareCount + 1);
					Serial.print(": ");
					Serial.println(scannedNetwork);
					#endif
					while(savedNetworksCount < NUMBER_SAVED_SSID) {
						#ifdef DEBUG_ON
						Serial.print("Network Compare: ");
						Serial.println(SAVED_SSID[savedNetworksCount]);
						#endif
						
						// Scanned network matched on of the known networks.
						if(strcmp(scannedNetwork, SAVED_SSID[savedNetworksCount]) == 0) {
							//strcpy(matchedPASSWORD, SAVED_PASSWORD[savedNetworksCount]);
							savedNetworksCount = 0;
							
							#ifdef DEBUG_ON
							Serial.println("Network match found.");
							Serial.print("Matched SSID: ");
							Serial.println(scannedNetwork);
							//Serial.print("Matched PASSWORD: ");
							//Serial.println(matchedPASSWORD);
							Serial.println("Networks Scan Finished\n");
							#endif
							
							strcpy(wiFlyResponse, "");
							while(!exitWiFlyCommandMode());
							
							return true;
						}
						savedNetworksCount++;
					
					}
					
					tokenCount = 0;
					savedNetworksCount = 0;
				}
				stringPointer2 = strtok(NULL, ",\r\n");
			}
			tokenCount = 0;
			
		}
		networksCompareCount++;
	}
	
	#ifdef DEBUG_ON
	Serial.println("No network match found.");
	Serial.println("Networks Scan Finished\n");
	#endif
	
	strcpy(wiFlyResponse, "");
	while(!exitWiFlyCommandMode());
	
	// No networks found.
	return false;

}*/


void connectToNetworkUsingWiFly() {

	//while(!scanNetworksUsingWiFly());
	
	sendSetCommandToWiFly("set ip dhcp 1\r");
	sendSetCommandToWiFly("set wlan join 0\r");
	//sendSetCommandToWiFly("set wlan auth 0\r");
	//sendSetCommandToWiFly("set wlan auth 8\r");
	sendSetCommandToWiFly("set wlan auth 4\r");
	
	// Set SSID.
	strcpy(tempCommand, "set wlan ssid ");
	//strcat(tempCommand, scannedNetwork);
	strcat(tempCommand, SAVED_SSID[0]);
	strcat(tempCommand, "\r");
	#ifdef DEBUG_ON
	Serial.print("Temp SSID: ");
	Serial.println(tempCommand);
	#endif
	sendSetCommandToWiFly(tempCommand);
	
	// Set PASSWORD.
    //strcpy(tempCommand, "set wlan key ");
	strcpy(tempCommand, "set wlan passphrase ");
	//strcat(tempCommand, matchedPASSWORD);
	strcat(tempCommand, SAVED_PASSWORD[0]);
	strcat(tempCommand, "\r");
	#ifdef DEBUG_ON
	Serial.print("Temp PASSWORD: ");
	Serial.println(tempCommand);
	#endif
	sendSetCommandToWiFly(tempCommand);

	while(!isConnected) {
		joinNetworkUsingWiFly(SAVED_SSID[0]);
		isConnected = checkWiFlyConnection();
	}

}

