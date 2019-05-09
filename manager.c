//manager.c
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "othertasks.h"
#include "functions.h"

//Global variables from main
extern float photo, gas, particle, temp, humid;
extern QueueHandle_t queue;
extern char signature[5][3];

//Manager task
void manager(void *pvParameter)
{
	char string[17], sensname[9];
	float value;
	char *token;
	
	//Counts for each variable
	int cphoto = 0, cgas = 0, cparticle = 0, ctemp = 0, chumid = 0;
	
	//Conditions/Status of each sensor (high, low, etc.)
	char photostat[3], gasstat[3], particlestat[3], tempstat[3], humidstat[3];
	
	while(1)
	{
		//Dequeue the next message if one exists
		if (uxQueueSpacesAvailable(queue) != 100)	//100 is the size of the queue
		{
			//Receive the next message from the queue
			if (xQueueReceive(queue, &string, portMAX_DELAY))
			{
				//printf("\n%s\n", string);
				
				//Parse the name of the sensor from the comma-separated message
				token = strtok(string, ",");
				strcpy(sensname, token);
				
				//Get the value from the message
				token = strtok(NULL, ",");
				value = atof(token);
				
				//Calc average and save for correct sensor variable 
				if (!strcmp(sensname, "photo"))
				{
					if (cphoto == 0) //First value for this sensor
						photo = value;
					else
						photo = ewma(photo, value, cphoto);
					cphoto++;	
					
					//Assign status for signature
					if (photo > 0.8)
						strcpy(photostat, "VH");
					else if (photo <= 0.8 && photo > 0.6)
						strcpy(photostat, "H");
					else if (photo <= 0.6 && photo > 0.3)
						strcpy(photostat, "N");
					else if (photo <= 0.3 && photo > 0.1)
						strcpy(photostat, "L");
					else
						strcpy(photostat, "VL");
				}
				else if (!strcmp(sensname, "gas"))
				{
					if (cgas == 0) //First value for this sensor
						gas = value;
					else
						gas = ewma(gas, value, cgas);
					cgas++;	
					
					//Assign status for signature
					if (gas > 200)
						strcpy(gasstat, "VH");
					else if (gas <= 200 && gas > 50)
						strcpy(gasstat, "H");
					else if (gas <= 50 && gas > 25)
						strcpy(gasstat, "N");
					else if (gas <= 25 && gas > 9)
						strcpy(gasstat, "L");
					else
						strcpy(gasstat, "VL");
				}
				else if (!strcmp(sensname, "particle"))
				{
					if (cparticle == 0) //First value for this sensor
						particle = value;
					else
						particle = ewma(particle, value, cparticle);
					cparticle++;	
					
					//Assign status for signature
					if (particle > 3000)
						strcpy(particlestat, "VH");
					else if (particle <= 3000 && particle > 1050)
						strcpy(particlestat, "H");
					else if (particle <= 1050 && particle > 300)
						strcpy(particlestat, "N");
					else if (particle <= 300 && particle > 150)
						strcpy(particlestat, "L");
					else
						strcpy(particlestat, "VL");
				}
				else if (!strcmp(sensname, "temp"))
				{
					if (ctemp == 0) //First value for this sensor
						temp = value;
					else
						temp = ewma(temp, value, ctemp);
					ctemp++;	
					
					//Assign status for signature
					if (temp > 100)
						strcpy(tempstat, "VH");
					else if (temp <= 100 && temp > 80)
						strcpy(tempstat, "H");
					else if (temp <= 80 && temp > 60)
						strcpy(tempstat, "N");
					else if (temp <= 60 && temp > 30)
						strcpy(tempstat, "L");
					else
						strcpy(tempstat, "VL");
				}
				else if (!strcmp(sensname, "humid"))
				{
					if (chumid == 0) //First value for this sensor
						humid = value;
					else	
						humid = ewma(humid, value, chumid);
					chumid++;	
					
					//Assign status for signature
					if (humid > 80)
						strcpy(humidstat, "VH");
					else if (humid <= 80 && humid > 50)
						strcpy(humidstat, "H");
					else if (humid <= 50 && humid > 30)
						strcpy(humidstat, "N");
					else if (humid <= 30 && humid > 10)
						strcpy(humidstat, "L");
					else
						strcpy(humidstat, "VL");
				}
				else
					printf("\nIncorrect name in queue message!\n");
			}
		}
		
		//Fill in the signature array with the updated values
		strcpy(signature[0], photostat);
		strcpy(signature[1], gasstat);	//Needs semaphore
		strcpy(signature[2], particlestat);
		strcpy(signature[3], tempstat);
		strcpy(signature[4], humidstat);
		
		//printf("\n%s, %s, %s, %s, %s\n", 
		//signature[0], signature[1], signature[2], signature[3], signature[4]); //Test code
		
		
		//Added small delay 
		vTaskDelay(50 / portTICK_RATE_MS);
	}
}