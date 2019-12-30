#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>

#include "ioctl.h"

#define DEVICE "/dev/char_device"

int main()
{
    int i, fd;
    long ioctl_ret;
    char ch, writeBuf[100], readBuf[100];

    fd = open(DEVICE, O_RDWR);

    if(fd == -1)
    {
        fprintf(stdout, "File %s failed to open: %s\n", DEVICE, strerror(errno));
        exit(-1);
    }
    fprintf(stdout, "r = read from device\n");
    fprintf(stdout, "w = write to device\n");
    fprintf(stdout, "0 = handshake\n");
    fprintf(stdout, "1 = get major number\n");
    fprintf(stdout, "2 = get minor number\n");
    fprintf(stdout, "3 = get remaining write size\n");
    fprintf(stdout, "Enter command: ");
    fscanf(stdin, "%c", &ch);

    switch(ch)
    {
        case 'w':
            fprintf(stdout, "Enter data: ");
            fscanf(stdin, " %[^\n]", writeBuf);
            write(fd, writeBuf, sizeof(writeBuf));
            break;

        case 'r':
            read(fd, readBuf, sizeof(readBuf));
            fprintf(stdout, "Device: %s\n", readBuf);
            break;

	    case '0':
            ioctl_ret = ioctl(fd, HANDSHAKE);
	        if(ioctl_ret < 0)
            {
                fprintf(stdout, "Ioctl failed to send\n");
                break;
            }
	        fprintf(stdout, "Return: %ld\n", ioctl_ret);
            break;

        case '1':
            ioctl_ret = ioctl(fd, GET_MAJOR_NUMBER);
	        if(ioctl_ret < 0)
            {
                fprintf(stdout, "Ioctl failed to send\n");
                break;
            }
	        fprintf(stdout, "Return: %ld\n", ioctl_ret);
            break;

        case '2':
            ioctl_ret = ioctl(fd, GET_MINOR_NUMBER);
	        if(ioctl_ret < 0)
            {
                fprintf(stdout, "Ioctl failed to send\n");
                break;
            }
	        fprintf(stdout, "Return: %ld\n", ioctl_ret);
            break;

        case '3':
            ioctl_ret = ioctl(fd, GET_REMAINING_SIZE);
	        if(ioctl_ret < 0)
            {
                fprintf(stdout, "Ioctl failed to send\n");
                break;
            }
	        fprintf(stdout, "Return: %ld\n", ioctl_ret);
            break;
    
        default:
            fprintf(stdout, "Command not recognized\n");
            break;
    }

    close(fd);

    return 0;
}
