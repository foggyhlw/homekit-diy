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
#include "wifi_info.h"
#include <neotimer.h>

#define PIN_A 14
#define PIN_B 12
#define PIN_C 13
#define PIN_RELAY 4

#define PLAY_PAUSE 0
#define MODE 1
#define FF 2
#define REW 3
#define BL_SKIP 4


#define LOG_D(fmt, ...)   printf_P(PSTR(fmt "\n") , ##__VA_ARGS__);

Neotimer mytimer = Neotimer(200); // 1 second timer

//==============================
// HomeKit setup and loop
//==============================

// access your HomeKit characteristics defined in my_accessory.c
extern "C" homekit_server_config_t config;
extern "C" homekit_characteristic_t cha_outlet1;
extern "C" homekit_characteristic_t cha_outlet2;
extern "C" homekit_characteristic_t cha_outlet3;
extern "C" homekit_characteristic_t cha_outlet4;
extern "C" homekit_characteristic_t cha_outlet5;

static uint32_t next_heap_millis = 0;

void config_pins(){
	pinMode(PIN_A, OUTPUT);
	digitalWrite(PIN_A, LOW);
	pinMode(PIN_B, OUTPUT);
	digitalWrite(PIN_B, LOW);
	pinMode(PIN_C, OUTPUT);
	digitalWrite(PIN_C, LOW);
	pinMode(PIN_RELAY, OUTPUT);
	digitalWrite(PIN_RELAY, LOW);
}
// choose mode resister
// 0:play/pause
// 1:mode
// 2:ff
// 3:rew
// 4:skip
// 5:A/B
void mode_select(const uint8 mode){
	switch(mode){
		case 0:
			digitalWrite(PIN_A,LOW);
			digitalWrite(PIN_B,LOW);
			digitalWrite(PIN_C,LOW);
			break;
		case 1:
			digitalWrite(PIN_A,HIGH);
			digitalWrite(PIN_B,LOW);
			digitalWrite(PIN_C,LOW);
			break;
		case 2:
			digitalWrite(PIN_A,LOW);
			digitalWrite(PIN_B,HIGH);
			digitalWrite(PIN_C,LOW);
			break;
		case 3:
			digitalWrite(PIN_A,HIGH);
			digitalWrite(PIN_B,HIGH);
			digitalWrite(PIN_C,LOW);
			break;
		case 4:
			digitalWrite(PIN_A,LOW);
			digitalWrite(PIN_B,LOW);
			digitalWrite(PIN_C,HIGH);
			break;
		default:
			Serial.printf("no matching options");
			break;
	}
}

//Called when the switch value is changed by iOS Home APP
void cha_outlet1_setter(const homekit_value_t value) {
	bool on = value.bool_value;
	if (on){
		mode_select(PLAY_PAUSE);
		digitalWrite(PIN_RELAY,HIGH);
		mytimer.start();
	}
	//reset cha_outlet value to false
		
	// cha_outlet1.value.bool_value = on;	//sync the value
	// LOG_D("Switch: %s", on ? "ON" : "OFF");
	// digitalWrite(PIN_SWITCH, on ? LOW : HIGH);
}
void cha_outlet2_setter(const homekit_value_t value) {
	bool on = value.bool_value;
	if (on){
		mode_select(MODE);
		digitalWrite(PIN_RELAY,HIGH);
		mytimer.start();
	}
}

void cha_outlet3_setter(const homekit_value_t value) {
	bool on = value.bool_value;
	if (on){
		mode_select(FF);
		digitalWrite(PIN_RELAY,HIGH);
		mytimer.start();
	}
}

void cha_outlet4_setter(const homekit_value_t value) {
	bool on = value.bool_value;
	if (on){
		mode_select(REW);
		digitalWrite(PIN_RELAY,HIGH);
		mytimer.start();
	}
}
// void cha_outlet5_setter(const homekit_value_t value) {
// 	bool on = value.bool_value;
// 	if (on){
// 		mode_select(BL_SKIP);
// 		digitalWrite(PIN_RELAY,HIGH);
// 		mytimer.start();
// 	}
// }

void reset_outlets(){
	cha_outlet1.value.bool_value = false ;
	homekit_characteristic_notify(&cha_outlet1, cha_outlet1.value);
	cha_outlet2.value.bool_value = false ;
	homekit_characteristic_notify(&cha_outlet2, cha_outlet2.value);
	cha_outlet3.value.bool_value = false ;
	homekit_characteristic_notify(&cha_outlet3, cha_outlet3.value);
	cha_outlet4.value.bool_value = false ;
	homekit_characteristic_notify(&cha_outlet4, cha_outlet4.value);
	// cha_outlet5.value.bool_value = false ;
	// homekit_characteristic_notify(&cha_outlet5, cha_outlet5.value);
}

void my_homekit_setup() {
	config_pins();
	//Add the .setter function to get the switch-event sent from iOS Home APP.
	//The .setter should be added before arduino_homekit_setup.
	//HomeKit sever uses the .setter_ex internally, see homekit_accessories_init function.
	//Maybe this is a legacy design issue in the original esp-homekit library,
	//and I have no reason to modify this "feature".
	cha_outlet1.setter = cha_outlet1_setter;
	cha_outlet2.setter = cha_outlet2_setter;
	cha_outlet3.setter = cha_outlet3_setter;
	cha_outlet4.setter = cha_outlet4_setter;
	// cha_outlet5.setter = cha_outlet5_setter;
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
		reset_outlets();
		// show heap info every 5 seconds
		next_heap_millis = t + 1 * 2000;
		// LOG_D("Free heap: %d, HomeKit clients: %d",
				// ESP.getFreeHeap(), arduino_homekit_connected_clients_count());

	}
}

void setup() {
	Serial.begin(115200);
	wifi_connect(); // in wifi_info.h
	//homekit_storage_reset(); // to remove the previous HomeKit pairing storage when you first run this new HomeKit example
	my_homekit_setup();
}

void loop() {
	if (mytimer.done()){
		digitalWrite(PIN_RELAY, LOW);
		mytimer.reset();
	}
	my_homekit_loop();
	delay(10);
}
