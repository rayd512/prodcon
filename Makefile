CC=g++
CFLAGS=-Wall -O2 -std=c++11
OBJECTS=main.o 

prodcon: $(OBJECTS)
	$(CC) -o prodcon $(OBJECTS)

main.o: main.cpp
	$(CC) $(CFLAGS) -c main.cpp -o main.o

clean:
	$(RM) prodcon *.o
