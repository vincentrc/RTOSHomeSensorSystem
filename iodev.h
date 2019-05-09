//iodev.h
#ifndef iodev_H_
#define iodev_H_


//Structure for an io device
struct iodev
{
	//DEvice name
	char name[16];

	//Device functions
	void (*open)();
	void (*read)();
	void (*write)(int);
	void (*ctl)();
	void (*close)();
};

//Initializes io table 
void ioinit();

//Device interface api prototypes
int dopen(char *name);
int dread(int fd);
int dwrite(int fd, int on);	//Only for LEDs
int dctl(int fd);
int dclose(int fd);



#endif