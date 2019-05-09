//othertasks.h
#ifndef othertasks_H_
#define othertasks_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

//Task prototypes
void manager(void *pvParameter);
void command(void *pvParameter);


#endif