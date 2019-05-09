//photo.h
#ifndef photo_H_
#define photo_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

//API functions
void Photo_open();
void Photo_read();
void Photo_close();
void Photo_ctl();

//Photo sensor task prototype
void Photo_Task(void *pvParameter);

#endif