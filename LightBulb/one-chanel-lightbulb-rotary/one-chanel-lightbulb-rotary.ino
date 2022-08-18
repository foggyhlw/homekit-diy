/*
 * Example05_WS2812_Neopixel.ino
 *
 *  Created on: 2020-10-01
 *      Author: Juergen Fink
 *	Thanks to all the other helpful people commenting here.
 *
 * This example allows to change brightness and color of a connected neopixel strip/matrix
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
#include "Button2.h"
#include "ESPRotary.h"
#include <EEPROM.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager

#define LOG_D(fmt, ...)   printf_P(PSTR(fmt "\n") , ##__VA_ARGS__);
#define MAP_100_2_255(val) map(val,0,100,0,255)
#define MAIN_LED_PIN 2  //D4
#define INPUT_BUTTON_PIN 14   //D5
#define ROTARY_PIN1	 12  //D6
#define ROTARY_PIN2	13  //D7
#define CLICKS_PER_STEP 3
#define RESET_HK_PERIOD 10000
#define WIFI_CHECK_PERIOD 10000

#define RESET_HK_CLICK 4   // used to reset homekit pairing information if button is clicked 4times after startup within 10s

int reset_click_number = 0;
int wifi_reconnect_count = 0;
unsigned long time_now = 0;
unsigned long time_now1 = 0;
bool first_start_loop = true;
bool is_on = false;

float current_brightness =  50;

WiFiManager wm;

Button2 button;
ESPRotary rotary = ESPRotary(ROTARY_PIN1, ROTARY_PIN2, CLICKS_PER_STEP);

void setup() {
  rotary.setRightRotationHandler(RightRotationHandler);
  rotary.setLeftRotationHandler(LeftRotationHandler);
  button.begin(INPUT_BUTTON_PIN);
  button.setLongClickTime(500);  // set longclick time to 500ms 
	Serial.begin(115200);
	// wifi_connect(); // in wifi_info.h
  pinMode(MAIN_LED_PIN, OUTPUT);
  button.setClickHandler(single_click_handler);
  button.setLongClickHandler(long_click_handler);
  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP 
  // wm.resetSettings();
  wm.setConfigPortalBlocking(false);
  wm.setConfigPortalTimeout(60);
  if(wm.autoConnect("AutoConnectAP")){
        Serial.println("connected...yeey :)");
    }
    else {
        Serial.println("Configportal running");
    }

	my_homekit_setup();
}

void loop() {
  if(first_start_loop){
    if(millis() > time_now + RESET_HK_PERIOD){
      first_start_loop = false;
      time_now = millis();
      if(reset_click_number >= 4){
        analogWrite(MAIN_LED_PIN,0);
        delay(1000);
        analogWrite(MAIN_LED_PIN,100);
        delay(1000);
        analogWrite(MAIN_LED_PIN,0);
        homekit_storage_reset();
        ESP.restart();
      }
    }
  }
  if(millis() > time_now1 + WIFI_CHECK_PERIOD){
    time_now1 = millis();
    if(WiFi.isConnected()){
      wifi_reconnect_count = 0;
    }
    else{
      wifi_reconnect_count++;
      Serial.print("Wifi connection is lost!\n");
    }
    
  }
  if(wifi_reconnect_count >= 60){
    Serial.print("No WiFi, try restart!\n");
    ESP.restart();
  }
  wm.process();
  // if(WiFi.isConnected()){
  my_homekit_loop();
  // }
  rotary.loop();
  button.loop();
	delay(10);
}

//==============================
// HomeKit setup and loop
//==============================

// access your HomeKit characteristics defined in my_accessory.c

extern "C" homekit_server_config_t accessory_config;
extern "C" homekit_characteristic_t cha_on;
extern "C" homekit_characteristic_t cha_bright;
extern "C" homekit_characteristic_t name;

static uint32_t next_heap_millis = 0;

void my_homekit_setup() {
uint8_t mac[WL_MAC_ADDR_LENGTH];
	WiFi.macAddress(mac);
	int name_len = snprintf(NULL, 0, "%s_%02X%02X%02X",
			name.value.string_value, mac[3], mac[4], mac[5]);
	char *name_value = (char*) malloc(name_len + 1);
	snprintf(name_value, name_len + 1, "%s_%02X%02X%02X",
			name.value.string_value, mac[3], mac[4], mac[5]);
	name.value = HOMEKIT_STRING_CPP(name_value);

  cha_on.setter = set_on;
  cha_bright.setter = set_bright;
	arduino_homekit_setup(&accessory_config);
}

void my_homekit_loop() {
	arduino_homekit_loop();
	// const uint32_t t = millis();
	// if (t > next_heap_millis) {
	// 	// show heap info every 5 seconds
	// 	next_heap_millis = t + 5 * 1000;
	// 	LOG_D("Free heap: %d, HomeKit clients: %d",
	// 			ESP.getFreeHeap(), arduino_homekit_connected_clients_count());
	// }
}

void set_on(const homekit_value_t v) {
    bool on = v.bool_value;
    cha_on.value.bool_value = on; //sync the value

    if(on) {
        is_on = true;
        Serial.println("On");
    } else  {
        is_on = false;
        Serial.println("Off");
    }
    // updateColor();
    updateBrightness();
}

void set_bright(const homekit_value_t v) {
    Serial.println("set_bright");
    int bright = v.int_value;
    cha_bright.value.int_value = bright; //sync the value

    current_brightness = bright;
    updateBrightness();
    // updateColor();
}


void updateBrightness(){
  if(is_on){
    analogWrite(MAIN_LED_PIN, MAP_100_2_255(current_brightness));
  }
  else if(!is_on){
    analogWrite(MAIN_LED_PIN, 0);
  }
}

void single_click_handler(Button2& btn){
  Serial.print("click!\n");
  reset_click_number++;
  if(is_on){     // turn off when light is on
    analogWrite(MAIN_LED_PIN, 0);
    is_on = false;
  }
  else if(!is_on){  // turn on when light is off and set to last brightness
    is_on = true;
  }
  cha_on.value.bool_value = is_on;
  homekit_characteristic_notify(&cha_on, cha_on.value);
  updateBrightness();
}

void long_click_handler(Button2& btn){
  Serial.print("long click! \n");
}


void RightRotationHandler(ESPRotary& r) {
  Serial.println(r.getPosition());
  if(current_brightness<100){
    current_brightness += 10;
  }
  else{
    current_brightness = 100;
  }
  cha_bright.value.int_value = current_brightness; //sync the value
  homekit_characteristic_notify(&cha_bright, cha_bright.value);
  updateBrightness();
}

void LeftRotationHandler(ESPRotary& r) {
  Serial.println(r.getPosition());
  if(current_brightness>10){
    current_brightness -= 10;
  }
  else{
    current_brightness = 10;
  }
  cha_bright.value.int_value = current_brightness; //sync the value
  homekit_characteristic_notify(&cha_bright, cha_bright.value);
  updateBrightness();
}