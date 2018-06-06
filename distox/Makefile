

CC = g++
CFLAGS = -g -O2 -Wall 
# CFLAGS = -g -O2 -Wall -DLINUX

OBJS = \
  Matrix.o \
  Vector.o \
  Serial.o \
  Protocol.o

default: $(OBJS)


%.o: %.cpp
	$(CC) $(CFLAGS) -o $@ -c $^

Serial: Serial.cpp
	$(CC) $(CFLAGS) -DTEST -o $@ $^

clean:
	rm -f *.o 

