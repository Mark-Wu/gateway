#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>


#include "ble_subdev.h"
#include "linkkit.h"
#include "iot_import.h"
#include "serial_com.h"
#include "queue.h"

#include "cJSON.h"

#define DPRINT(...)                                      \
do {                                                     \
    printf("\033[1;31;40m%s.%d: ", __func__, __LINE__);  \
    printf(__VA_ARGS__);                                 \
    printf("\033[0m");                                   \
} while (0)




typedef struct {
    int AWituvjBzm;
    char WIFI_Band[255];
    char WiFI_RSSI;
    char WIFI_AP_BSSID[255];
    char WIFI_Channel;
    char WiFI_SNR;

    int connected;
    int lk_dev;
} gateway_t;

static int gateway_get_property(char *in, char *out, int out_len, void *ctx)
{
    printf("gateway property get: %s\n", in);
#if 0
    gateway_t *gw = ctx;
    if (!gw) {
        DPRINT("gateway not found\n");
        return -1;
    }

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

    int i;
    for (i = 0; i < iSize; i++) {
        cJSON *pSub = cJSON_GetArrayItem(rJson, i);

        if (strcmp(pSub->valuestring, "ZB_Band") == 0) {
            cJSON_AddNumberToObject(pJson, "ZB_Band", gw->ZB_Band);
        } else if (strcmp(pSub->valuestring, "ZB_Channel") == 0) {
            cJSON_AddNumberToObject(pJson, "ZB_Channel", gw->ZB_Channel);
        } else if (strcmp(pSub->valuestring, "ZB_CO_MAC") == 0) {
            cJSON_AddStringToObject(pJson, "ZB_CO_MAC", gw->ZB_CO_MAC);
        } else if (strcmp(pSub->valuestring, "ZB_PAN_ID") == 0) {
            cJSON_AddStringToObject(pJson, "ZB_PAN_ID", gw->ZB_PAN_ID);
        } else if (strcmp(pSub->valuestring, "EXT_PAN_ID") == 0) {
            cJSON_AddStringToObject(pJson, "EXT_PAN_ID", gw->EXT_PAN_ID);
        } else if (strcmp(pSub->valuestring, "NETWORK_KEY") == 0) {
            cJSON_AddStringToObject(pJson, "NETWORK_KEY", gw->NETWORK_KEY);
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

static int gateway_set_property(char *in, void *ctx)
{
    gateway_t *gw = ctx;

    printf("gateway set in: %s\n", in);

    cJSON *rJson = cJSON_Parse(in);
    if (!rJson)
        return -1;
    cJSON_Delete(rJson);

    return 0;
}

static int gateway_call_service(char *identifier, char *in, char *out, int out_len, void *ctx)
{
    if (strcmp(identifier, "SetTimerTask") == 0) {
        snprintf(out, out_len, "{\"SetTimer\": \"hello, gateway!\"}");
    } else if (strcmp(identifier, "TimeReset") == 0) {
        DPRINT("TimeReset params: %s\n", in);
    }

    return 0;
}

static int post_all_properties(gateway_t *gw)
{

    printf("gateway post property in .\n");
  
    cJSON *pJson = cJSON_CreateObject();
    if (!pJson)
        return -1;

    cJSON_AddNumberToObject(pJson, "AWituvjBzm",  gw->AWituvjBzm);
    cJSON_AddStringToObject(pJson, "WIFI_Band",  gw->WIFI_Band);
    cJSON_AddNumberToObject(pJson, "WiFI_RSSI",  gw->WiFI_RSSI);
    cJSON_AddStringToObject(pJson, "WIFI_AP_BSSID", gw->WIFI_AP_BSSID);
    cJSON_AddNumberToObject(pJson, "WIFI_Channel",  gw->WIFI_Channel);
    cJSON_AddNumberToObject(pJson, "WiFI_SNR",  gw->WiFI_SNR);
    
    char *p = cJSON_PrintUnformatted(pJson);
    if (!p) {
        cJSON_Delete(pJson);
        return -1;
    }

    printf("property: %s\n", p);

    linkkit_gateway_post_property_json_sync(gw->lk_dev, p, 5000);

    cJSON_Delete(pJson);
    free(p);

    return 0;
}

static int event_handler(linkkit_event_t *ev, void *ctx)
{
    gateway_t *gw = ctx;

    switch (ev->event_type) {
    case LINKKIT_EVENT_CLOUD_CONNECTED:
        {
            printf("cloud connected\n");
            post_all_properties(gw);    /* sync to cloud */
            gw->connected = 1;
        }
        break;
    case LINKKIT_EVENT_CLOUD_DISCONNECTED:
        {
            gw->connected = 0;
            printf("cloud disconnected\n");
        }
        break;
    case LINKKIT_EVENT_SUBDEV_DELETED:
        {
            char *productKey = ev->event_data.subdev_deleted.productKey;
            char *deviceName = ev->event_data.subdev_deleted.deviceName;
            printf("delete subdev %s<%s>\n", productKey, deviceName);
        }
        break;
    case LINKKIT_EVENT_SUBDEV_PERMITED:
        {
            char *productKey = ev->event_data.subdev_permited.productKey;
            int   timeoutSec = ev->event_data.subdev_permited.timeoutSec;
            printf("permit subdev %s in %d seconds\n", productKey, timeoutSec);
        }
        break;
    case LINKKIT_EVENT_RESET_SUCCESS:
        {
            char *productKey = ev->event_data.reset_success.productKey;
            char *deviceName = ev->event_data.reset_success.deviceName;
            printf("device %s<%s> reset success\n", deviceName, productKey);
        }
        break;
    default:
        printf("unknown event type %d\n", ev->event_type);
    }

    return 0;
}

static linkkit_cbs_t alink_cbs = {
    .get_property = gateway_get_property,
    .set_property = gateway_set_property,
    .call_service = gateway_call_service,
    .down_rawdata = NULL,
    .post_rawdata_reply = NULL,
};

static int ota_get_firmware_version(const char *productKey, const char *deviceName, char *version, int buff_len)
{
    snprintf(version, buff_len, "v1.0.0.0");
    return 0;
}

typedef struct {
    char productKey[32];
    char deviceName[64];
    char new_version[64];
    int file_size;
} ota_ctx_t;

static void *ota_start(const char *productKey, const char *deviceName, const char *new_version, int file_size)
{
    ota_ctx_t *octx = malloc(sizeof(ota_ctx_t));
    if (!octx)
        return NULL;
    memset(octx, 0, sizeof(ota_ctx_t));

    strncpy(octx->productKey,  productKey,  sizeof(octx->productKey)  - 1);
    strncpy(octx->deviceName,  deviceName,  sizeof(octx->deviceName)  - 1);
    strncpy(octx->new_version, new_version, sizeof(octx->new_version) - 1);

    octx->file_size = file_size;

    printf("start upgrade, %s<%s> version %s file size %d\n", deviceName, productKey, new_version, file_size);

    return octx;
}

static int ota_write(void *handle, unsigned char *data, int data_len)
{
    ota_ctx_t *octx = (ota_ctx_t *)handle;

    DPRINT("recv %d bytes for %s<%s>\n", data_len, octx->deviceName, octx->productKey);

    return 0;
}

static int ota_stop(void *handle, int err)
{
    ota_ctx_t *octx = (ota_ctx_t *)handle;

    if (err == OTA_UPGRADE_ERROR_NONE)
        DPRINT("download upgrade package success\n");
    else
        DPRINT("download upgrade package failed\n");

    free(octx);

    return 0;
}

static linkkit_ota_params ota_params = {
    .get_firmware_version = ota_get_firmware_version,

    .start = ota_start,
    .write = ota_write,
    .stop  = ota_stop,
};

int main(void)
{

    //memset((char *)&gateway_cert,0x00,sizeof(gateway_cert));
    //load_gateway_conf(&gateway_cert);

    gateway_t gateway;
    memset(&gateway, 0, sizeof(gateway_t));


    /* fill fake gateway network info */
    gateway.AWituvjBzm = 0x00;
    strcpy(gateway.WIFI_Band,"2.4G");
    gateway.WiFI_RSSI = -32;
    strcpy(gateway.WIFI_AP_BSSID,"DK-PC_Network");
    gateway.WIFI_Channel = 7;
    gateway.WiFI_SNR = -23;
    


    linkkit_params_t *initParams = linkkit_gateway_get_default_params();
    if (!initParams)
        return -1;

    /* set maximum message size as 20KB */
    int maxMsgSize = 20 * 1024;
    linkkit_gateway_set_option(initParams, LINKKIT_OPT_MAX_MSG_SIZE, &maxMsgSize, sizeof(int));

    /* set maximum message queue size as 8 */
    int maxMsgQueueSize = 8;
    linkkit_gateway_set_option(initParams, LINKKIT_OPT_MAX_MSG_QUEUE_SIZE, &maxMsgQueueSize, sizeof(int));

    int loglevel = 0;
    linkkit_gateway_set_option(initParams, LINKKIT_OPT_LOG_LEVEL, &loglevel, sizeof(int));

    int threadPoolSize = 4;
    linkkit_gateway_set_option(initParams, LINKKIT_OPT_THREAD_POOL_SIZE, &threadPoolSize, sizeof(int));

    linkkit_gateway_set_event_callback(initParams, event_handler, &gateway);

    if (linkkit_gateway_init(initParams) < 0) {
        DPRINT("linkkit_gateway_init failed\n");
        return -1;
    }
 
    gateway.lk_dev = linkkit_gateway_start(&alink_cbs, &gateway);
    if (gateway.lk_dev < 0) {
        linkkit_gateway_exit();
        DPRINT("linkkit_gateway_start failed\n");
        return -1;
    }

    linkkit_ota_service_init(&ota_params);



    post_all_properties(&gateway);    /* sync to cloud */

    linkkit_gateway_trigger_event_json_sync(gateway.lk_dev, "Error", "{\"ErrorCode\": 0}", 10000);

    ble_subdev_init();
     
    serial_init("/dev/ttyS0");

    run_serial_service(NULL);   /* should be never returned.  */

    ble_subdev_exit();

    linkkit_gateway_stop(gateway.lk_dev);
    linkkit_gateway_exit();

    return 0;
}
