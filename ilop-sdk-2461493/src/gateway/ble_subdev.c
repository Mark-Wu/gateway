#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>

#include "linkkit.h"
#include "ble_subdev.h"
#include "cJSON.h"
#include "serial_com.h"

#define NELEMS(x)   (sizeof(x) / sizeof((x)[0]))

#define DPRINT(...)                                      \
do {                                                     \
    printf("\033[1;31;40m%s.%d: ", __func__, __LINE__);  \
    printf(__VA_ARGS__);                                 \
    printf("\033[0m");                                   \
} while (0)


typedef struct {
    int  devid;
    char productKey[32];
    char deviceName[64];
    char deviceSecret[64];

    int PowerSwitch;
    struct{
        char Timer[256];
        bool Enable;
        bool IsValid;
        }Localtime;
} socket_t;

typedef struct {
    char *productKey;
    char *deviceName;
    char *deviceSecret;
} socket_conf_t;

static const socket_conf_t socket_maps[] = {
    {"a1JG1cEzNl4","CC78AB6155B2","7CeXR4kwb37w9EC7sVZETxd7rKzJihPi"},
};

static socket_t *sockets[1];

static int socket_get_property(char *in, char *out, int out_len, void *ctx)
{
    printf("socket get property in: %s\n", in);
    int i;
    socket_t *socket = ctx;
#if 0
    cJSON *rJson = cJSON_Parse(in);
    if (!rJson)
        return -1;

    int iSize = cJSON_GetArraySize(rJson);
    if (iSize <= 0) {
        cJSON_Delete(rJson);
        return -1;
    }

    cJSON *pJson = cJSON_CreateObject();
    if (!pJson) {
        cJSON_Delete(rJson);
        return -1;
    }
    for (i = 0; i < iSize; i++) {
        cJSON *pSub = cJSON_GetArrayItem(rJson, i);
        cJSON *array = NULL,arrayobj = NULL;
        if (strcmp(pSub->valuestring, "PowerSwitch") == 0) {
            cJSON_AddNumberToObject(pJson, "PowerSwitch", sockets[i]->PowerSwitch);
        } else if (strcmp(pSub->valuestring, "LocalTimer") == 0) {
            cJSON_AddItemToObject(pJson,"LocalTimer",array = cJSON_CreateArray());
            cJSON_AddItemToArray(array,arrayobj);

            arrayobj = cJSON_CreateObject();
            cJSON_AddItemToArray(array,arrayobj);
            cJSON_AddItemToObject(arrayobj,"Timer",cJSON_CreateString(sockets[0]->Localtime.Timer));

            arrayobj = cJSON_CreateObject();
            cJSON_AddItemToArray(array,arrayobj);
            cJSON_AddItemToObject(arrayobj, "Enable",  cJSON_CreateBool(sockets[0]->Localtime.Enable));

            arrayobj = cJSON_CreateObject();
            cJSON_AddItemToArray(array,arrayobj);    
            cJSON_AddItemToObject(arrayobj, "IsValid",  cJSON_CreateBool(sockets[0]->Localtime.IsValid));  

        }
    }

    char *p = cJSON_PrintUnformatted(pJson);
    if (!p) {
        cJSON_Delete(rJson);
        cJSON_Delete(pJson);
        return -1;
    }

    if (strlen(p) >= out_len) {
        cJSON_Delete(rJson);
        cJSON_Delete(pJson);
        free(p);
        return -1;
    }

    strcpy(out, p);

    DPRINT("out: %s\n", out);

    cJSON_Delete(rJson);
    cJSON_Delete(pJson);
    free(p);
#endif
    return 0;
}

static int socket_set_property(char *in, void *ctx)
{
    int ArraySize,i;
    printf("socket set property in: %s\n", in);

    socket_t *socket = ctx;

    printf("socket set %s.%s: in %s\n", socket->productKey, socket->deviceName, in);

    cJSON *rJson = cJSON_Parse(in);
    if (!rJson)
        return -1;

    cJSON *PowerSwitch = cJSON_GetObjectItem(rJson, "PowerSwitch");
    if (PowerSwitch)
        socket->PowerSwitch = PowerSwitch->valueint;

    cJSON *LocalTimer = cJSON_GetObjectItem(rJson, "LocalTimer");
    if(LocalTimer){
        ArraySize = cJSON_GetArraySize(LocalTimer);
        for(i=0;i<ArraySize;i++){
            cJSON *objread = cJSON_GetArrayItem(LocalTimer,i);
            cJSON *itemread = NULL;
            itemread = cJSON_GetObjectItem(objread,"Timer");
            if( itemread != NULL){
                memset(socket->Localtime.Timer,0x00,256);
                memcpy(socket->Localtime.Timer,itemread->valuestring,strlen(itemread->valuestring));
                printf("property %d timer:%s \r\n",i,itemread->valuestring);
            }
            itemread = cJSON_GetObjectItem(objread,"Enable");
            if( itemread != NULL){
                socket->Localtime.Enable = itemread->valueint;
                printf("property %d Enable:%d \r\n",i,itemread->valueint);
            }
            itemread = cJSON_GetObjectItem(objread,"IsValid");
            if( itemread != NULL){
                socket->Localtime.IsValid = itemread->valueint;
                printf("property %d IsValid:%d \r\n",i,itemread->valueint);
            }
            
        }
    }
    cJSON_Delete(rJson);

    linkkit_gateway_post_property_json_sync(socket->devid, in, 10000);

    return 0;
}

static int socket_call_service(char *identifier, char *in, char *out, int out_len, void *ctx)
{

    printf("socket call services. \r\n");
    socket_t *socket = ctx;
    printf("%s.%s: in %s\n", socket->productKey, socket->deviceName, in); 

    cJSON *rJson = cJSON_Parse(in);
    if (!rJson)
        return -1;

    cJSON *dataTime = cJSON_GetObjectItem(rJson, "dataTime");
    if (dataTime){
        
        //set ble dev time
        printf("services set subdeb time 'localtime' as return.\r\n");
        linkkit_gateway_post_property_json_sync(sockets[0]->devid, "{\"localtime\": \"12345678\"}", 5000);

    }

    cJSON *socketSwitch = cJSON_GetObjectItem(rJson, "socketSwitch");
    cJSON *Action = cJSON_GetObjectItem(rJson, "Action");
    if(socketSwitch && Action){
        cJSON *actionTime = cJSON_GetObjectItem(rJson, "actionTime");
        if(actionTime){
            // socket switch action with time

            linkkit_gateway_post_property_json_sync(sockets[0]->devid, "{\"result\": 1}", 5000);

        }else{
            // socket switch action without time

            linkkit_gateway_post_property_json_sync(sockets[0]->devid, "{\"result\": 1}", 5000);
        }


    }


    linkkit_gateway_post_property_json_sync(sockets[0]->devid, "{\"SetTimer\": \"hello, world!\"}", 5000);

    cJSON_Delete(pJson);
    return 0;
}

static linkkit_cbs_t socket_cbs = {
    .get_property = socket_get_property,
    .set_property = socket_set_property,
    .call_service = socket_call_service,
};



int ble_subdev_init(void)
{
    int i;
    for (i = 0; i < 1; i++) {
        socket_t *socket = malloc(sizeof(socket_t));
        if (!socket)
            break;
        memset(socket, 0, sizeof(socket_t));

        const socket_conf_t *conf = &socket_maps[i];

        strncpy(socket->productKey,   conf->productKey,   sizeof(socket->productKey) - 1);
        strncpy(socket->deviceName,   conf->deviceName,   sizeof(socket->deviceName) - 1);
        strncpy(socket->deviceSecret, conf->deviceSecret, sizeof(socket->deviceSecret) - 1);

        socket->PowerSwitch = 1;
        memset(socket->Localtime.Timer,0x00,256);
        sprintf(socket->Localtime.Timer,"%ld",time(NULL));
        socket->Localtime.Enable = true;
        socket->Localtime.IsValid = true;

        if (linkkit_gateway_subdev_register(socket->productKey, socket->deviceName, socket->deviceSecret) < 0) {
            free(socket);
            break;
        }

        socket->devid = linkkit_gateway_subdev_create(socket->productKey, socket->deviceName, &socket_cbs, socket);
        if (socket->devid < 0) {
            DPRINT("linkkit_gateway_subdev_create %s<%s> failed\n", socket->deviceName, socket->productKey);
            linkkit_gateway_subdev_unregister(socket->productKey, socket->deviceName);
            free(socket);
            break;
        }

        if (linkkit_gateway_subdev_login(socket->devid) < 0) {
            DPRINT("linkkit_gateway_subdev_login %s<%s> failed\n", socket->deviceName, socket->productKey);
            linkkit_gateway_subdev_destroy(socket->devid);
            linkkit_gateway_subdev_unregister(socket->productKey, socket->deviceName);
            free(socket);
            break;
        }

        sockets[i] = socket;
    }


    return 0;
}

int ble_subdev_exit(void)
{
    int i;
    for (i = 0; i < 1; i++) {
        socket_t *socket = sockets[i];
        if (!socket)
            continue;

        linkkit_gateway_subdev_logout(socket->devid);
        linkkit_gateway_subdev_destroy(socket->devid);
        linkkit_gateway_subdev_unregister(socket->productKey, socket->deviceName);
        free(socket);

        sockets[i] = NULL;
    }

    return 0;
}


#if 0
int ble_subdev_post_properties(void)
{
    printf("\r\n\r\nble subdev post property in .\n");
    
    cJSON *pJson = cJSON_CreateObject();
    if (!pJson)
        return -1;
    cJSON* array = NULL;
    cJSON *arrayobj = NULL;

    cJSON_AddBoolToObject(pJson, "PowerSwitch",  sockets[0]->PowerSwitch);
    cJSON_AddItemToObject(pJson,"LocalTimer",array = cJSON_CreateArray());
    cJSON_AddItemToArray(array,arrayobj);

    arrayobj = cJSON_CreateObject();
    cJSON_AddItemToArray(array,arrayobj);
    cJSON_AddItemToObject(arrayobj,"Timer",cJSON_CreateString(sockets[0]->Localtime.Timer));

    arrayobj = cJSON_CreateObject();
    cJSON_AddItemToArray(array,arrayobj);
    cJSON_AddItemToObject(arrayobj, "Enable",  cJSON_CreateBool(sockets[0]->Localtime.Enable));

    arrayobj = cJSON_CreateObject();
    cJSON_AddItemToArray(array,arrayobj);    
    cJSON_AddItemToObject(arrayobj, "IsValid",  cJSON_CreateBool(sockets[0]->Localtime.IsValid));  
   
    char *p = cJSON_PrintUnformatted(pJson);
    if (!p) {
        cJSON_Delete(pJson);
        return -1;
    }
    printf("subdev id:%d \r\n",sockets[0]->devid);
    printf("subdev property: %s\n", p);

    int ret = linkkit_gateway_post_property_json_sync(sockets[0]->devid, p, 5000);

    cJSON_Delete(pJson);
    free(p);

    return 0;
}
#endif