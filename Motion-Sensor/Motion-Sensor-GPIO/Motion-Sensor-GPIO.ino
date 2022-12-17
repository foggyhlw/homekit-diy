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
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager

#define LOG_D(fmt, ...)   printf_P(PSTR(fmt "\n") , ##__VA_ARGS__);
#define MAP_100_2_55(val) map(val,0,100,0,55)   //Fadeled library use 10bit resolution while arduino is only 8bit
#define BUILT_IN_LED 2
#define RESET_HK_PERIOD 10000
#define WIFI_CHECK_PERIOD 10000
#define UPDATE_PERIOD 1000
#define RESET_HK_CLICK 4   // used to reset homekit pairing information if button is clicked 4times after startup within 10s

#define LD2410_DOUT_PIN 12

int reset_click_number = 0;
int wifi_reconnect_count = 0;
unsigned long time_now = 0;
unsigned long time_now1 = 0;
unsigned long time_now2 = 0;
unsigned long time_now3 = 0;
bool first_start_loop = true;
volatile bool press_rotation_flag = false;   

uint8_t occupancy_detected = 0;
// bool motion_detected = false;
bool light_active = false;
float lux = 0.01 ;   // min 0.0001 max 100000
WiFiManager wm;
//==============================
// HomeKit setup and loop
//==============================

// access your HomeKit characteristics defined in my_accessory.c
extern "C" homekit_server_config_t accessory_config;
extern "C" homekit_characteristic_t cha_occupancy;
extern "C" homekit_characteristic_t cha_light;
extern "C" homekit_characteristic_t device_name;
extern "C" homekit_characteristic_t cha_light_active;
static uint32_t next_heap_millis = 0;


void setup() {
	Serial.begin(115200);
  Serial.print(F("\nLD2410 radar sensor initialising: "));
  pinMode(LD2410_DOUT_PIN, INPUT);
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
        digitalWrite(BUILT_IN_LED,0);
        delay(1000);
        digitalWrite(BUILT_IN_LED,1);
        delay(1000);
        digitalWrite(BUILT_IN_LED,0);
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
  if(millis() > time_now3 + UPDATE_PERIOD){
    time_now3 = millis();
    update_motion();
    update_light();
  }
  if(millis() > time_now2 + 10){
    wm.process();
    my_homekit_loop();
    time_now2 = millis();
  }
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
}

void update_motion(){
  if(digitalRead(LD2410_DOUT_PIN) == HIGH){
    occupancy_detected = 1;
  }
  else{
    occupancy_detected = 0;
  }
  cha_occupancy.value.uint8_value = occupancy_detected;
  homekit_characteristic_notify(&cha_occupancy, cha_occupancy.value);
}

void update_light(){
  lux = 1;
  cha_light.value.float_value = lux;
  homekit_characteristic_notify(&cha_light, cha_light.value);
  light_active = true;
  cha_light_active.value.bool_value = light_active;
  homekit_characteristic_notify(&cha_light_active, cha_light_active.value);
}

