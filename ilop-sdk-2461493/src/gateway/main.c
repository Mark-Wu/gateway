
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zconf.h>

#define BAUDRATE B115200
#define MODEMDEVICE "/dev/ttyS0"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

volatile int STOP=FALSE;


unsigned char send_buff[] = {0xCC,0x78,0xAB,0x61,0x55,0xB2 ,0x00 ,0x10,0x00 ,0x10,0x01 ,0x00,0x00 ,0x03 ,0x00,0x02 ,0x0A ,0x04 ,0x23,0xA1,0x01,0x01 ,0x51};


int seria_read()
{
    int fd,c, res;
    struct termios oldtio,newtio;
    char buf[255];

    fd = open(MODEMDEVICE, O_RDWR | O_NOCTTY );
    if (fd <0) {perror(MODEMDEVICE); exit(-1); }

    tcgetattr(fd,&oldtio); /* save current port settings */

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CRTSCTS | CS8 | CLOCAL | CREAD;

    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 150;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 6;   /* blocking read until 5 chars received */

    tcflush(fd, TCIFLUSH);
    tcsetattr(fd,TCSANOW,&newtio);


    while (STOP==FALSE) {       /* loop for input */
        res = read(fd,buf,255);   /* returns after 5 chars have been input */
        buf[res]=0;               /* so we can printf... */
        int i;
        for (i = 0; i < res; ++i){
            printf("%02X ", *(unsigned char *)(buf + i));
        }
        printf("\r\n");
        res = write(fd,send_buff,sizeof(send_buff));
        printf("send len = %d\r\n",res);

        if (buf[0]=='z') STOP=TRUE;
    }
    tcsetattr(fd,TCSANOW,&oldtio);
}

int serial_write()
{
    return 0;
}

int main(int argc,char **argv)
{
    seria_read();
    return 0;
}