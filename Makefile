CC = gcc
CFLAGS = -Wall -Wextra    
LIBS = -lpthread

all: ground_station rocket_sim

ground_station: ground_station.c serial_utils.h
	$(CC) $(CFLAGS) ground_station.c -o ground_station $(LIBS)

rocket_sim: rocket_sim.c
	$(CC) $(CFLAGS) rocket_sim.c -o rocket_sim

clean:
	rm -f ground_station rocket_sim