/*
 * my_accessory.c
 * Define the accessory in C language using the Macro in characteristics.h
 *
 *  Created on: 2020-05-15
 *      Author: Mixiaoxiao (Wang Bin)
 */

#include <homekit/homekit.h>
#include <homekit/characteristics.h>

void my_accessory_identify(homekit_value_t _value) {
	printf("accessory identify\n");
}

// format: float; min 0, max 100, step 0.1, unit celsius
homekit_characteristic_t cha_temperature = HOMEKIT_CHARACTERISTIC_(CURRENT_TEMPERATURE, 1);

// format: float; min 0, max 100, step 1
homekit_characteristic_t cha_humidity = HOMEKIT_CHARACTERISTIC_(CURRENT_RELATIVE_HUMIDITY, 1);

homekit_characteristic_t cha_active = HOMEKIT_CHARACTERISTIC_(ACTIVE, (uint8_t)1);

homekit_characteristic_t cha_humidity_threshold = HOMEKIT_CHARACTERISTIC_(RELATIVE_HUMIDITY_HUMIDIFIER_THRESHOLD, (float)60);

homekit_characteristic_t cha_current_humidifier = HOMEKIT_CHARACTERISTIC_(CURRENT_HUMIDIFIER_DEHUMIDIFIER_STATE, (uint8_t)1);
homekit_characteristic_t cha_target_humidifier = HOMEKIT_CHARACTERISTIC_(TARGET_HUMIDIFIER_DEHUMIDIFIER_STATE, (uint8_t)1);

homekit_characteristic_t cha_switch_on = HOMEKIT_CHARACTERISTIC_(ON, false);

homekit_accessory_t *accessories[] = {
     HOMEKIT_ACCESSORY(.id=1, .category=homekit_accessory_category_humidifier, .services=(homekit_service_t*[]) {
        HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics=(homekit_characteristic_t*[]) {
            HOMEKIT_CHARACTERISTIC(NAME, "Humidifier"),
            HOMEKIT_CHARACTERISTIC(MANUFACTURER, "Arduino HomeKit"),
            HOMEKIT_CHARACTERISTIC(SERIAL_NUMBER, "0123456"),
            HOMEKIT_CHARACTERISTIC(MODEL, "ESP8266/ESP32"),
            HOMEKIT_CHARACTERISTIC(FIRMWARE_REVISION, "1.0"),
            HOMEKIT_CHARACTERISTIC(IDENTIFY, my_accessory_identify),
            NULL
        }),
		HOMEKIT_SERVICE(HUMIDIFIER_DEHUMIDIFIER, .primary=true, .characteristics=(homekit_characteristic_t*[]){
		  HOMEKIT_CHARACTERISTIC(NAME, "Humidifier"),
      &cha_active,
      &cha_humidity,
      &cha_humidity_threshold,
      &cha_current_humidifier,
      &cha_target_humidifier,
			NULL
		}),
//   HOMEKIT_SERVICE(SWITCH, .primary=false, .characteristics=(homekit_characteristic_t*[]){
//      &cha_switch_on,
//      NULL
//    }),
        NULL
    }),
    NULL
};

homekit_server_config_t config = {
		.accessories = accessories,
		.password = "111-11-111"
};
