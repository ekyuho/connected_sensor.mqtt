//
//    FILE: wifi.ino
//  AUTHOR: Kyuho Kim (ekyuho@gmail.com)
// CREATED: September 4, 2017
// Released to the public domain
//
#ifndef MyWifi_h
#define MyWifi_h

#include "Arduino.h"

class MyWifi 
{
  public:
    MyWifi(void);
  	void begin(void);
  	void configure(String);
    bool connect_ap(void);
	void print(void);
    bool tcp;
    bool ack;
    char ssid[32];
    char password[32];
	bool isConnected(void);
  private:
	void parse(String);
	void scan(void);
};
#endif
