/*
 * wifi_info.h
 *
 *  Created on: 2020-05-15
 *      Author: Mixiaoxiao (Wang Bin)
 */

#ifndef WIFI_INFO_H_
#define WIFI_INFO_H_

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#include <WiFi.h>
#endif

const char *ssid = "foggy_2G";
const char *password = "1989Fox228";

void wifi_connect() {
  int cnt = 0;
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.setAutoConnect(true);
  WiFi.begin(ssid,password);
// if wifi cannot connect start smartconfig
  while(WiFi.status() != WL_CONNECTED) {
    delay(2000);
    Serial.print(".");
    if(cnt++ >= 10){
      Serial.println("start smartconfig");
       WiFi.beginSmartConfig();

       while(1){
           delay(2000);
           if(WiFi.smartConfigDone()){
             Serial.println("SmartConfig Success");
             break;
           }
       }
    }
  }
}

#endif /* WIFI_INFO_H_ */
