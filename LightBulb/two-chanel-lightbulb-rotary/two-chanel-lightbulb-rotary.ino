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
// #include "wifi_info.h"
#include "Button2.h"
#include "ESPRotary.h"
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <FadeLed.h>

#define LOG_D(fmt, ...)   printf_P(PSTR(fmt "\n") , ##__VA_ARGS__);
#define MAP_100_2_255(val) map(val,0,100,0,55)
#define MAIN_LED_PIN 4
#define AMBIENT_LED_PIN 2
#define INPUT_BUTTON_PIN 13
#define ROTARY_PIN2	 12  //D6
#define ROTARY_PIN1	14  //D7
#define CLICKS_PER_STEP 4
#define RESET_HK_PERIOD 10000
#define WIFI_CHECK_PERIOD 10000
#define RESET_HK_CLICK 4   // used to reset homekit pairing information if button is clicked 4times after startup within 10s

// FadeLed leds[2] = {MAIN_LED_PIN, AMBIENT_LED_PIN};
FadeLed mainled(MAIN_LED_PIN);
FadeLed ambled(AMBIENT_LED_PIN);
int reset_click_number = 0;
int wifi_reconnect_count = 0;
unsigned long time_now = 0;
unsigned long time_now1 = 0;
unsigned long time_now2 = 0;

bool first_start_loop = true;
volatile bool press_rotation_flag = false;   

bool chanel_one_is_on = false;
bool chanel_two_is_on = false;
float chanel_one_current_brightness =  00;
float chanel_two_current_brightness =  00;

bool chanel_one_changing_flag = false;
bool chanel_two_changing_flag = false;
// float current_sat = 0.0;
// float current_hue = 0.0;
// int rgb_colors[3];
Button2 button;
WiFiManager wm;
ESPRotary rotary = ESPRotary(ROTARY_PIN1, ROTARY_PIN2, CLICKS_PER_STEP);

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
extern "C" homekit_characteristic_t device_name;
static uint32_t next_heap_millis = 0;


void setup() {
  rotary.setRightRotationHandler(RightRotationHandler);
  rotary.setLeftRotationHandler(LeftRotationHandler);
  button.begin(INPUT_BUTTON_PIN);
  button.setLongClickTime(500);  // set longclick time to 500ms 

	Serial.begin(115200);
	// wifi_connect(); // in wifi_info.h
  mainled.setTime(1000,true);
  ambled.setTime(2000);
  mainled.setInterval(10);
  // mainled.off();
  // ambled.off();
  pinMode(MAIN_LED_PIN, OUTPUT);
  pinMode(AMBIENT_LED_PIN, OUTPUT);
  digitalWrite(MAIN_LED_PIN, LOW);
  digitalWrite(AMBIENT_LED_PIN, LOW);
  button.setClickHandler(single_click_handler);
  button.setLongClickHandler(longClick_handler);
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

//********************loop************************
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
  FadeLed::update();
  // if(mainled.done()){
  //   if(chanel_one_changing_flag == true){
  //     chanel_one_cha_bright.value.int_value = chanel_one_current_brightness; //sync the value
  //     homekit_characteristic_notify(&chanel_one_cha_bright, chanel_one_cha_bright.value);
  //     chanel_one_changing_flag =false;
  //   }
  // }
  // if(ambled.done()){
  //   if(chanel_two_changing_flag == true){
  //     chanel_two_cha_bright.value.int_value = chanel_two_current_brightness; //sync the value
  //     homekit_characteristic_notify(&chanel_two_cha_bright, chanel_two_cha_bright.value);
  //     chanel_two_changing_flag = false;
  //   }
  // }

  // if(WiFi.isConnected()){

  // }
  if(millis() > time_now2 + 10){
    wm.process();
    my_homekit_loop();
    time_now2 = millis();
  }
  button.loop();
  rotary.loop();
	// delay(10);
}



void my_homekit_setup() {
  uint8_t mac[WL_MAC_ADDR_LENGTH];
	WiFi.macAddress(mac);
	int name_len = snprintf(NULL, 0, "%s_%02X%02X%02X",
			device_name.value.string_value, mac[3], mac[4], mac[5]);
	char *name_value = (char*) malloc(name_len + 1);
	snprintf(name_value, name_len + 1, "%s_%02X%02X%02X",
			device_name.value.string_value, mac[3], mac[4], mac[5]);
	device_name.value = HOMEKIT_STRING_CPP(name_value);

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
	// const uint32_t t = millis();
	// if (t > next_heap_millis) {
	// 	// show heap info every 5 seconds
	// 	next_heap_millis = t + 5 * 1000;
	// 	LOG_D("Free heap: %d, HomeKit clients: %d",
	// 			ESP.getFreeHeap(), arduino_homekit_connected_clients_count());

	// }
}

void chanel_one_set_on(const homekit_value_t v) {
    bool on = v.bool_value;
    chanel_one_cha_on.value.bool_value = on; //sync the value

    if(on) {
        chanel_one_is_on = true;
        Serial.println("On");
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
    } else  {
        chanel_two_is_on = false;
        Serial.println("Off");
    }
    // updateColor();
    chanel_two_updateBrightness();
}

void chanel_one_set_bright(const homekit_value_t v) {
    // Serial.println("set_bright");
    int bright = v.int_value;
    chanel_one_cha_bright.value.int_value = bright; //sync the value

    chanel_one_current_brightness = bright;
    chanel_one_updateBrightness();
    // updateColor();
}

void chanel_two_set_bright(const homekit_value_t v) {
    // Serial.println("set_bright");
    int bright = v.int_value;
    chanel_two_cha_bright.value.int_value = bright; //sync the value
    chanel_two_current_brightness = bright;
    chanel_two_updateBrightness();
    // updateColor();
}

void chanel_one_updateBrightness(){
  if(chanel_one_is_on){
    // analogWrite(MAIN_LED_PIN, MAP_100_2_255(chanel_one_current_brightness));
    // mainled.set(chanel_one_current_brightness);
    mainled.set(MAP_100_2_255(chanel_one_current_brightness));

  }
  else if(!chanel_one_is_on){
    // analogWrite(MAIN_LED_PIN, 0);
    mainled.off();
  }
}

void chanel_two_updateBrightness(){
  if(chanel_two_is_on){
    // analogWrite(AMBIENT_LED_PIN, MAP_100_2_255(chanel_two_current_brightness));
    // ambled.set(chanel_two_current_brightness);
    ambled.set(MAP_100_2_255(chanel_two_current_brightness));
  }
  else if(!chanel_two_is_on){
    // analogWrite(AMBIENT_LED_PIN, 0);
    ambled.set(0);
  }
}


void single_click_handler(Button2& btn){
  Serial.print("click!\n");
  reset_click_number++;

  if(chanel_one_is_on){     // turn off when light is on
    mainled.off();
    // analogWrite(MAIN_LED_PIN, 0);
    chanel_one_is_on = false;
  }
  else if(!chanel_one_is_on){  // turn on when light is off and set to last brightness

    chanel_one_is_on = true;
  }
  chanel_one_cha_on.value.bool_value = chanel_one_is_on;
  homekit_characteristic_notify(&chanel_one_cha_on, chanel_one_cha_on.value);
  chanel_one_updateBrightness();
}

void longClick_handler(Button2& btn){
  Serial.print("long click!\n");
  // Serial.println(press_rotation_flag);
  if(press_rotation_flag == false){
    if(chanel_two_is_on){     // turn off when light is on
    // analogWrite(AMBIENT_LED_PIN, 0);
    ambled.off();
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
  else{
    press_rotation_flag = false;
  }
  
}


void RightRotationHandler(ESPRotary& r) {
  if(button.isPressed()){   
    press_rotation_flag = true;
    // Serial.println(press_rotation_flag);
    if(chanel_two_current_brightness<100){
      chanel_two_current_brightness += 10;
    }
    else{
      chanel_two_current_brightness = 100;
    }
    chanel_two_changing_flag = true;
    chanel_two_cha_bright.value.int_value = chanel_two_current_brightness; //sync the value
    homekit_characteristic_notify(&chanel_two_cha_bright, chanel_two_cha_bright.value);
    chanel_two_updateBrightness();
  }
  else{   
    // Serial.println(r.getPosition());
    if(chanel_one_current_brightness<100){
      chanel_one_current_brightness += 10;
    }
    else{
      chanel_one_current_brightness = 100;
    }
    chanel_one_changing_flag = true;
    chanel_one_cha_bright.value.int_value = chanel_one_current_brightness; //sync the value
    homekit_characteristic_notify(&chanel_one_cha_bright, chanel_one_cha_bright.value);
    chanel_one_updateBrightness();
  }
}

void LeftRotationHandler(ESPRotary& r) {
  if(button.isPressed()){   
    press_rotation_flag = true;
    // Serial.println(r.getPosition());
   if(chanel_two_current_brightness>10){
    chanel_two_current_brightness -= 10;
    }
    else{
      chanel_two_current_brightness = 10;
    }
    chanel_two_cha_bright.value.int_value = chanel_two_current_brightness; //sync the value
    homekit_characteristic_notify(&chanel_two_cha_bright, chanel_two_cha_bright.value);
    chanel_two_updateBrightness();
  }
  else{   
    // Serial.println(r.getPosition());
    if(chanel_one_current_brightness>10){
      chanel_one_current_brightness -= 10;
      }
      else{
        chanel_one_current_brightness = 10;
      }
    chanel_one_cha_bright.value.int_value = chanel_one_current_brightness; //sync the value
    homekit_characteristic_notify(&chanel_one_cha_bright, chanel_one_cha_bright.value);
    chanel_one_updateBrightness();
  }
}