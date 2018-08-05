//
// Created by smart on 10/31/17.
//
#include <assert.h>
#include <pthread.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <stdio.h>      /*标准输入输出定义*/
#include <stdlib.h>     /*标准函数库定义*/
#include <unistd.h>     /*Unix 标准函数定义*/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>      /*文件控制定义*/
#include <termios.h>    /*PPSIX 终端控制定义*/
#include <errno.h>      /*错误号定义*/


#include "serial_com.h"
#include "queue.h"


#define FALSE  -1
#define TRUE   0
#define BAUDRATE B230400
//#define BAUDRATE B115200
pthread_attr_t attr;
pthread_t seria_thread = 0;
//static const char *plugin_info = "serial v 1.0";
static pthread_mutex_t serial_lock;
static int serial_fd;

static pthread_mutex_t read_lock;
static Queue *read_queue = NULL;

static pthread_mutex_t write_lock;
static Queue *write_queue = NULL;




struct termios oldtio,newtio;


/*********************************************************************/
static int OpenDev(char *Dev)
{
    int fd = open( Dev, O_RDWR | O_NOCTTY | O_NDELAY);         //| O_NOCTTY | O_NDELAY
    if (-1 == fd)
    {
        perror("Can't Open Serial Port");
        return -1;
    }
    else
        return fd;
}


/**
*@brief  设置串口通信速率
*@param  fd     类型 int  打开串口的文件句柄
*@param  speed  类型 int  串口速度
*@return  void
*/

static int   speed_arr[] = {B230400, B115200, B19200, B9600, B4800, B2400, B1200, B300};
static int   name_arr[] = {230400,115200,  19200,  9600,  4800,  2400,  1200,  300};

static void set_speed(int fd, int speed)
{
    int   i;
    int   status;
    struct termios   Opt;
    tcgetattr(fd, &Opt);
    for ( i= 0;  i < sizeof(speed_arr) / sizeof(int);  i++) {
        if  (speed == name_arr[i]) {
            tcflush(fd, TCIOFLUSH);
            cfsetispeed(&Opt, speed_arr[i]);
            cfsetospeed(&Opt, speed_arr[i]);
            status = tcsetattr(fd, TCSANOW, &Opt);
            if  (status != 0) {
                perror("tcsetattr fd1");
                return;
            }
            tcflush(fd,TCIOFLUSH);
        }
    }
}


/**
*@brief   设置串口数据位，停止位和效验位
*@param  fd     类型  int  打开的串口文件句柄
*@param  databits 类型  int 数据位   取值 为 7 或者8
*@param  stopbits 类型  int 停止位   取值为 1 或者2
*@param  parity  类型  int  效验类型 取值为N,E,O,,S
*/
static int set_Parity(int fd,int databits,int stopbits,int parity)
{
    struct termios options;
    options.c_lflag  &= ~(ICANON | ECHO | ECHOE | ISIG);  /*Input*/
    options.c_oflag  &= ~OPOST;   /*Output*/
    if  ( tcgetattr( fd,&options)  !=  0) {
        perror("SetupSerial 1");
        return(FALSE);
    }
    options.c_cflag &= ~CSIZE;
    switch (databits) /*设置数据位数*/
    {
    case 7:
        options.c_cflag |= CS7;
        break;
    case 8:
        options.c_cflag |= CS8;
        break;
    default:
        fprintf(stderr,"Unsupported data size/n"); return (FALSE);
    }
    switch (parity)
    {
        case 'n':
        case 'N':
            options.c_cflag &= ~PARENB;   /* Clear parity enable */
            options.c_iflag &= ~INPCK;     /* Enable parity checking */
            break;
        case 'o':
        case 'O':
            options.c_cflag |= (PARODD | PARENB); /* 设置为奇效验*/
            options.c_iflag |= INPCK;             /* Disnable parity checking */
            break;
        case 'e':
        case 'E':
            options.c_cflag |= PARENB;     /* Enable parity */
            options.c_cflag &= ~PARODD;   /* 转换为偶效验*/
            options.c_iflag |= INPCK;       /* Disnable parity checking */
            break;
        case 'S':
        case 's':  /*as no parity*/
            options.c_cflag &= ~PARENB;
            options.c_cflag &= ~CSTOPB;break;
        default:
            fprintf(stderr,"Unsupported parity/n");
            return (FALSE);
    }
    /* 设置停止位*/
    switch (stopbits)
    {
        case 1:
            options.c_cflag &= ~CSTOPB;
            break;
        case 2:
            options.c_cflag |= CSTOPB;
           break;
        default:
             fprintf(stderr,"Unsupported stop bits/n");
             return (FALSE);
    }
    /* Set input parity option */
    if (parity != 'n')
        options.c_iflag |= INPCK;
    tcflush(fd,TCIFLUSH);
    options.c_cc[VTIME] = 150; /* 设置超时15 seconds*/
    options.c_cc[VMIN] = 10; /* Update the options and do it NOW */
    /* Make raw */
    //cfmakeraw(&options);

    if (tcsetattr(fd,TCSANOW,&options) != 0)
    {
        perror("SetupSerial 3");
        return (FALSE);
    }
    return (TRUE);
}

int  serial_init(char *pserial_dir)
{
    char *dev  = pserial_dir; //串口0

    serial_fd = OpenDev(dev);
    printf("serial:%s fd :%d\r\n",dev,serial_fd);

    tcgetattr(serial_fd,&oldtio); /* save current port settings */

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CRTSCTS | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 150;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 10;   /* blocking read until 5 chars received */
    
    set_speed(serial_fd,230400);
    
    //set_speed(serial_fd,230400);
    if (set_Parity(serial_fd,8,1,'N') == FALSE)
    {
        printf("Set Parity Error/n");
        return -1;
    }
    printf("Set Parity successfully./n");
    //serial_queue = InitQueue();
    pthread_mutex_init(&serial_lock, NULL);

    return 0;
}

void serial_exit(void)
{
    close(serial_fd);
    //DestroyQueue(serial_queue);
}

int add_login_data(char *pdata)
{
    int len;
    time_t time_stamp;
    struct tm *p;
    char *ptemp = pdata;


    *ptemp++ = 0x23;//'#'
    *ptemp++ = 0xA3;
    len = strlen("TTC_GATEWAY");
    memcpy(ptemp,"TTC_GATEWAY",len);
    ptemp += len;

    time(&time_stamp);
    p=localtime(&time_stamp); /* 获取当前时间 */  
    *(uint16_t *)ptemp = 1900+p->tm_year;
    ptemp += sizeof(uint16_t);
    *ptemp++ = (1+p->tm_mon);
    *ptemp++ = p->tm_mday;
    *ptemp++ = (p->tm_hour+8);
    *ptemp++ = p->tm_min;
    *ptemp++ = p->tm_sec;

    len = ptemp - pdata;
    return len;
}
int add_on_off_data(char *pdata,char on)
{
    int len;
    char *ptemp = pdata;
    *ptemp++ = 0x23;//'#'
    *ptemp++ = 0xA1;
    *ptemp++ = 0x01; //chazuo number
    *ptemp++ = on; // 0x00 or 0x01
    len = ptemp - pdata;
    return len;
}

int add_on_off_withtime_data(char *pdata,char serial_number,char cycle,char on,char s_h,char s_m,char e_h,char e_m)
{
    int len;
    char *ptemp = pdata;
    *ptemp++ = 0x23;//'#'
    *ptemp++ = 0xA2;
    *ptemp++ = serial_number; 
    *ptemp++ = cycle;     
    *ptemp++ = 0x01; 
    *ptemp++ = on;
    *ptemp++ = s_h;
    *ptemp++ = s_m;
    *ptemp++ = e_h;
    *ptemp++ = e_m;
    len = ptemp - pdata;
    return len;
}

int add_set_time_data(char *pdata,uint16_t year,char mon, char data, char hour, char min, char sec)
{
    int len;
    char *ptemp = pdata;
    *ptemp++ = 0x23;//'#'
    *ptemp++ = 0xA0;
    *(uint16_t *)ptemp = htons(year);
    ptemp += 2;
    *ptemp++ = mon;
    *ptemp++ = data;
    *ptemp++ = hour;
    *ptemp++ = min;
    *ptemp++ = sec;
    len = ptemp - pdata;
    return len;

}

int create_command(char *dest_pkg,char*subdev_mac,uint8_t uuid_type,uint8_t overtime,char data_type)
{
    char package[256] = {0};
    char *pos = NULL;
    char *pdata_len = NULL;
    int package_len = 0;
    int i = 0;
    uint8_t sum = 0;

    assert(subdev_mac != NULL);
    pos = package;
    //add sudev mac
    memcpy(pos,subdev_mac,6);
    pos += 6;
    //add uuid type
    *pos++ = uuid_type;
 
    if(uuid_type == 0){
        // service UUID
        *(uint16_t *)pos = htons(0x1000);
        pos += sizeof(uint16_t);
        // features UUID
        *(uint16_t *)pos = htons(0x1001);
        pos += sizeof(uint16_t);     
    }else{
        //128bit service uudi
    }
    // RID
    *(uint16_t *)pos = htons(0x0000);
    pos += sizeof(uint16_t);
    // length default:3
    *pos++ = 0x03;
    // control byte:16bits 
    *(uint16_t *)pos = htons(0x0002);
    pos += sizeof(uint16_t);
    // overtime : value*100ms default:0x50
    *pos++ = overtime;
    // data length and data
    pdata_len = pos;
    pos++;
    switch(data_type){
        case 0x00: //log in
            *pdata_len = add_login_data(pos);
            break;
        case 0x01: // on
            *pdata_len = add_on_off_data(pos,0x01);
            break;
        case 0x02: // off
            *pdata_len = add_on_off_data(pos,0x00);
            break;
        default:
            break;
    }
    pos += *pdata_len;
    package_len = pos - package;
    //check sum
    for(i = 0; i < package_len; i++ )
        sum += (uint8_t)package[i];

    package[package_len] = sum;
    package_len++;
    
    memcpy(dest_pkg,package,package_len);

    printf("gateway cmd:\r\n");
    for(i = 0; i < package_len; i++ ){
        printf("%02X ",(uint8_t)dest_pkg[i]);
    }
    printf("\r\n");
       
    return package_len;
}

static int bledev_login(void)
{
    char buffer[1024];
    int len;
    uint8_t mac[6] = {0xcc,0x78,0xab,0x61,0x55,0xb2};
    char *wpackage = NULL;

    len = create_command(buffer,mac,0,0x50,0x00);
    wpackage = (char *)malloc(len + sizeof(int));
    
    *(int *)wpackage = len;

    memcpy(wpackage + sizeof(int),buffer,len);

    pthread_mutex_lock(&write_lock);
    EnQueue(write_queue,(void*)wpackage);
    pthread_mutex_unlock(&write_lock);
    return 0;
}

void *serial_RW(void *param)
{
    bool run = true;
    int len,fs_sel;
    fd_set fs_read;
    uint8_t nread;
    struct timeval time;
    char buffer[1024] = {0};
    int i = 0;
    void *rpackage = NULL;
    void *wpackage = NULL;
    

    while(run){

        FD_ZERO(&fs_read);
        FD_SET(serial_fd,&fs_read);
        time.tv_sec = 15;
        time.tv_usec = 0;

        fs_sel = select(serial_fd+1,&fs_read,NULL,NULL,&time);
        if(fs_sel) {
            usleep(1200);
            if((len = (int)read(serial_fd,buffer,1024)) != -1){
                if (len == 0) {
                    continue;
                } else{
                    printf("serial.recv_length = %d.\r\n",len);
                    for (i = 0; i < len; ++i)
                    {
                        printf("%02X ",(unsigned char)buffer[i]);
                    }
                    printf("\r\n received a package\r\n");

                    rpackage = (char*)malloc(len + sizeof(int));
                    *(int *)rpackage = len;
                    memcpy(rpackage + sizeof(int),buffer,len);

                    pthread_mutex_lock(&read_lock);
                    EnQueue(read_queue,(void*)rpackage);
                    pthread_mutex_unlock(&read_lock);  
                    rpackage = NULL;               
                }
            }
        }else{
            printf("over time.\r\n ");
            bledev_login();

        }
        if(GetSize(write_queue) > 0){
            printf("check write queue %d.\r\n",GetSize(write_queue));          
            pthread_mutex_lock(&write_lock);            
            DeQueue(write_queue,&wpackage);
            pthread_mutex_unlock(&write_lock);
            len = write(serial_fd,wpackage+sizeof(int),*(int *)wpackage);
            printf(" write package:%d \r\n",len);
            free(wpackage);

        }else{
            usleep(200);
        }

    }
    return NULL;
}


static int8_t dev_data_resolver(uint8_t *data,char len)
{
    uint8_t start;
    uint8_t key_word;
    if(data == NULL && len < 3)
        return -1;
    if(data[0] != '*')
        return -2;
    switch(data[1]){
        case 0xA0:
            printf("change time: %c. \r\n",data[2]);
            break;
        case 0xA1:
            printf(" socket on/off: %c. \r\n",data[2]);
            break;
        case 0xA2:
            printf("socket with time on/off : %c. \r\n",data[2]);
            break;
        case 0xA3:
            printf("dev bind result : %c. \r\n",data[2]);   
            break;
        case 0xA5:
            printf("socket number : %d control: %d. \r\n",data[2],data[3]);   
            break;
        default:
            printf("unknown data type.\r\n");
            break;
    }


    return 0;
}

static uint8_t serial_package_handler(uint8_t *package,size_t len)
{
    int ret = 0;
    int i = 0;
    GATEWAY_PROTO *gateway_proto = NULL;
    DEV_CMD *dev_cmd = NULL;
    char uuit_type = 0;
    uint16_t cmd_seri_number;
    uint16_t contrl_bits;
    uint8_t data_len;

    if(len < sizeof(GATEWAY_PROTO))
        return 0;

    gateway_proto = (GATEWAY_PROTO*)(package + 4);
    printf("ble dev mac:\n");
    for(i = 0;i < 6;i++){
        printf("%02X ",gateway_proto->mac[i]);
    }
    printf("\r\n");
    printf("uuid type:%d \r\n",gateway_proto->uuid_type);
    uuit_type = gateway_proto->uuid_type;
    if(uuit_type == 0){
        printf("service uuid:%d \r\n",gateway_proto->uuid_service);
        
        cmd_seri_number = ntohs(gateway_proto->cmd_seri_number);
        contrl_bits = ntohs(gateway_proto->control_bits);
        if(0x0001 && contrl_bits){
            printf("logout ble.\r\n");
        }
        if(0x0002 && contrl_bits){
            printf("ble need response.\r\n");
        }
        data_len = gateway_proto->length1;
        printf("data length:%d\r\n",data_len);
        dev_data_resolver(gateway_proto->dev_cmd,data_len);
    }
    
    return ret;
}

void *run_serial_service(void *param)
{
    bool run = true;
    char buffer[1024] = {0};
    void *rpackage = NULL;
    void *wpackage = NULL;
    
    /* initialize read and write queue */
    pthread_mutex_init(&read_lock, NULL);
    pthread_mutex_init(&write_lock, NULL);
    read_queue = InitQueue();
    write_queue = InitQueue();

    bledev_login();
     /* initialize detach thread attribution */
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED); 

    if (pthread_create(&seria_thread,&attr,serial_RW,param)) {
        perror("create serial RW thread error.");
        return NULL;
    }
    
    while(run){
        if(GetSize(read_queue) > 0){
            printf("read_queue size = %d \r\n",GetSize(read_queue));
            char *ptemp = NULL;
            int len,i;
            // resolver subdevr
            pthread_mutex_lock(&read_lock);
            DeQueue(read_queue,&rpackage);
            pthread_mutex_unlock(&read_lock);

            // sudev handler    
            ptemp = (char *)rpackage;
            len = *(int *)rpackage;
            printf("serial.rec.package size: %d \r\n",len);
            printf("package: ");
            for (i = 0; i < len; ++i){
                printf("%02X ", *(uint8_t *)(ptemp + 4 + i));
            }
            printf("\r\n");

            serial_package_handler(rpackage,len);

            free(rpackage);
        } 

        usleep(1000);
      

    }
    return NULL;
}

char *get_serial_package(void)
{
   
    void* package = 0;


    return package == 0 ? NULL:(char *)package;
}