CC=g++
CFLAGS=-Wall -O2 -std=c++11
LDFLAGS=-lpthread
OBJECTS=main.o tands.o thread_handler.o
DIR1=src/helpers/
INC=$(DIR1)
INC_PARAMS=$(foreach d, $(INC), -I$d)

all: prodcon

debug: CFLAGS += -g
debug: prodcon 

prodcon: $(OBJECTS)
	$(CC) -o prodcon $(OBJECTS) $(LDFLAGS)

main.o: main.cpp thread_handler.o
	$(CC) $(CFLAGS) -c main.cpp $(INC_PARAMS) -o main.o $(LDFLAGS)

tands.o: $(DIR1)tands.cpp
	$(CC) $(CFLAGS) -c $(DIR1)tands.cpp $(INC_PARAMS) -o tands.o $(LDFLAGS)

thread_handler.o: $(DIR1)thread_handler.cpp tands.o
	$(CC) $(CFLAGS) -c $(DIR1)thread_handler.cpp $(INC_PARAMS) -o thread_handler.o $(LDFLAGS)

clean:
	$(RM) prodcon *.o
