#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define SERIAL_DEV "/dev/ttyAMA0"
#define IR_TX_DEV "/dev/motor_ctrl"

int main() {
    int serial_fd = open(SERIAL_DEV, O_RDONLY);
    if (serial_fd < 0) {
        perror("Failed to open serial port");
        return 1;
    }

    int ir_fd = open(IR_TX_DEV, O_WRONLY);
    if (ir_fd < 0) {
        perror("Failed to open IR TX device");
        close(serial_fd);
        return 1;
    }

    char buffer[256];
    ssize_t len;

    while (1) {
        len = read(serial_fd, buffer, sizeof(buffer) - 1);
        if (len > 0) {
            buffer[len] = '\0'; 

            char *start = buffer;
            while (*start == ' ' || *start == '\n' || *start == '\r') start++;
            char *end = start + strlen(start) - 1;
            while (end > start && (*end == ' ' || *end == '\n' || *end == '\r')) *end-- = '\0';

            if (strlen(start) > 0) {
                printf("Received command: %s\n", start);

                dprintf(ir_fd, "%s\n", start); 
            }
        } else {
            usleep(100000);
        }
    }

    close(serial_fd);
    close(ir_fd);

    return 0;
}

