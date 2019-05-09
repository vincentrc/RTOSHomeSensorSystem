//actuators.h
#ifndef actuators_H_
#define actuators_H_

void furnace_open();
void furnace_write(int on);
void ac_open();
void ac_write(int on);
void humidifier_open();
void humidifier_write(int on);
void dehumidifier_open();
void dehumidifier_write(int on);
void aircleaner_open();
void aircleaner_write(int on);
void ealarm_open();
void ealarm_write(int on);


#endif