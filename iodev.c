//iodev.c
#include <string.h>
#include "iodev.h"
#include "dht.h"
#include "gas.h"
#include "particle.h"
#include "photo.h"

#define filetablesize 12

//External function pointers
extern void (*dht_open_ptr)(); extern void (*dht_read_ptr)(); extern void (*dht_ctl_ptr)(); extern void (*dht_close_ptr)();
extern void (*particle_open_ptr)(); extern void (*particle_read_ptr)(); extern void (*particle_ctl_ptr)(); extern void (*particle_close_ptr)();
extern void (*photo_open_ptr)(); extern void (*photo_read_ptr)(); extern void (*photo_ctl_ptr)(); extern void (*photo_close_ptr)();
extern void (*gas_open_ptr)(); extern void (*gas_read_ptr)(); extern void (*gas_ctl_ptr)(); extern void (*gas_close_ptr)();
extern void (*furnace_open_ptr)(); extern void (*furnace_write_ptr)(int);
extern void (*ac_open_ptr)(); extern void (*ac_write_ptr)(int);
extern void (*humidifier_open_ptr)(); extern void (*humidifier_write_ptr)(int);
extern void (*dehumidifier_open_ptr)(); extern void (*dehumidifier_write_ptr)(int);
extern void (*aircleaner_open_ptr)(); extern void (*aircleaner_write_ptr)(int);
extern void (*ealarm_open_ptr)(); extern void (*ealarm_write_ptr)(int);


//io device table (global)
struct iodev iotable[12];

//io device table init function
void ioinit()
{
	strcpy(iotable[0].name, "photo"); iotable[0].open = photo_open_ptr; iotable[0].read = photo_read_ptr; iotable[0].ctl = photo_ctl_ptr; iotable[0].close = photo_close_ptr;
	strcpy(iotable[1].name, "gas"); iotable[1].open = gas_open_ptr; iotable[1].read = gas_read_ptr; iotable[1].ctl = gas_ctl_ptr; iotable[1].close = gas_close_ptr;
	strcpy(iotable[2].name, "particle"); iotable[2].open = particle_open_ptr; iotable[2].read = particle_read_ptr; iotable[2].ctl = particle_ctl_ptr; iotable[2].close = particle_close_ptr;
	strcpy(iotable[3].name, "dht"); iotable[3].open = dht_open_ptr; iotable[3].read = dht_read_ptr; iotable[3].ctl = dht_ctl_ptr; iotable[3].close = dht_close_ptr;
	strcpy(iotable[4].name, "furnace"); iotable[4].open = furnace_open_ptr; iotable[4].write = furnace_write_ptr; 
	strcpy(iotable[5].name, "ac"); iotable[5].open = ac_open_ptr; iotable[5].write = ac_write_ptr;
	strcpy(iotable[6].name, "humidifier"); iotable[6].open = humidifier_open_ptr; iotable[6].write = humidifier_write_ptr; 
	strcpy(iotable[7].name, "dehumidifier"); iotable[7].open = dehumidifier_open_ptr; iotable[7].write = dehumidifier_write_ptr; 
	strcpy(iotable[8].name, "air cleaner"); iotable[8].open = aircleaner_open_ptr; iotable[8].write = aircleaner_write_ptr; 
	strcpy(iotable[9].name, "e-alarm"); iotable[9].open = ealarm_open_ptr; iotable[9].write = ealarm_write_ptr;
}



//File table (2d array of void function pointers)
//Each row is for a device and each column is for a function (open, read, ctl, close)
void (*filetable[12][4])() = 
{{NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL}};

//Separate file table for the write function due to different arguments (maps the same)
void (*writefiletable[12])(int) = 
{ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };




//Device interface wrapper api

//Opens the device with the given name, puts it on the file table and returns its file descriptor
//Returns -1 on error
int dopen(char *name)
{
	int x, y;

	//Find device in iotable
	for(x=0;x<=10;x++)
	{
		if(x == 10)	//Device not found
		{
			printf("\ndopen: No device exists with that name!\n");
			return -1;
		}

		if(!strcmp(name, iotable[x].name))	//Device found
			break;
	}
	
	//Find first empty row in file table
	y = 0;
	while(*filetable[y] != NULL)
	{ y++; }

	//If there is no room
	if (y >= filetablesize)
	{
		printf("\ndopen: File table full!\n");
		return -1;
	}

	//Place the device functions in the file table
	filetable[y][0] = iotable[x].open;
	filetable[y][1] = iotable[x].read;
	filetable[y][2] = iotable[x].ctl;
	filetable[y][3] = iotable[x].close;
	writefiletable[y] = iotable[x].write;

	//Call the open function for that device
	(*filetable[y][0])();
	
	//Return row index as file descriptor
	return y;
}

//Calls the read function for the device with the given file descriptor, return -1 on error
int dread(int fd)
{
	//Check if fd is valid (By checking the file table row and read and write functions)
	if (*filetable[fd] == NULL)
	{
		printf("\ndread: Invalid file descriptor!\n");
		return -1;
	}

	//Check if the read function exists for that device (won't for LEDs)
	else if (*filetable[fd][1] == NULL)
	{
		printf("\ndread: This device has no read function!\n");
		return -1;
	}

	//Call the read function for that device
	(*filetable[fd][1])();

	return 0;	//No error
}

//Calls the write function for the given file descriptor, returns -1 on error
int dwrite(int fd, int on)	//Only for LEDs
{
	//Check if fd is valid 
	if (*filetable[fd] == NULL )
	{
		printf("\ndwrite: Invalid file descriptor!\n");
		return -1;
	}

	//Check if the write function exists for that device (only will for LEDs)
	else if (*writefiletable[fd] == NULL)
	{
		printf("\ndwrite: This device has no write function!\n");
		return -1;
	}

	//Call the write function for that device
	(*writefiletable[fd])(on);

	return 0; //No error
}

//Calls the ctl function for the given file descriptor, returns -1 on error
int dctl(int fd)
{
	//Check if fd is valid 
	if (*filetable[fd] == NULL )
	{
		printf("\ndctl: Invalid file descriptor!\n");
		return -1;
	}

	//Check if the ctl function exists for that device (won't for LEDs)
	else if (*filetable[fd][3] == NULL)
	{
		printf("\ndctl: This device has no ctl function!\n");
		return -1;
	}

	//Call the ctl function for that device
	(*filetable[fd][3])();

	return 0;	//No error
}

//Calls the close function for the given file descriptor, returns -1 on error
int dclose(int fd)
{
	//Check if fd is valid 
	if (*filetable[fd] == NULL )
	{
		printf("\ndclose: Invalid file descriptor!\n");
		return -1;
	}

	//Check if the close function exists for that device
	else if (*filetable[fd][3] == NULL)
	{
		printf("\ndclose: This device has no close function!\n");
		return -1;
	}

	//Call the close function for that device
	(*filetable[fd][3])();

	return 0;	//No error
}






