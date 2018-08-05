#include "config.h"
#include "uci.h"

#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include <uci.h>


#define UCI_GATEWAY_PK  "ali_config.@gateway[0].ProductKey"
#define UCI_GATEWAY_DN  "ali_config.@gateway[0].DeviceName"
#define UCI_GATEWAY_DS  "ali_config.@gateway[0].DeviceSecret"  

#define UCI_SUBDEV_PK  "ali_config.@subdev[0].ProductKey"
#define UCI_SUBDEV_DN  "ali_config.@subdev[0].DeviceName"
#define UCI_SUBDEV_DS  "ali_config.@subdev[0].DeviceSecret" 

static int load_item(const char *psource,char *result)
{
    struct uci_context *c;
    struct uci_ptr p;
    char *a = strdup(psource); 
    c = uci_alloc_context();
    if(UCI_OK != uci_lookup_ptr(c, &p, a, true)){
        uci_perror(c, "no found!\n");
        return -1;
    }
    memcpy(result,p.o->v.string,strlen(p.o->v.string));
    printf("%s=%s\n",psource,p.o->v.string);
    uci_free_context(c);
    free(a);

    return 0;
}
int load_gateway_conf(CERTIFICATE *pconf)
{
    load_item(UCI_GATEWAY_PK,pconf->ProductKey);
    load_item(UCI_GATEWAY_DN,pconf->DeviceName);
    load_item(UCI_GATEWAY_DS,pconf->DeviceSecret);
    return 0;
}

int load_subdev_conf(CERTIFICATE *pconf)
{
    load_item(UCI_SUBDEV_PK,pconf->ProductKey);
    load_item(UCI_SUBDEV_DN,pconf->DeviceName);
    load_item(UCI_SUBDEV_DS,pconf->DeviceSecret);
    return 0;
}
