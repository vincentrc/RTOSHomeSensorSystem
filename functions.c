#include <string.h>
#include "functions.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "soc/timer_group_struct.h" 
#include "soc/timer_group_reg.h"

//Exponentially Weighted Moving Average Function
float ewma(float old, float new, int n)
{
	float alpha = ((float)2/(n + 1));
	float updated = alpha*new + (1 - alpha)*old;
	return updated;	
}

//Function for getting a value from NV stroage given the key
int nvsget(nvs_handle my_handle, char *key, char value[50], esp_err_t err)
{
	
	//Open NVS handle
  err = nvs_open("storage", NVS_READWRITE, &my_handle);
  if (err != ESP_OK)
  {
    printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
  	return 1;
  }
  else
  {
    size_t size = 128; //I arbitrarily chose this size
    //Read the nvs handle for given key and place the value in the value variable
    err = nvs_get_str(my_handle, key, value, &size);
    switch (err)
    {
            case ESP_OK:  //Found the variable successfully 
                break;  //pass

            case ESP_ERR_NVS_NOT_FOUND: //Not found/initialized
                printf("\nNVS key does not exist!\n");
                return 1; //1 for error

            default :
                printf("Error (%s) reading!\n", esp_err_to_name(err));
                return 1; //1 for error
    }

    // Close handle
    nvs_close(my_handle);
  }
  
  return 0; //0 for no error
}

//Function for storing value in NV storage under given key
int nvsset(nvs_handle my_handle, char *key, char *value, esp_err_t err)
{
	//Open NVS handle
  err = nvs_open("storage", NVS_READWRITE, &my_handle);
  if (err != ESP_OK)
  {
    printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
  	return 1;	//1 for error
  }
  else
  {
	//Store value under given key 
	err = nvs_set_str(my_handle, key, value);
	if (err != ESP_OK)	//Failed
	{
		printf("\nFailed to store new value in memory!\n");
		return 1; //1 for error
	}
	else //All is well, commit the changes
	{
		err = nvs_commit(my_handle);
		if (err != ESP_OK)  //Failed
		{
			printf("\nFailed to commit new password storage!\n");
			return 1; //1 for error
		}
	}
	  
    // Close handle
    nvs_close(my_handle);
  }
  
  return 0; //0 for no error
}

//Getstring definition
void getstring(char *string, int length)
{
	char ch;
	int i;
	
	//Gets-like function using getchar
	i = 0;
	while(i<length-1) //-1 is to make room for the null terminator \0
	{
		ch = getchar();
		
		//Only take valid characters as input
		if (ch >= 0 && ch <= 127)
		{
			//If the character was the enter key, break
			if (ch == 10)
				break;
			
			//Otherwise add to the string and keep looking
			else
			{
				string[i] = ch;
				i++;
			}
		}
		
		//This stops the watchdog timer from closing the program 
		TIMERG0.wdt_wprotect=TIMG_WDT_WKEY_VALUE;
		TIMERG0.wdt_feed=1;
		TIMERG0.wdt_wprotect=0;
	}
	
	//Append the null terminator at the end
	string[i] = '\0';
	
	return;
}

//Function to parse a "set name=value" string and place the name and val in strings
int argparse(char *input, char *key, char *value)
{
	char *token;
	
	//First parse the string by spaces
	token = strtok(input, " ");
	
	
	if(strcmp(token, "set") != 0) //Check first arg is set
		return 1; //1 for error
	
	else
	{
		token = strtok(NULL, "=");
		
		if(token == NULL)
			return 1; //1 for error
		else
		{
			strcpy(key, token);	//Save the key in key
			
			token = strtok(NULL, "="); 
			if(token == NULL)
				return 1; //1 for error
			
			strcpy(value, token); //Save the value in value
			
			return 0; //0 for no error
		}
		
	}
}










