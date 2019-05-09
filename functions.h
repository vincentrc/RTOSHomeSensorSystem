//functions.h
#ifndef functions_H_
#define functions_H_

#include "nvs_flash.h"
#include "nvs.h"

float ewma(float old, float new, int n);
int nvsget(nvs_handle my_handle, char *key, char value[50], esp_err_t err);
int nvsset(nvs_handle my_handle, char *key, char value[50], esp_err_t err);
void getstring(char *string, int length);
int argparse(char *input, char *key, char *value);

#endif