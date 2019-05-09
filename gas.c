//gas.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "nvs.h"
#include "rom/ets_sys.h"
#include "driver/gpio.h"
#include "esp_adc_cal.h"
#include "gas.h"
#include "functions.h"
#include "iodev.h"
#include "freertos/semphr.h"

//Global variables from main
extern TaskHandle_t gasHandle;
extern QueueHandle_t queue;
extern SemaphoreHandle_t queueSemaphore;

//Global function pointers
void (*gas_open_ptr)() = &gas_open;
void (*gas_read_ptr)() = &gas_read;
void (*gas_ctl_ptr)() = &gas_ctl;
void (*gas_close_ptr)() = &gas_close;


//Gas sensor task
void gastask(void *pvParameter)
{
	nvs_handle my_handle = NULL;
	int gasPeriod = 1000;
	char periodstring[25];
	esp_err_t err = NULL;
	TickType_t xLastWakeTime, finishtime;
	TickType_t exectime, slacktime, utilization;
	int fd;

	//Initialize the sensor
	fd = dopen("gas");

	//Initialize particle task period
	nvsset(my_handle, "gasPeriod", "1000", err);

	while(1)
	{
		//Set start time for period
		xLastWakeTime = xTaskGetTickCount();
		
		//Take a reading and place in queue
		dread(fd);

		//Check NVS for period (Since it is user configurable and could change)
		nvsget(my_handle, "gasPeriod", periodstring, err);
		gasPeriod = atoi(periodstring);	//Switch to int and update period
		
		//Capture the finish time for the task
		finishtime = xTaskGetTickCount();
		
		//Calculate execution time, slack time and utilization
		exectime = finishtime - xLastWakeTime;
		slacktime = gasPeriod - exectime;
		utilization = exectime/(gasPeriod/portTICK_RATE_MS);
		
		printf("\nexec: %d\n", exectime);
		printf("\nslack: %d\n", slacktime);
		printf("\nutil: %d\n", utilization);

		//Wait for the configured period
		vTaskDelayUntil(&xLastWakeTime, gasPeriod / portTICK_RATE_MS);
	}
}


//API functions
void gas_open()
{

	//Initializations
	adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_6,ADC_ATTEN_DB_0);//Using ADC6 which is GPIO34

	//Set open variable to 1
	return;
}

void gas_read()
{
	char string[17]; //String for queue
	float sensorVal=0;
	float ppm=0;


	//Read in the value from the sensor
	sensorVal = adc1_get_raw(ADC1_CHANNEL_6);

    ppm=3.027*exp(1.0698*(sensorVal*5/4095));

	//Set up string for queue
	sprintf(string, "gas,%3.2f,", ppm);
    //printf("\nPPM= %0.3lf\n",ppm );

	//Take a reading and place in queue 
	if (xSemaphoreTake(queueSemaphore, 0))
	{
		//Add the reading to the queue
		if (uxQueueSpacesAvailable(queue) > 0) //Only if space available
			xQueueSend(queue, &string, 0);
		xSemaphoreGive( queueSemaphore );
	}
	//Add the reading to the queue
	if (uxQueueSpacesAvailable(queue) > 0) //Only if space available
		xQueueSend(queue, &string, 0);

	return;
}

void gas_close()
{
	//Seletes particle task, freeing its memory
	vTaskDelete( gasHandle );
}

void gas_ctl()
{
	nvs_handle my_handle = NULL;
	char string[50];
	esp_err_t err = NULL;

	//Prompt user for new task period
	printf("\nPlease enter a new period in ms:\n");
	getstring(string, 50);

	//Sets in NVS
	nvsset(my_handle, "gasPeriod", string, err);
	return;
}