
#ifndef SERIAL_UTILS_H
#define SERIAL_UTILS_H

#include <termios.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int open_serial_port(const char *portname) {
    int fd = open(portname, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd == -1) { perror("Port closed"); return -1; }

    struct termios tty;
    tcgetattr(fd, &tty); 

    
    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag |= CS8;   
    tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    tty.c_oflag &= ~OPOST;  

    tcsetattr(fd, TCSANOW, &tty); 
    return fd;
}

#endif