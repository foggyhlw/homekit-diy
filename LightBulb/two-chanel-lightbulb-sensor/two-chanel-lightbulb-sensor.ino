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
#include "wifi_info.h"
#include "Button2.h"
#include <Wire.h>
#include <BH1750.h>


#define LOG_D(fmt, ...)   printf_P(PSTR(fmt "\n") , ##__VA_ARGS__);
#define MAP_100_2_255(val) map(val,0,100,0,255)
#define MAIN_LED_PIN 4
#define AMBIENT_LED_PIN 2
#define INPUT_BUTTON_PIN 13
#define LUX_UPDATE_PERIOD  10000    //lux update time set to 10s
#define MOTION_PIN 5
// #define FANS_UPDATE_PERIOD 60000
unsigned long time_lux_update = 0;
// unsigned long time_now2 = 0;


int brightness_levels[] = {0,33,66,100};
int brightness_level_num = sizeof(brightness_levels)/sizeof(int);
int brightness_level_index = 1;
bool chanel_one_is_on = false;
bool chanel_two_is_on = false;
int chanel_one_current_brightness =  brightness_levels[brightness_level_index];
int chanel_two_current_brightness = 66;       //chanel tow did not have physical brightness button

bool last_motion = false;
float lux =0;   //to store lux value read from bh1750  homekit cha value is float
// float current_sat = 0.0;
// float current_hue = 0.0;
// int rgb_colors[3];
Button2 button;
BH1750 lightMeter(0x23);

void setup() {
  Wire.begin(12,14);
  lightMeter.begin();
  button.begin(INPUT_BUTTON_PIN);
  button.setLongClickTime(500);  // set longclick time to 500ms 
	Serial.begin(115200);
  pinMode(MAIN_LED_PIN, OUTPUT);
  pinMode(AMBIENT_LED_PIN, OUTPUT);
  digitalWrite(MAIN_LED_PIN, LOW);
  digitalWrite(AMBIENT_LED_PIN, LOW);
  pinMode(MOTION_PIN, INPUT);
	wifi_connect(); // in wifi_info.h
  button.setClickHandler(single_click_handler);
  button.setLongClickHandler(longClick_handler);
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

extern "C" homekit_server_config_t accessory_config;
extern "C" homekit_characteristic_t chanel_one_cha_on;
extern "C" homekit_characteristic_t chanel_one_cha_bright;
// extern "C" homekit_characteristic_t cha_sat;
// extern "C" homekit_characteristic_t cha_hue;
extern "C" homekit_characteristic_t chanel_two_cha_on;
extern "C" homekit_characteristic_t chanel_two_cha_bright;

extern "C" homekit_characteristic_t motion_detect;
extern "C" homekit_characteristic_t light_sensor;

static uint32_t next_heap_millis = 0;

void my_homekit_setup() {

  chanel_one_cha_on.setter = chanel_one_set_on;
  chanel_one_cha_bright.setter = chanel_one_set_bright;
  // cha_sat.setter = set_sat;
  // cha_hue.setter = set_hue;
  chanel_two_cha_on.setter = chanel_two_set_on;
  chanel_two_cha_bright.setter = chanel_two_set_bright;
  
	arduino_homekit_setup(&accessory_config);
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
  if(millis() > time_lux_update + LUX_UPDATE_PERIOD){
    time_lux_update = millis();
    update_brightness();
  }
  // check if motion sensor is trigger in each loop , interrupt dont work
  if(digitalRead(MOTION_PIN) == true && last_motion == false ){
    last_motion = true;
    motion_detect.value.bool_value = true;
    homekit_characteristic_notify(&motion_detect, motion_detect.value);
  }
  if(digitalRead(MOTION_PIN) == false && last_motion == true ){
    last_motion = false;
    motion_detect.value.bool_value = false;
    homekit_characteristic_notify(&motion_detect, motion_detect.value);
  }
}


void chanel_one_set_on(const homekit_value_t v) {
    bool on = v.bool_value;
    chanel_one_cha_on.value.bool_value = on; //sync the value

    if(on) {
        chanel_one_is_on = true;
        Serial.println("On");
        if(brightness_level_index == 0 ){   //if current brightness is 0, then when turned on change brightness to 66%
          brightness_level_index = 2;
          chanel_one_current_brightness = brightness_levels[brightness_level_index];
          chanel_one_cha_bright.value.int_value = chanel_one_current_brightness; //sync the value
        }
    } else  {
        chanel_one_is_on = false;
        Serial.println("Off");
    }
    // updateColor();
    chanel_one_updateBrightness();
}

void chanel_two_set_on(const homekit_value_t v) {
    bool on = v.bool_value;
    chanel_two_cha_on.value.bool_value = on; //sync the value
    if(on) {
        chanel_two_is_on = true;
        Serial.println("On");
        if(chanel_two_current_brightness == 0 ){
          chanel_two_current_brightness = 66;
          chanel_two_cha_bright.value.int_value = chanel_two_current_brightness;
        }

    } else  {
        chanel_two_is_on = false;
        Serial.println("Off");
    }
    // updateColor();
    chanel_two_updateBrightness();
}

void chanel_one_set_bright(const homekit_value_t v) {
    Serial.println("set_bright");
    int bright = v.int_value;
    chanel_one_cha_bright.value.int_value = bright; //sync the value

    chanel_one_current_brightness = bright;
    chanel_one_updateBrightness();
    // updateColor();
}

void chanel_two_set_bright(const homekit_value_t v) {
    Serial.println("set_bright");
    int bright = v.int_value;
    chanel_two_cha_bright.value.int_value = bright; //sync the value
    chanel_two_updateBrightness();
    // updateColor();
}

void chanel_one_updateBrightness(){
  if(chanel_one_is_on){
    analogWrite(MAIN_LED_PIN, MAP_100_2_255(chanel_one_current_brightness));
  }
  else if(!chanel_one_is_on){
    analogWrite(MAIN_LED_PIN, 0);
  }
}

void chanel_two_updateBrightness(){
  if(chanel_two_is_on){
    analogWrite(AMBIENT_LED_PIN, MAP_100_2_255(chanel_two_cha_bright.value.int_value));
  }
  else if(!chanel_two_is_on){
    analogWrite(AMBIENT_LED_PIN, 0);
  }
}

void single_click_handler(Button2& btn){
  if(brightness_level_index < brightness_level_num-1){
    brightness_level_index += 1;
  }
  else{
    brightness_level_index = 0;
  }
  chanel_one_current_brightness = brightness_levels[brightness_level_index];
  if(chanel_one_current_brightness == 0){
    chanel_one_is_on = false;
  }
  else{
     chanel_one_is_on = true;
  }
  chanel_one_cha_on.value.bool_value = chanel_one_is_on;
  homekit_characteristic_notify(&chanel_one_cha_on, chanel_one_cha_on.value);
  // Serial.print(current_brightness);
  chanel_one_cha_bright.value.int_value = chanel_one_current_brightness; //sync the value
  homekit_characteristic_notify(&chanel_one_cha_bright, chanel_one_cha_bright.value);
  chanel_one_updateBrightness();
}

void longClick_handler(Button2& btn){
  if(chanel_two_is_on){     // turn off when light is on
    analogWrite(AMBIENT_LED_PIN, 0);
    chanel_two_is_on = false;
  }
  else if(!chanel_two_is_on){  // turn on when light is off and set to last brightness
    chanel_two_is_on = true;
    // chanel_two_current_brightness = brightness_levels[brightness_level_index];
  }
  chanel_two_cha_on.value.bool_value = chanel_two_is_on;
  homekit_characteristic_notify(&chanel_two_cha_on, chanel_two_cha_on.value);
  chanel_two_updateBrightness();
}

void update_brightness(){
  lux = lightMeter.readLightLevel();
  light_sensor.value.float_value = lux;
  homekit_characteristic_notify(&light_sensor, light_sensor.value);
}

void motion_trigger(){

}

void motion_reset_isr(){
  motion_detect.value.bool_value = false;
  homekit_characteristic_notify(&motion_detect, motion_detect.value);
}