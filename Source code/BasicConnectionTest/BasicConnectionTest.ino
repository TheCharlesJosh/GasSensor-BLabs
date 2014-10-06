#include <LiquidCrystal.h>
#include <SPI.h>
#include <WiFly.h>

#define WIFI_SSID        "Alba Family"
#define WIFI_PASS        "casaalba"

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

void setup() {
  lcd.begin(16,2);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Hello.");
  lcd.setCursor(0,1);
  WiFly.begin();
  if (!WiFly.join(WIFI_SSID, WIFI_PASS)) {
     lcd.print("Not connected.");
  } else {
     lcd.print("Connected.");
  }
}

void loop(){
}
