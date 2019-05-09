//actuators.c
#include "driver/gpio.h"
#include "actuators.h"


//Global function pointers
void (*furnace_open_ptr)() = &furnace_open; void (*furnace_write_ptr)(int) = &furnace_write;
void (*ac_open_ptr)() = &ac_open; void (*ac_write_ptr)(int) = &ac_write;
void (*humidifier_open_ptr)() = &humidifier_open; void (*humidifier_write_ptr)(int) = &humidifier_write;
void (*dehumidifier_open_ptr)() = &dehumidifier_open; void (*dehumidifier_write_ptr)(int) = &dehumidifier_write;
void (*aircleaner_open_ptr)() = &aircleaner_open; void (*aircleaner_write_ptr)(int) = &aircleaner_write;
void (*ealarm_open_ptr)() = &ealarm_open; void (*ealarm_write_ptr)(int) = &ealarm_write;


//Open and write function for each "actuator" (LED)

//Furnace
void furnace_open()
{
	gpio_config_t io_conf;

	//set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins to set
    io_conf.pin_bit_mask = 1ULL<<22;
    //configure GPIO with the given settings
    gpio_config(&io_conf);
	
	return;
}

void furnace_write(int on)
{
	//Error
	if(on != 0 && on != 1)
	{
		printf("\nfurnace_write: Invalid Input!\n");
		return;
	}
	
	//Setr the LED level to the passed arg
	gpio_set_level(22, on);
	return;
}


//Air conditioner
void ac_open()
{
	gpio_config_t io_conf;

	//set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins to set
    io_conf.pin_bit_mask = 1ULL<<4;
    //configure GPIO with the given settings
    gpio_config(&io_conf);
	
	return;
}

void ac_write(int on)
{
	//Error
	if(on != 0 && on != 1)
	{
		printf("\nfurnace_write: Invalid Input!\n");
		return;
	}
	
	//Setr the LED level to the passed arg
	gpio_set_level(1, on);
	return;
}


//Humidifier
void humidifier_open()
{
	gpio_config_t io_conf;

	//set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins to set
    io_conf.pin_bit_mask = 1ULL<<5;
    //configure GPIO with the given settings
    gpio_config(&io_conf);
	
	return;
}

void humidifier_write(int on)
{
	//Error
	if(on != 0 && on != 1)
	{
		printf("\nfurnace_write: Invalid Input!\n");
		return;
	}
	
	//Setr the LED level to the passed arg
	gpio_set_level(3, on);
	return;
}


//Dehumidifier
void dehumidifier_open()
{
	gpio_config_t io_conf;

	//set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins to set
    io_conf.pin_bit_mask = 1ULL<<21;
    //configure GPIO with the given settings
    gpio_config(&io_conf);
	
	return;
}

void dehumidifier_write(int on)
{
	//Error
	if(on != 0 && on != 1)
	{
		printf("\nfurnace_write: Invalid Input!\n");
		return;
	}
	
	//Setr the LED level to the passed arg
	gpio_set_level(21, on);
	return;
}


//Air cleaner
void aircleaner_open()
{
	gpio_config_t io_conf;

	//set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins to set
    io_conf.pin_bit_mask = 1ULL<<19;
    //configure GPIO with the given settings
    gpio_config(&io_conf);
	
	return;
}

void aircleaner_write(int on)
{
	//Error
	if(on != 0 && on != 1)
	{
		printf("\nfurnace_write: Invalid Input!\n");
		return;
	}
	
	//Setr the LED level to the passed arg
	gpio_set_level(19, on);
	return;
}


//Emergency Alarm
void ealarm_open()
{
	gpio_config_t io_conf;

	//set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins to set
    io_conf.pin_bit_mask = 1ULL<<18;
    //configure GPIO with the given settings
    gpio_config(&io_conf);
	
	return;
}

void ealarm_write(int on)
{
	//Error
	if(on != 0 && on != 1)
	{
		printf("\nfurnace_write: Invalid Input!\n");
		return;
	}
	
	//Setr the LED level to the passed arg
	gpio_set_level(18, on);
	return;
}




