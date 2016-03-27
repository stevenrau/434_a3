CC=gcc
CFLAGS=-Wall -pedantic
LIB=-lm

SENSOR_NETWORK_EXEC=sensorNetwork
SENSOR_NETWORK_SOURCE=sensorNetwork.c sensorNetwork.h
BASE_SOURCE=base.c base.h
NODE_SOURCE=node.c node.h
CONN_SOURCE=connection.c connection.h

EXEC=$(SENSOR_NETWORK_EXEC)

ALL_SOURCE=$(SENSOR_NETWORK_SOURCE) $(BASE_SOURCE) $(NODE_SOURCE) $(CONN_SOURCE)

all: sensorNetwork

sensorNetwork: $(ALL_SOURCE)
	$(CC) $(CFLAGS) -o $(SENSOR_NETWORK_EXEC) $(ALL_SOURCE) $(LIB)

clean:
	rm -f *.o $(EXEC) *~
