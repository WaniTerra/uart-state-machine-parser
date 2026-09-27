#define _XOPEN_SOURCE 600
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

typedef struct __attribute__((packed)) RocketData {
    uint32_t timestamp;
    float altitude;
    float speed;
} RocketData;

int main() {
    int master_fd = posix_openpt(O_RDWR | O_NOCTTY);
    if (master_fd == -1) { perror("openpt"); return 1; }

    grantpt(master_fd);
    unlockpt(master_fd);

    
    printf("Port: %s\n", ptsname(master_fd));

    RocketData data = {0, 0.0, 0.0};
    uint8_t tx_buffer[64]; 
    while(1) {
        data.timestamp = (uint32_t)time(NULL);
        data.altitude += 5.5;
        data.speed += 2.0;

        tx_buffer[0] = 0xAA;
        tx_buffer[1] = 0xBB;

        memcpy(&tx_buffer[2], &data, sizeof(RocketData));

        uint8_t checksum = 0;
      
        for(int i = 0; i < sizeof(RocketData); i++) {
            checksum ^= tx_buffer[2 + i];
        }
       
        tx_buffer[2 + sizeof(RocketData)] = checksum;

        
        int total_len = 2 + sizeof(RocketData) + 1;
        write(master_fd, tx_buffer, total_len);

        printf("[TX] Sended: %.2f m\n", data.altitude);
        usleep(100000); 
    }
    return 0;
}