#include "config.h"
#include "uci.h"

#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include <uci.h>
 


int load_uci_item(const char *psource,char *result)
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
    //printf("libilop-hal.so %s=%s\n",psource,p.o->v.string);
    uci_free_context(c);
    free(a);

    return 0;
}
#if 0
int load_gateway_conf(CERTIFICATE *pconf)
{
    load_item(UCI_GATEWAY_PK,pconf->ProductKey);
    load_item(UCI_GATEWAY_DN,pconf->DeviceName);
    load_item(UCI_GATEWAY_DS,pconf->DeviceSecret);
    return 0;
}
#endif
