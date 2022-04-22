//
//    FILE: server.ino
//  AUTHOR: Kyuho Kim (ekyuho@gmail.com)
// CREATED: November 20, 2017
// Released to the public domain
//
#ifndef Sogang_h
#define Sogang_h

#include "Arduino.h"

#ifdef ESP32
#include <HTTPClient.h>
#else
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#endif

#include <PubSubClient.h>
WiFiClient client;

class Sogang
{
public:
	int user;
	String apikey;
	long unsigned serial=0;
	char *ssid;
	String topic;
	int _port;
	String clientId = "";
	String _host;
	int _seq = 0;
	String netstat="init";
	String version;
	PubSubClient *mqttClient;

		
	const char* mqttUser = NULL;
	const char* mqttPassword = NULL;
	char tb1[1024], tb2[1024];

	Sogang(void) {
		_host = "damoa.io";
		_port=1883;
		user = 0;
		apikey = "";
	}

	void begin(char *_ssid, String _version) {
		version=_version;
		ssid= _ssid;
		topic= "dust/volunteer/";
		Serial.printf("\n sg.begin(%s, %s)", ssid, topic.c_str());
		mqttClient = new PubSubClient(client);
		mqttClient->setServer(_host.c_str(), _port);

		byte mac[6];
		char macStr[18] = {0};
		WiFi.macAddress(mac);
		sprintf(macStr, "%02x%02x%02x%02x%02x%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
		apikey = String(macStr);
		Serial.printf("\n got apikey= %s", apikey.c_str());
	}

	const char* status2str(int code) {
		switch(code) {
			case -4 : return("MQTT_CONNECTION_TIMEOUT - the server didn't respond within the keepalive time");
			case -3 : return("MQTT_CONNECTION_LOST - the network connection was broken");
			case -2 : return("MQTT_CONNECT_FAILED - the network connection failed");
			case -1 : return("MQTT_DISCONNECTED - the client is disconnected cleanly");
			case 0 : return("MQTT_CONNECTED - the client is connected");
			case 1 : return("MQTT_CONNECT_BAD_PROTOCOL - the server doesn't support the requested version of MQTT");
			case 2 : return("MQTT_CONNECT_BAD_CLIENT_ID - the server rejected the client identifier");
			case 3 : return("MQTT_CONNECT_UNAVAILABLE - the server was unable to accept the connection");
			case 4 : return("MQTT_CONNECT_BAD_CREDENTIALS - the username/password were rejected");
			case 5 : return("MQTT_CONNECT_UNAUTHORIZED - the client was not authorized to connect");
		}
		return("unknown");
	}

	int connect(){ //ZZ
	
		if (!mqttClient->connected()) {
			Serial.printf("\n connecting mqtt server...");
			for (int i=0;i<8;i++) clientId += String(random(16), HEX);
			clientId = String("pub")+"_"+clientId;
			if (mqttClient->connect(clientId.c_str(), mqttUser, mqttPassword )) {
				netstat = "mqtt ok";
				mqttClient->setKeepAlive(60);
				mqttClient->setSocketTimeout(60);
				Serial.printf("connected as %s", clientId.c_str());
				sprintf(tb2, "%s%s/ack", topic.c_str(), apikey.c_str());
				Serial.printf("\nmqtt subscribed %s", tb2);
				mqttClient->subscribe(tb2);
				#ifdef USE_BLUETOOTH
				SerialBT.printf("\ninit mqtt ok");
				#endif
			} else {
				netstat = "mqtt fail";
				Serial.printf("\nmqtt client_id %s, failed with %d %s", clientId.c_str(), mqttClient->state(), status2str(mqttClient->state()));
				Serial.printf("\n check network connection. ");
				return 0;
			}
		}
		return 1;
	}
	
	void callback(char* rtopic, byte* _payload, unsigned int length) {
		String payload = "";
		//Serial.printf("\n gogo len=%d ", length);
		for (int i=0;i<length;i++) payload +=String((char)_payload[i]);
		Serial.printf("\n got callback %s %s", rtopic, payload.c_str());
		if (payload.startsWith("X-ACK:")) {
			String u_s = payload.substring(6, 12);
			Serial.printf("\n got %s", u_s);
			user = u_s.toInt();
			netstat = "mqtt ok";
		} else {
			Serial.printf("\n error");
		}
	}

	void send(int pm25, int pm10) {
		sprintf(tb1, "{\"mac\":\"%s\",\"ssid\":\"%s\",\"rssi\":%d,\"topic\":\"%s\",\"version\":\"%s\",\"serial\":%lu,\"pm25\":\"%d\",\"pm10\":\"%d\"}", apikey.c_str(), ssid, WiFi.RSSI(), topic.c_str(), version.c_str(), serial++,pm25,pm10);
		if (strlen(tb1)>MQTT_MAX_PACKET_SIZE) {
			Serial.printf("\n pubsub buffer overflow");
		}

		connect();
		mqttClient->publish((topic+apikey+"/data").c_str(), tb1);
		Serial.printf("\n%s %s\n", (topic+apikey+"/data").c_str(), tb1);


	#ifdef XX
		Serial.println(s);
		http.begin(s);

		int httpCode = http.GET();
		String line="";
		if(httpCode == HTTP_CODE_OK) {
			line = http.getString();
			Serial.println(line);
		}
		http.end();
		
		bool ack = false;
		//Serial.println("s1\n");
		if (line.startsWith("X-ACK:")) {
			//Serial.println(line);
			ack = true;
			if (!user) {
				// X-ACK: {"u":212010,"s":0,"i":"0D-1,1D-1","f":"3","time":"2022-04-20T03:09:50","ip":"210.90.237.204"}
				String u_s = line.substring(8, 11);
				String user_s = line.substring(12, 18);
				if (u_s != "\"u\"") {
					Serial.println("Format Error: "+ line);
				} else {
					Serial.printf("got %s\n", user_s.c_str());
					user = atoi(user_s.c_str());
				}
			}
		}

	  Serial.println("about returning sg.send()\n");
	  if (!ack) return "no ACK";
	  return "";

		if (!client.connect(_host.c_str(), _port)) {
			Serial.printf("\n failed to connect %s:%d", _host.c_str(), _port);
			return "no ACK";
		}
		//Serial.printf("\n tcp connected %s", _host.c_str());
		client.print(String("GET ")+_path+ payload +" HTTP/1.1\r\n" +
			 "Host: " + _host + "\r\n" +
			 "Connection: close\r\n" +
			 "\r\n"
			);

		bool ack = false;
		//Serial.printf("\n waiting data");
		while (client.connected() || client.available()) {
			//Serial.printf("\n expecting data...");
			if (client.available()){
				String line = client.readStringUntil('\n');
				Serial.printf("\n %s", line.c_str());
				if (line.startsWith("X-ACK:")) {
					//Serial.println(line);
					ack = true;
					if (!user) {
						// X-ACK: {"u":212010,"s":0,"i":"0D-1,1D-1","f":"3","time":"2022-04-20T03:09:50","ip":"210.90.237.204"}
						String u_s = line.substring(8, 11);
						String user_s = line.substring(12, 18);
						if (u_s != "\"u\"") {
							Serial.println("Format Error: "+ line);
						} else {
							Serial.printf("got %s\n", user_s.c_str());
							user = atoi(user_s.c_str());
						}
					}
				}
			}
		}
		//Serial.printf(" done data");
		client.stop();
		Serial.println();
		if (!ack) return "no ACK";
		return "";
	#endif
	}

};
#endif
