//gas.h
#ifndef gas_H_
#define gas_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

//API functions
void gas_open();
void gas_read();
void gas_close();
void gas_ctl();

//Gas sensor task prototype
void gastask(void *pvParameter);

#endif