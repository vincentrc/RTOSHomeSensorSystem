//dht.h
#ifndef dht_H_
#define dht_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

//API functions
void DHT_open();
void DHT_read();
void DHT_close();
void DHT_ctl();

//dht11 sensor task prototype (Temp and Humidity)
void DHT_Task(void *pvParameter);

#endif