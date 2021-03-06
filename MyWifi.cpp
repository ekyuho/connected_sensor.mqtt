//
//    FILE: wifi.ino
//  AUTHOR: Kyuho Kim (ekyuho@gmail.com)
// CREATED: September 4, 2017
// Released to the public domain
//
#include "MyWifi.h"
#ifdef ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif
#include <EEPROM.h>
#include <assert.h>

void oled_wifi_going(int, char*, char*);
void oled_no_wifi(void);


MyWifi::MyWifi(void) {
  tcp = true;
  ack = true;
}

void MyWifi::begin(void) { 
	EEPROM.begin(128);
	String cmd = "";
	for (int i=0; i< 128; i++) {
		char c = EEPROM.read(i);
		if (!c) break;
		cmd += String(c);
	}
	
	if (!cmd.startsWith("ssid,password=")) {
		cmd = "ssid,password=cookie2,0317137263";
		parse(cmd);
	}
	EEPROM.commit();

	parse(cmd);
}

void MyWifi::configure(String cmd) {
	parse(cmd);
	EEPROM.begin(128);
	for (int i=0; i<cmd.length(); i++)
		EEPROM.write(i, cmd.charAt(i));
	EEPROM.write(cmd.length(), 0);
	EEPROM.commit();
	
	Serial.println("restart in 5 seconds...");
	ESP.restart();
}

void MyWifi::print(){
	Serial.printf("\n ssid,password=%s,%s", ssid,password);
}

void MyWifi::parse(String cmd) {
String ssid_S = cmd.substring(14);
	int k = ssid_S.indexOf(",");
	String password_S = ssid_S.substring(k+1);
	ssid_S = ssid_S.substring(0,k);
	strcpy(ssid, ssid_S.c_str());
	strcpy(password, password_S.c_str());
}

void MyWifi::scan () {
    int n = WiFi.scanNetworks();
    Serial.println("Networks nearby");
    for (int i = 0; i < n; ++i) {
	  Serial.printf("%2d: %s (%d)\n", i+1, WiFi.SSID(i).c_str(), WiFi.RSSI(i));
    }
}

bool MyWifi::isConnected(void) {
	return WiFi.isConnected();
}

bool MyWifi::connect_ap() {
  if (WiFi.status() == WL_CONNECTED) return true;

  scan();
  int count = 30;
  Serial.printf("\nconnecting to %s(%s)\n", ssid, password);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    oled_wifi_going(count, ssid, password);
    if (!count--) {
      Serial.println("\nNO WIFI");
	    oled_no_wifi();
      return false;
    }
    extern void check_cmd();
    check_cmd();
  }
  Serial.printf("............\nConnected as ");
  Serial.println(WiFi.localIP());
  return true;
}
