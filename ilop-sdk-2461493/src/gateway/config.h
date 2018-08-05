#ifndef __GATEWAY_CONFIG_H__
#define __GATEWAY_CONFIG_H__
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct{
    char ProductKey[128];
    char DeviceName[128];
    char DeviceSecret[128];
}CERTIFICATE;

int load_gateway_conf(CERTIFICATE *pconf);
int load_subdev_conf(CERTIFICATE *pconf);
#endif




