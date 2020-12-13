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
#include "DHTesp.h"
#include "Button2.h"

DHTesp dht;
#define PIN_HUMIDIFIER 16
#define PIN_BUTTON 13
Button2 button = Button2(PIN_BUTTON);
#define LOG_D(fmt, ...)   printf_P(PSTR(fmt "\n") , ##__VA_ARGS__);
float humidity_threshold_value = 60;
void setup() {
	Serial.begin(115200);
	wifi_connect(); // in wifi_info.h
	//homekit_storage_reset(); // to remove the previous HomeKit pairing storage when you first run this new HomeKit example
	my_homekit_setup();
}

void loop() {
	my_homekit_loop();
  button.loop();
	delay(10);
}

//==============================
// HomeKit setup and loop
//==============================

// access your HomeKit characteristics defined in my_accessory.c
extern "C" homekit_server_config_t config;
//extern "C" homekit_characteristic_t cha_switch_on;
extern "C" homekit_characteristic_t cha_humidity;
extern "C" homekit_characteristic_t cha_active;
extern "C" homekit_characteristic_t cha_humidity_threshold;
extern "C" homekit_characteristic_t cha_current_humidifier;
extern "C" homekit_characteristic_t cha_target_humidifier;

static uint32_t next_heap_millis = 0;

uint8_t cha_current_humidifier_getter( ){
  Serial.println(cha_current_humidifier.value.uint8_value);
  return cha_current_humidifier.value.uint8_value;
}

//Called when the switch value is changed by iOS Home APP  修改
void cha_active_setter(const homekit_value_t value) {
	uint8_t active = value.uint8_value;   
	cha_active.value.uint8_value = active;	//sync the value
  if(active == 1){
	  digitalWrite(PIN_HUMIDIFIER,HIGH);
    uint8 current_humidifier = 2;    //0 inactive  1 idle  2 humidifying  3 dehumidifying
    cha_current_humidifier.value.uint8_value = current_humidifier;
    homekit_characteristic_notify(&cha_current_humidifier, cha_active.value);
  }
  else{
    digitalWrite(PIN_HUMIDIFIER,LOW);
    uint8 current_humidifier = 0;    //0 inactive  1 idle  2 humidifying  3 dehumidifying
    cha_current_humidifier.value.uint8_value = current_humidifier;
    homekit_characteristic_notify(&cha_current_humidifier, cha_active.value);
  }
}

// void cha_target_humidifier_state_setter(const homekit_value_t value){
//   uint8_t target_state = value.uint8_value;
//   cha_target_humidifier.value.uint8_value = target_state;
//   Serial.println(target_state);
//   if(target_state == 1){
//     digitalWrite(PIN_HUMIDIFIER,HIGH);
//     uint8 current_humidifier = 2;    //0 inactive  1 idle  2 humidifying  3 dehumidifying
//     cha_current_humidifier.value.uint8_value = current_humidifier;
//     homekit_characteristic_notify(&cha_current_humidifier, cha_active.value);
//   }
//   else{
//     digitalWrite(PIN_HUMIDIFIER,LOW);
//     uint8 current_humidifier = 0;    //0 inactive  1 idle  2 humidifying  3 dehumidifying
//     cha_current_humidifier.value.uint8_value = current_humidifier;
//     homekit_characteristic_notify(&cha_current_humidifier, cha_active.value); 
//   }
// }


// void cha_current_humidifier_state_setter(const homekit_value_t value){
//   uint8_t current_state = value.uint8_value;
//   cha_current_humidifier.value.uint8_value = current_state;
//   Serial.println(current_state);
//   if(current_state == 1){
//     digitalWrite(PIN_HUMIDIFIER,HIGH);
//     uint8 current_humidifier = 2;    //0 inactive  1 idle  2 humidifying  3 dehumidifying
//     cha_current_humidifier.value.uint8_value = current_humidifier;
//     homekit_characteristic_notify(&cha_current_humidifier, cha_active.value);
//   }
//   else{
//     digitalWrite(PIN_HUMIDIFIER,LOW);
//     uint8 current_humidifier = 0;    //0 inactive  1 idle  2 humidifying  3 dehumidifying
//     cha_current_humidifier.value.uint8_value = current_humidifier;
//     homekit_characteristic_notify(&cha_current_humidifier, cha_active.value); 
//   }
// }


void cha_humidity_threshold_setter(const homekit_value_t value){
  float humidity_threshold = value.float_value;
  cha_humidity_threshold.value.float_value = humidity_threshold;
  humidity_threshold_value = humidity_threshold;
  Serial.println(humidity_threshold);
}

void my_homekit_setup() {
	pinMode(PIN_HUMIDIFIER, OUTPUT);
	digitalWrite(PIN_HUMIDIFIER, LOW);
  button.setPressedHandler(button_press_interrupt);
  dht.setup(4, DHTesp::DHT11);
//	cha_switch_on.setter = cha_switch_setter;
  cha_active.setter = cha_active_setter;
  cha_humidity_threshold.setter = cha_humidity_threshold_setter;
  // cha_target_humidifier.setter = cha_target_humidifier_state_setter;
  // cha_current_humidifier.setter = cha_current_humidifier_state_setter;
	arduino_homekit_setup(&config);
  uint8_t active = 0;
  cha_active.value.uint8_value= active;
  homekit_characteristic_notify(&cha_active, cha_active.value);
}

void my_homekit_loop() {
	arduino_homekit_loop();
	const uint32_t t = millis();
	if (t > next_heap_millis) {
		// show heap info every 5 seconds
    my_homekit_report();
		next_heap_millis = t + 5 * 1000;
		LOG_D("Free heap: %d, HomeKit clients: %d",
				ESP.getFreeHeap(), arduino_homekit_connected_clients_count());
    cha_current_humidifier_getter();
	}
}

void my_homekit_report() {
   float humidity = dht.getHumidity();
//  cha_temperature.value.float_value = t;
//  homekit_characteristic_notify(&cha_temperature, cha_temperature.value);
  cha_humidity.value.float_value = humidity;
  homekit_characteristic_notify(&cha_humidity, cha_humidity.value);
 if(humidity > humidity_threshold_value && cha_current_humidifier_getter() != 0){
    digitalWrite(PIN_HUMIDIFIER, LOW);
 }
 if(humidity < humidity_threshold_value - 10 && cha_current_humidifier_getter() != 0){
    digitalWrite(PIN_HUMIDIFIER, HIGH);
 }
}

int random_value(int min, int max) {
  return min + random(max - min);
}

void button_press_interrupt(Button2& btn){
  if(cha_current_humidifier_getter() != 0){
    digitalWrite(PIN_HUMIDIFIER,LOW);
    uint8_t active = 0;
    cha_active.value.uint8_value= active;
    homekit_characteristic_notify(&cha_active, cha_active.value);
    uint8 current_humidifier = 0;    //0 inactive  1 idle  2 humidifying  3 dehumidifying
    cha_current_humidifier.value.uint8_value = current_humidifier;
    homekit_characteristic_notify(&cha_current_humidifier, cha_active.value);
  }
  else{
	  digitalWrite(PIN_HUMIDIFIER,HIGH);
    uint8_t active = 1;
    cha_active.value.uint8_value= active;
    homekit_characteristic_notify(&cha_active, cha_active.value);
    uint8 current_humidifier = 2;    //0 inactive  1 idle  2 humidifying  3 dehumidifying
    cha_current_humidifier.value.uint8_value = current_humidifier;
    homekit_characteristic_notify(&cha_current_humidifier, cha_active.value);
  }
}
