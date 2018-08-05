#ifndef __GATEWAY_CONFIG_H__
#define __GATEWAY_CONFIG_H__
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#if 0
typedef struct{
    char ProductKey[128];
    char DeviceName[128];
    char DeviceSecret[128];
}CERTIFICATE;
#endif

#define UCI_SUBDEV_PK  "ali_config.@subdev[0].ProductKey"
#define UCI_SUBDEV_DN  "ali_config.@subdev[0].DeviceName"
#define UCI_SUBDEV_DS  "ali_config.@subdev[0].DeviceSecret" 

#define UCI_GATEWAY_PK  "ali_config.@gateway[0].ProductKey"
#define UCI_GATEWAY_DN  "ali_config.@gateway[0].DeviceName"
#define UCI_GATEWAY_DS  "ali_config.@gateway[0].DeviceSecret"  

int load_uci_item(const char *psource,char *result);
#endif




