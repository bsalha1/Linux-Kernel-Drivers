#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>

#define DEVICE "/dev/vbdev"
int main()
{
    int fd = open(DEVICE, O_RDWR);
    // char write_buffer[] = "Hello world!";
    char read_buffer[1024];

    // write(fd, write_buffer, sizeof(write_buffer));
    // ioctl(fd, 0);
    // read(fd, read_buffer, sizeof(read_buffer));
    
    // printf("Read: %s\n", read_buffer);
    close(fd);
    return EXIT_SUCCESS;
}