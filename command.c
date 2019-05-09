//command.c
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "iodev.h"

//Signature global variable
extern char signature[5][3];
/* 	Signature will hold the status of the 5 sensor values in the order:
	0: photo 
	1: gas 
	2: particle 
	3: temp 
	4: humid
	The values will be:
	very low (VL), low (L), normal (N), high (H), very high (VH)
*/


//Command task - Turns actuation equipment (LEDs) on/off based on signature
void command(void *pvParameter)
{
	
	//File descriptor variables
	int furnacefd, acfd, humidifierfd, dehumidifierfd, aircleanerfd, ealarmfd;
	
	//First open all actuation devices as files
	//printf("a");
	furnacefd = dopen("furnace");
	//printf("b");
	acfd = dopen("ac");//This one
	//printf("c");
	humidifierfd = dopen("humidifier");
	//printf("d");
	dehumidifierfd = dopen("dehumidifier");
	//printf("e");
	aircleanerfd = dopen("air cleaner");
	//printf("f");
	ealarmfd = dopen("e-alarm");
	//printf("g");
	
	
	
	//Periodically poll the signature and control actuators accordingly
	while(1)
	{
		//Furnace
		if(!strcmp(signature[3], "L") || !strcmp(signature[3], "VL"))
			dwrite(furnacefd, 1);
		else
			dwrite(furnacefd, 0);
		
		//Air conditioner
		if(!strcmp(signature[3], "H") || !strcmp(signature[3], "VH"))
			dwrite(acfd, 1);
		else
			dwrite(acfd, 0);
		
		//Humidifier
		if(!strcmp(signature[4], "L") || !strcmp(signature[4], "VL"))
			dwrite(humidifierfd, 1);
		else
			dwrite(humidifierfd, 0);
		
		//Dehumidifier
		if(!strcmp(signature[4], "H") || !strcmp(signature[4], "VH"))
			dwrite(dehumidifierfd, 1);
		else
			dwrite(dehumidifierfd, 0);
		
		//Air Cleaner
		if(!strcmp(signature[1], "H") || !strcmp(signature[1], "VH") 
			|| !strcmp(signature[2], "H") || !strcmp(signature[2], "H"))
			dwrite(aircleanerfd, 1);
		else
			dwrite(aircleanerfd, 0);
		
		//Emergency Alarm
		if(!strcmp(signature[0], "VH") && !strcmp(signature[1], "VH") 
			&& !strcmp(signature[2], "VH") && !strcmp(signature[3], "VH"))
			dwrite(ealarmfd, 1);
		else
			dwrite(ealarmfd, 0);
		
		//Delay a small amount of time between iterations
		vTaskDelay(50 / portTICK_RATE_MS);	

	}
}






