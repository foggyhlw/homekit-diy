/*
 * switch.ino
 *
 *  Created on: 2020-05-15
 *      Author: Mixiaoxiao (Wang Bin)
 *
 * HAP section 8.38 Switch
 * An accessory contains a switch.
 *
 * This example shows how to:
 * 1. define a switch accessory and its characteristics (in my_accessory.c).
 * 2. get the switch-event sent from iOS Home APP.
 * 3. report the switch value to HomeKit.
 *
 * You should:
 * 1. read and use the Example01_TemperatureSensor with detailed comments
 *    to know the basic concept and usage of this library before other examples。
 * 2. erase the full flash or call homekit_storage_reset() in setup()
 *    to remove the previous HomeKit pairing storage and
 *    enable the pairing with the new accessory of this new HomeKit example.
 */

#include <Arduino.h>
#include <arduino_homekit_server.h>
//#include "wifi_info.h"
#include <WiFiManager.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>

#define LOG_D(fmt, ...)   printf_P(PSTR(fmt "\n") , ##__VA_ARGS__);

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFiManager wm;
  //homekit_storage_reset(); // to remove the previous HomeKit pairing storage when you first run this new HomeKit example
  bool res;
    // res = wm.autoConnect(); // auto generated AP name from chipid
    res = wm.autoConnect("AutoConnectAP"); // anonymous ap
    //res = wm.autoConnect("AutoConnectAP","password"); // password protected ap

    if(!res) {
        Serial.println("Failed to connect");
        // ESP.restart();
    } 
    else {
        //if you get here you have connected to the WiFi    
        Serial.println("connected...yeey :)");
    }

	//wifi_connect(); // in wifi_info.h
	my_homekit_setup();
}

int loopcount = 0;
int num = 100;
void loop() {
  if(loopcount > 100){
      num++;
      loopcount=0;
      my_homekit_loop();
    }
    loopcount++;
    delay(10);
}

//function to put number on display
//void printNumber(int v) {
//    int ones;
//    int tens;
//    int hundreds;
//    boolean negative;  
//
//    if(v < -999 || v > 999) 
//       return;
//    if(v<0) {
//        negative=true;
//        v=v*-1;
//    }
//    ones=v%10;
//    v=v/10;
//    tens=v%10;
//    v=v/10;
//    hundreds=v;     
//    if(negative) {
//       //print character '-' in the leftmost column 
//       lc.setChar(0,3,'-',false);
//    }
//    else {
//       //print a blank in the sign column
//       lc.setChar(0,3,' ',false);
//    }
//    //Now print the number digit by digit
//    if(hundreds !=0){
//    lc.setDigit(0,2,(byte)hundreds,false);
//    }
//    lc.setDigit(0,1,(byte)tens,false);
//    lc.setDigit(0,0,(byte)ones,false);
//}


//==============================
// HomeKit setup and loop
//==============================

// access your HomeKit characteristics defined in my_accessory.c
extern "C" homekit_server_config_t config;
extern "C" homekit_characteristic_t cha_switch_on1;
extern "C" homekit_characteristic_t cha_switch_on2;
extern "C" homekit_characteristic_t cha_switch_on3;
extern "C" homekit_characteristic_t cha_switch_on4;
static uint32_t next_heap_millis = 0;

#define PIN_SWITCH1 13
#define PIN_SWITCH2 16
#define PIN_SWITCH3 15
#define PIN_SWITCH4 5
//Called when the switch value is changed by iOS Home APP
void cha_switch_on_setter1(const homekit_value_t value) {
	bool on = value.bool_value;
	cha_switch_on1.value.bool_value = on;	//sync the value
	LOG_D("Switch: %s", on ? "ON" : "OFF");
	digitalWrite(PIN_SWITCH1, on ? LOW : HIGH);
}
void cha_switch_on_setter2(const homekit_value_t value) {
  bool on = value.bool_value;
  cha_switch_on2.value.bool_value = on; //sync the value
  LOG_D("Switch: %s", on ? "ON" : "OFF");
  digitalWrite(PIN_SWITCH2, on ? LOW : HIGH);
}
void cha_switch_on_setter3(const homekit_value_t value) {
  bool on = value.bool_value;
  cha_switch_on3.value.bool_value = on; //sync the value
  LOG_D("Switch: %s", on ? "ON" : "OFF");
  digitalWrite(PIN_SWITCH3, on ? LOW : HIGH);
}
void cha_switch_on_setter4(const homekit_value_t value) {
  bool on = value.bool_value;
  cha_switch_on4.value.bool_value = on; //sync the value
  LOG_D("Switch: %s", on ? "ON" : "OFF");
  digitalWrite(PIN_SWITCH4, on ? LOW : HIGH);
}
void my_homekit_setup() {
	pinMode(PIN_SWITCH1, OUTPUT);
	digitalWrite(PIN_SWITCH1, HIGH);
  pinMode(PIN_SWITCH2, OUTPUT);
  digitalWrite(PIN_SWITCH2, HIGH);
   pinMode(PIN_SWITCH3, OUTPUT);
  digitalWrite(PIN_SWITCH3, HIGH);
   pinMode(PIN_SWITCH4, OUTPUT);
  digitalWrite(PIN_SWITCH4, HIGH);
	//Add the .setter function to get the switch-event sent from iOS Home APP.
	//The .setter should be added before arduino_homekit_setup.
	//HomeKit sever uses the .setter_ex internally, see homekit_accessories_init function.
	//Maybe this is a legacy design issue in the original esp-homekit library,
	//and I have no reason to modify this "feature".
	cha_switch_on1.setter = cha_switch_on_setter1;
  cha_switch_on2.setter = cha_switch_on_setter2;
  cha_switch_on3.setter = cha_switch_on_setter3;
  cha_switch_on4.setter = cha_switch_on_setter4;
	arduino_homekit_setup(&config);

	//report the switch value to HomeKit if it is changed (e.g. by a physical button)
	//bool switch_is_on = true/false;
	//cha_switch_on.value.bool_value = switch_is_on;
	//homekit_characteristic_notify(&cha_switch_on, cha_switch_on.value);
}

void my_homekit_loop() {
  arduino_homekit_loop();
	const uint32_t t = millis();
	if (t > next_heap_millis) {
		// show heap info every 5 seconds  
		next_heap_millis = t + 5 * 1000;
		LOG_D("Free heap: %d, HomeKit clients: %d",
		ESP.getFreeHeap(), arduino_homekit_connected_clients_count());
	}
}
