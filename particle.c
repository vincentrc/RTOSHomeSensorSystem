//particle.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "nvs.h"
#include "rom/ets_sys.h"
#include "driver/gpio.h"
#include "esp_adc_cal.h"
#include "particle.h"
#include "functions.h"
#include "iodev.h"
#include "freertos/semphr.h"

//Global variables from main
extern TaskHandle_t particleHandle;
extern QueueHandle_t queue;
extern SemaphoreHandle_t queueSemaphore;

//Global function pointers
void (*particle_open_ptr)() = &particle_open;
void (*particle_read_ptr)() = &particle_read;
void (*particle_ctl_ptr)() = &particle_ctl;
void (*particle_close_ptr)() = &particle_close;

//Particle sensor task
void particletask(void *pvParameter)
{
	nvs_handle my_handle = NULL;
	int particlePeriod = 1000;
	char periodstring[25];
	esp_err_t err = NULL;
	TickType_t xLastWakeTime, finishtime;
	TickType_t exectime, slacktime, utilization;
	int fd;
	
	//Initialize the sensor
	fd = dopen("particle");
	
	//Initialize task period
	nvsset(my_handle, "particlePeriod", "1000", err);
	
	while(1)
	{
		//Set start time for period
		xLastWakeTime = xTaskGetTickCount();
		
		//Take a reading and place in queue 
		dread(fd);
		
		//Check NVS for period (Since it is user configurable and could change)
		nvsget(my_handle, "particlePeriod", periodstring, err);
		particlePeriod = atoi(periodstring);	//Switch to int and update period
		
		//Capture the finish time for the task
		finishtime = xTaskGetTickCount();
		
		//Calculate execution time, slack time and utilization
		exectime = finishtime - xLastWakeTime;
		slacktime = particlePeriod - exectime;
		utilization = exectime/(particlePeriod/portTICK_RATE_MS);
		
		//Wait for the remainder of the configured period
		 vTaskDelayUntil( &xLastWakeTime, particlePeriod / portTICK_RATE_MS);
	}
}


//API functions
void particle_open()
{
	gpio_config_t io_conf;

	//Initializations
	adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_0,ADC_ATTEN_DB_0);

	//set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins to set
    io_conf.pin_bit_mask = 1ULL<<13;
	//disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);
	
	return;
}

void particle_read()
{
	char string[17]; //String for queue
	float dustVal;
	
	//Read in the value from the sensor
	gpio_set_level(13, 0);
	ets_delay_us(280);
	dustVal = adc1_get_raw(ADC1_CHANNEL_0);	//Analogread goes here
	dustVal /= 4;
	ets_delay_us(40);
	gpio_set_level(13, 1);
	ets_delay_us(40);
	
	//Set up string for queue
	sprintf(string, "particle,%3.2f,", dustVal);
	//printf("\n%s\n", string);
	
	
	//Take a reading and place in queue 
	if (xSemaphoreTake(queueSemaphore, 0))
	{
		//Add the reading to the queue
		if (uxQueueSpacesAvailable(queue) > 0) //Only if space available
			xQueueSend(queue, &string, 0);
		xSemaphoreGive( queueSemaphore );
	}

		
	return;
}

void particle_close()
{
	//Deletes particle task, freeing its memory 
	vTaskDelete( particleHandle );
}

void particle_ctl()
{
	nvs_handle my_handle = NULL;
	char string[50];
	esp_err_t err = NULL;
	
	//Prompt user for new task period
	printf("\nPlease enter a new period in ms:\n");
	getstring(string, 50);
	
	//Sets in NVS
	nvsset(my_handle, "particlePeriod", string, err);
	return;
}