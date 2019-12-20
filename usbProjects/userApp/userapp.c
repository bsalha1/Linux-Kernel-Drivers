#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#define DEVICE "/dev/pen1"

int main()
{
    int i, fd;
    char ch, writeBuf[100], readBuf[100];

    fd = open(DEVICE, O_RDWR);

    if(fd == -1)
    {
        fprintf(stdout, "File %s failed to open: %s\n", DEVICE, strerror(errno));
        exit(-1);
    }

    fprintf(stdout, "r = read from device\n");
    fprintf(stdout, "w = write to device\n");
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
        default:
            fprintf(stdout, "Command not recognized\n");
            break;
    }

    close(fd);

    return 0;
}
