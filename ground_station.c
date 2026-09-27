#include <pthread.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include "serial_utils.h"

typedef struct __attribute__((packed)) RocketData
{
    uint32_t timestamp;
    float altitude;
    float speed;
} RocketData;

typedef struct
{
    RocketData buffer[10];
    int head;
    int tail;
} RingBuffer;

RingBuffer rb = {.head = 0, .tail = 0};
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
int serial_fd;

typedef enum
{
    WAIT_HEAD1,
    WAIT_HEAD2,
    READ_DATA,
    CHECK_CS
} State;

void *receiver_thread(void *arg)
{
    State state = WAIT_HEAD1;
    uint8_t byte;
    uint8_t recv_buf[sizeof(RocketData)];
    int idx = 0;
    uint8_t calc_cs = 0;

    printf("[Receiver] Data waiting...\n");

    while (1)
    {

        int n = read(serial_fd, &byte, 1);
        if (n <= 0)
        {
            usleep(100);
            continue;
        }

        switch (state)
        {
        case WAIT_HEAD1:
            if (byte == 0xAA)
                state = WAIT_HEAD2;
            break;

        case WAIT_HEAD2:
            if (byte == 0xBB)
            {
                state = READ_DATA;
                idx = 0;
                calc_cs = 0;
            }
            else
            {
                state = WAIT_HEAD1;
            }
            break;

        case READ_DATA:
            recv_buf[idx++] = byte;
            calc_cs ^= byte;
            if (idx == sizeof(RocketData))
            {
                state = CHECK_CS;
            }
            break;

        case CHECK_CS:
            if (byte == calc_cs)
            {
                pthread_mutex_lock(&mutex);
                memcpy(&rb.buffer[rb.head], recv_buf, sizeof(RocketData));
                rb.head = (rb.head + 1) % 10;
                pthread_cond_signal(&cond);
                pthread_mutex_unlock(&mutex);
            }
            else
            {
                printf("[RX ERROR] Garbage data \n");
            }
            state = WAIT_HEAD1;
            break;
        }
    }
    return NULL;
}

void *consumer_thread(void *arg)
{
    while (1)
    {
        pthread_mutex_lock(&mutex);
        while (rb.head == rb.tail)
        {
            pthread_cond_wait(&cond, &mutex);
        }
        RocketData data = rb.buffer[rb.tail];
        rb.tail = (rb.tail + 1) % 10;
        pthread_mutex_unlock(&mutex);

        printf(" -> [Ground Stat] Speed: %.2f | High: %.2f\n", data.speed, data.altitude);
    }
    return NULL;
}

int main()
{
    char port_path[50];
    printf("Port (/dev/pts/X): ");
    scanf("%s", port_path);

    serial_fd = open_serial_port(port_path);
    if (serial_fd < 0)
        return 1;

    pthread_t t1, t2;
    pthread_create(&t1, NULL, receiver_thread, NULL);
    pthread_create(&t2, NULL, consumer_thread, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    close(serial_fd);
    return 0;
}