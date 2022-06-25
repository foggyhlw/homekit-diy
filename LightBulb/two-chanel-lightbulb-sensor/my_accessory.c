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

homekit_characteristic_t chanel_one_cha_on = HOMEKIT_CHARACTERISTIC_(ON, false);
homekit_characteristic_t chanel_one_cha_name = HOMEKIT_CHARACTERISTIC_(NAME, "Philips Lamp");
homekit_characteristic_t chanel_one_cha_bright = HOMEKIT_CHARACTERISTIC_(BRIGHTNESS, 66);

homekit_characteristic_t chanel_two_cha_on = HOMEKIT_CHARACTERISTIC_(ON, false);
homekit_characteristic_t chanel_two_cha_name = HOMEKIT_CHARACTERISTIC_(NAME, "Ambient Lamp");
homekit_characteristic_t chanel_two_cha_bright = HOMEKIT_CHARACTERISTIC_(BRIGHTNESS, 60);   //int
// homekit_characteristic_t cha_sat = HOMEKIT_CHARACTERISTIC_(SATURATION, (float) 0);
// homekit_characteristic_t cha_hue = HOMEKIT_CHARACTERISTIC_(HUE, (float) 180);
homekit_characteristic_t motion_detect = HOMEKIT_CHARACTERISTIC_(MOTION_DETECTED, false);   //bool
homekit_characteristic_t motion_sensor_name = HOMEKIT_CHARACTERISTIC_(NAME, "Motion Sensor");   //bool
homekit_characteristic_t light_sensor = HOMEKIT_CHARACTERISTIC_(CURRENT_AMBIENT_LIGHT_LEVEL, (float)0);   //float
homekit_characteristic_t light_sensor_name = HOMEKIT_CHARACTERISTIC_(NAME, "Light Sensor");   //float
homekit_accessory_t *accessories[] = {
    HOMEKIT_ACCESSORY(.id=1, .category=homekit_accessory_category_lightbulb, .services=(homekit_service_t*[]) {
        HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics=(homekit_characteristic_t*[]) {
            HOMEKIT_CHARACTERISTIC(NAME, "Philips Lamp"),
            HOMEKIT_CHARACTERISTIC(MANUFACTURER, "Arduino HomeKit"),
            HOMEKIT_CHARACTERISTIC(SERIAL_NUMBER, "0123456"),
            HOMEKIT_CHARACTERISTIC(MODEL, "ESP8266/ESP32"),
            HOMEKIT_CHARACTERISTIC(FIRMWARE_REVISION, "1.0"),
            HOMEKIT_CHARACTERISTIC(IDENTIFY, my_accessory_identify),
            NULL
        }),
        HOMEKIT_SERVICE(LIGHTBULB, .primary=true, .characteristics=(homekit_characteristic_t*[]) {
            &chanel_one_cha_on,
            &chanel_one_cha_name,
            &chanel_one_cha_bright,
            // &cha_sat,
            // &cha_hue,
            NULL
        }),
        HOMEKIT_SERVICE(LIGHTBULB, .primary=false, .characteristics=(homekit_characteristic_t*[]) {
            &chanel_two_cha_on,
            &chanel_two_cha_name,
            &chanel_two_cha_bright,
            // &cha_sat,
            // &cha_hue,
            NULL
        }),
        HOMEKIT_SERVICE(LIGHT_SENSOR, .primary=false, .characteristics=(homekit_characteristic_t*[]) {
            &light_sensor,
            &light_sensor_name,
            NULL
        }),
        HOMEKIT_SERVICE(MOTION_SENSOR, .primary=false, .characteristics=(homekit_characteristic_t*[]) {
            &motion_detect,
            &motion_sensor_name,
            NULL
        }),
        NULL
    }),
    NULL
};

homekit_server_config_t accessory_config = {
    .accessories = accessories,
    .password = "123-45-687"
};