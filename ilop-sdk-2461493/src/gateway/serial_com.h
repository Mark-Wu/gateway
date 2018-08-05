//
// Created by smart on 10/31/17.
//

#ifndef SERIAL_COM_H
#define SERIAL_COM_H

#include <stdio.h>      /*标准输入输出定义*/
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>     /*标准函数库定义*/
#include <unistd.h>     /*Unix 标准函数定义*/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>      /*文件控制定义*/
#include <termios.h>    /*PPSIX 终端控制定义*/
#include <errno.h>      /*错误号定义*/
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include <stdint.h>


typedef struct{
	uint8_t mac[6];
	uint8_t uuid_type;
	uint16_t uuid_service;
	uint16_t uuid_features;
	uint16_t cmd_seri_number;
	uint8_t length;
	uint16_t control_bits;
	uint8_t overtime;
	uint8_t length1;
	uint8_t *dev_cmd[0];
}__attribute__((__packed__))GATEWAY_PROTO;


typedef struct{


}__attribute__((__packed__))DEV_CMD;





int create_command(char *dest_pkg,char*subdev_mac,uint8_t uuid_type,uint8_t overtime,char data_type); 
const char *get_serial_info(void);
int serial_init(char *pserial_dir);
void *run_serial_service(void *param);
void serial_exit(void);

#endif //SERIAL_COM_H
