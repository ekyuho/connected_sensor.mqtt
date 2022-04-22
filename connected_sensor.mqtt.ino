//
//    FILE: connected_sensor.ino
//  AUTHOR: Kyuho Kim (ekyuho@gmail.com)
// CREATED: September 4, 2017
// Last Modified: April 21, 2022
// Released to the public domain
//
#define VERSION "V3.1"
#include "MyWifi.h"
MyWifi mywifi;

#include "Sogang.h"
Sogang sg; 

#include <Wire.h>

#ifdef ESP32
HardwareSerial dustport(1); // ESP32 RX:13(TX of Dust Sensor)
#define DUSTRX 13
#define DUSTTX 5
#define ON digitalWrite(2, HIGH)
#define OFF digitalWrite(2, LOW)
#else
#include <SoftwareSerial.h>
SoftwareSerial dustport(D4, D0, false);  //RX, TX
#define ON 
#define OFF 
#endif

#include "Dust.h"
Dust dust;

#include "RunningMedian.h"
RunningMedian pm25s = RunningMedian(19);
RunningMedian pm10s = RunningMedian(19);

// "96 OLED
// D4:RX, D3:Data, D2: CLOCK, 
// 14:Data, 27:CLock for ESP32
const int INTERVAL = 60000;
String cmd = "";
int tick = 1;
unsigned long mark = 0, sec_mark = 0;
int missings = 0;
void oled_show(int, int, String);  

void got_dust(int pm25, int pm10) {
	Serial.printf("\n%02d: pm25,pm10=%d,%d", tick++,pm25, pm10);
	pm25s.add(pm25);
	pm10s.add(pm10);

	String msg = "";
	if (!mywifi.isConnected()) msg = "no WIFI";
	else if (!mywifi.tcp) msg = "no TCP";
	else if (!mywifi.ack) msg = "no ACK";
	else if (!sg.user) msg = "waiting";
	else msg = String(sg.user); 
	oled_show(pm25, pm10, msg);
}

void do_interval() {
	mywifi.connect_ap();

	int pm25 = int(pm25s.getMedian());
	yield();
	int pm10 = int(pm10s.getMedian());
	yield();
	if (pm25 > 999 || pm10>999) {
		Serial.print("dust value ?");
		Serial.print(pm25);
		Serial.print(" ");
		Serial.println(pm10);
	}
	yield();
	sg.send(pm25, pm10);
}

void got_cmd() {
	if (cmd.startsWith("user,ssid,password=")) mywifi.configure(cmd);
  if (cmd.startsWith("reboot")) ESP.restart(); 

	Serial.println("Command syntax:");
	Serial.println("  user,ssid,password=yourid,yourssid,yourpassword");
	mywifi.print();
	cmd = "";
}

void check_cmd() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n') got_cmd();
    else 
		if (c != '\r') cmd += String(c);
  }
}

void mycallback(char* rtopic, byte* payload, unsigned int length) {
	//Serial.printf("\n mycallback");
	sg.callback(rtopic, payload, length);
}

void setup() {
	Serial.begin(115200);
#ifdef ESP32
	dustport.begin(9600, SERIAL_8N1, DUSTRX, DUSTTX);
	pinMode(2, OUTPUT);
	Wire.begin(14,27);
#else
	dustport.begin(9600);
#endif
	Serial.printf("\n\nDust Sensor Box %s, 2022/4/19 by Valve God, Kyuho Kim", VERSION);
	oled_setup();

	mywifi.begin();
	if (mywifi.connect_ap()) Serial.println("Ready to receive sensor data. ");
	sg.begin(mywifi.ssid, VERSION);
	sg.mqttClient->setCallback(mycallback);
	pm25s.clear();
	pm10s.clear();
	mark = sec_mark = millis()+5000;
}
  
void loop() {
	unsigned long current = millis();

	if (current > mark) {
		mark = current + INTERVAL;
		do_interval();
		tick = 1;
		//Serial.println("done interval");
	}
	if (current > sec_mark) {
		sec_mark = current + 1000;
		if (missings++ > 5) {
			oled_waiting_dust(missings);
			Serial.printf("No data from dust sensor. check wiring.\n");
		}
	}
	yield();
	while (dustport.available() > 0) {
		ON;
		dust.do_char(dustport.read(), got_dust);
		OFF;
		missings = 0;
		yield();
	}
 
	check_cmd();
	sg.mqttClient->loop();
	yield();
}
