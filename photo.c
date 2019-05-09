//photo.c
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
#include "photo.h"
#include "functions.h"
#include "iodev.h"
#include "freertos/semphr.h"

//Global variables from main
extern TaskHandle_t PhotoHandle;
extern QueueHandle_t queue;
extern SemaphoreHandle_t queueSemaphore;

//Global function pointers
void (*photo_open_ptr)() = &Photo_open;
void (*photo_read_ptr)() = &Photo_read;
void (*photo_ctl_ptr)() = &Photo_ctl;
void (*photo_close_ptr)() = &Photo_close;


//Photo Sensor Task
void Photo_Task(void *pvParameter)
{
  nvs_handle my_handle = NULL;
	int Photo_Period = 1000;
  char periodstring[25];
  esp_err_t err = NULL;
  TickType_t xLastWakeTime, finishtime;
  TickType_t exectime, slacktime, utilization;
  int fd;

  //Initialize Photoresistor
  fd = dopen("photo");

	//Initialize task period
	nvsset(my_handle, "Photo_Period", "1000", err);
  

    while(1)
    {
		//Set start time for period
		xLastWakeTime = xTaskGetTickCount();
		
      dread(fd);

      nvsget(my_handle, "Photo_Period", periodstring, err);
      Photo_Period = atoi(periodstring);
	  
	  //Capture the finish time for the task
		finishtime = xTaskGetTickCount();
		
		//Calculate execution time, slack time and utilization
		exectime = finishtime - xLastWakeTime;
		slacktime = Photo_Period - exectime;
		utilization = exectime/(Photo_Period/portTICK_RATE_MS);

      vTaskDelayUntil(&xLastWakeTime, Photo_Period / portTICK_RATE_MS);
    }

}


//API functions
void Photo_open()
{

    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_3,ADC_ATTEN_DB_0);
    return;
}

void Photo_read()
{
  char string[17];

  float val = adc1_get_raw(ADC1_CHANNEL_3);
  val = val/4095;

  sprintf(string,"photo,%3.2f,",val);
  //printf("\nPhoto: %f\n", val);
  
	//Take a reading and place in queue 
	if (xSemaphoreTake(queueSemaphore, 0))
	{
		 if (uxQueueSpacesAvailable(queue) > 0)
			xQueueSend(queue, &string, 0);
		xSemaphoreGive( queueSemaphore );
	}


  return;
}

void Photo_close()
{
  vTaskDelete(PhotoHandle);
  return;
}

void Photo_ctl()
{
	nvs_handle my_handle = NULL;
	char string[50];
	esp_err_t err = NULL;

	//Prompt user for new task period
	printf("\nPlease enter a new period in ms:\n");
	getstring(string, 50);

	//Sets in NVS
	nvsset(my_handle, "Photo_Period", string, err);
	return;
}


