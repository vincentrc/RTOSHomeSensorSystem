//main.c
#include <stdio.h>
#include <stdlib.h>
#include <sodium.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "rom/ets_sys.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "particle.h"
#include "gas.h"
#include "dht.h"
#include "photo.h"
#include "othertasks.h"
#include "functions.h"
#include "iodev.h"
#include "freertos/semphr.h"

#define EXAMPLE_WIFI_SSID "ssid goes here"
#define EXAMPLE_WIFI_PASS "password goes here"


//Global vars for sensor averages and plant signature
float photo = 0, gas = 0, particle = 0, temp = 0, humid = 0;
char signature[5][3]; //Array for holding the signature of the plant condition
/* 	Signature will hold the status of the 5 sensor values in the order:
	0: photo 
	1: gas 
	2: particle 
	3: temp 
	4: humid
	The values will be:
	very low (VL), low (L), normal (N), high (H), very high (VH)
*/
	
//Queue global declaration
QueueHandle_t queue;

//Global queue semaphore
SemaphoreHandle_t queueSemaphore = NULL;

//Global variables
TaskHandle_t particleHandle = NULL;
TaskHandle_t gasHandle = NULL;
TaskHandle_t DHTHandle = NULL;
TaskHandle_t PhotoHandle = NULL;
TaskHandle_t managerHandle = NULL;
TaskHandle_t commandHandle = NULL;
int DHT_PIN;


//Main task 
void app_main()
{
	char str_p[20]; //input str for password hashing
	char keys[50][50]; //This array stores the names of keys created by the set command
	char value[50], key[50]; //Strings to store a value or key 
	char shellstring[50]; //input string for shell commands
	char hash_pass[crypto_pwhash_STRBYTES];
	int foundflag = 0;  //stay 0 if not found
	int shellstate = 0; //1 for logged on , 0 for logged off
	int i, x = 0; //i = iterator, x = index for keys array
	int error;
	nvs_handle my_handle;
	
	// Initialize NVS
	esp_err_t err = nvs_flash_init();
	if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
	{
		// NVS partition was truncated and needs to be erased
		// Retry nvs_flash_init
		ESP_ERROR_CHECK(nvs_flash_erase());
		err = nvs_flash_init();
	}
	ESP_ERROR_CHECK( err );
	
	//Initialize sodium
	if(sodium_init() < 0)
	{
		printf("\nSodium could not be loaded!\n");
		return;
	}
	
	//Create the queue and check for error
	queue = xQueueCreate(100, sizeof("11111111111111111"));
	if (queue == NULL)
	{
		printf("\nError Creating Queue\n");
		return;
	}
	
	//Initialize io device table
	ioinit();
	
	//Create Semaphores
	queueSemaphore = xSemaphoreCreateMutex();
	
	//Store ssid, password and system id in storage
	error = nvsset(my_handle, "ssid", EXAMPLE_WIFI_SSID, err);
	if (error == 1)	//Close program if error
		return;
	error = nvsset(my_handle, "wifipassword", EXAMPLE_WIFI_PASS, err);
	if (error == 1)	//Close program if error
		return;
	error = nvsset(my_handle, "sysID", "001", err);
	if (error == 1)	//Close program if error
		return;
		
	//Store keys in keys array
	strcpy(keys[x], "ssid"); x++;
	strcpy(keys[x], "wifipassword"); x++;
	strcpy(keys[x], "sysID"); x++;
	
	//Create sensor tasks
	xTaskCreate(&particletask, "particletask", 4096, NULL, 5, &particleHandle);
	vTaskDelay(250 / portTICK_RATE_MS);
	xTaskCreate(&gastask, "gastask", 4096, NULL, 5, &gasHandle);
	vTaskDelay(250 / portTICK_RATE_MS);
	xTaskCreate(&DHT_Task, "dhttask", 4096, NULL, 5, &DHTHandle);
	vTaskDelay(250 / portTICK_RATE_MS);
	xTaskCreate(&Photo_Task, "phototask", 4096, NULL, 5, &PhotoHandle);
	vTaskDelay(250 / portTICK_RATE_MS);
	
	//Create manager task
	xTaskCreate(&manager, "manager", 4096, NULL, 5, &managerHandle);
	vTaskDelay(250 / portTICK_RATE_MS);
	
	//Create command task
	xTaskCreate(&command, "command", 4096, NULL, 5, &commandHandle);
	

	//User interface for serial connection***********************************************************************************8
	//The system defaults to logged off and you must type "log on" to be able to enter password
	
	//This will be the loop for the shell (for commands log and set)
	while(1)
	{
		//Logged on state
		if (shellstate == 1)
		{
			//Print prompt char amd take user command
			printf("\n$ ");
			getstring(shellstring, sizeof(shellstring));
		
			//If user enter "log off"
			if (!strcmp(shellstring, "log off"))
			{
				shellstate = 0; 
				continue;
			}
		
			//If the user types "set"
			else if (!strcmp(shellstring, "set"))
			{
				printf("\n");
			
				//Iterate through keys and print key = value for each 
				i = 0;
				while(i < x) //Go until NULL (first empty element)
				{
					//Retrieve value from stroage
					error = nvsget(my_handle, keys[i], value, err);
				
					if (error == 1)	//Close program if error
						return;
				
					printf("%s = %s\n", keys[i], value);	//Print name = value pair
					i++;
				}
			}
		
			//Otherwise "set name=value" or invalid command
			else 	//Need to analyze input
			{
				error = argparse(shellstring, key, value);
				if (error == 1)
					printf("\nInvalid Command\n");
				else
				{
					//After parsing,set store in NV storage
					error = nvsset(my_handle, key, value, err);
					if (error == 1)	//Close program if error
						return;
					
					strcpy(keys[x], key); //Add to key array
					x++;
				}
			
			}

		}
	
		//Logged off state (Requires password to regain entry to the system)
		else
		{
			//Only command possible is "log on" I think
			printf("\n");
			getstring(shellstring, sizeof(shellstring));
		
			//If user enters "log on"
			if (!strcmp(shellstring, "log on"))
			{
				//Open NVS handle
				err = nvs_open("storage", NVS_READWRITE, &my_handle);
				 if (err != ESP_OK)
				{
					printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
					return;
				}
				else //No error
				{
					size_t size = crypto_pwhash_STRBYTES;
					//Read the nvs handle for "password" and place it into hash_pass
					err = nvs_get_str(my_handle, "password", &hash_pass, &size);
					switch (err)
					{
						case ESP_OK:  //found password variable
							printf("Please enter the password\n");
							foundflag = 1;
							break;  //pass
							
						case ESP_ERR_NVS_NOT_FOUND: //variable password needs to be created
							printf("\nPlease create a password: \n");
							getstring(str_p, sizeof(str_p));
							
							//Create new password hash value
							if (crypto_pwhash_str(hash_pass, str_p, strlen(str_p), crypto_pwhash_OPSLIMIT_INTERACTIVE, 8192) != 0)
							{
								printf("Out of memory");
								return;
							}
							else  //Store hash value under "password" key
							{
								err = nvs_set_str(my_handle, "password", hash_pass);
								if (err != ESP_OK) //Failed
								{
									printf("\nFailed to store new password in memory!\n");
									return;
								}
								else  //All is well, commit the changes
								{
									err = nvs_commit(my_handle);
									if (err != ESP_OK)  //Failed
									{
									printf("\nFailed to commit new password storage!\n");
									return;
									}
									else //All is still well, done
										break;
                    
								}
							}
							
						default :
							printf("Error (%s) reading!\n", esp_err_to_name(err));
							return;
					}
					
					// Close handle
					nvs_close(my_handle);	
				}
				
				if (foundflag == 1)  //found the key in system
				{
					//asked for user to input password above
					getstring(str_p, sizeof(str_p));
					if (crypto_pwhash_str_verify(hash_pass, str_p, strlen(str_p)) != 0) 
					{
					printf("\nWrong password!\n");
					continue; /* wrong password */
					}
				}
				
				//On success:
				shellstate = 1; 
				continue;
			}
		}
	
	}
	
	
}


















