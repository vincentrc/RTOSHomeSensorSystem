//particle.h
#ifndef particle_H_
#define particle_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

//API functions
void particle_open();
void particle_read();
void particle_close();
void particle_ctl();

//Particle sensor task prototype
void particletask(void *pvParameter);

#endif