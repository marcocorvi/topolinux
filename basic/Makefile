

# CC = arm-linux-g++
# STRIP = arm-linux-strip
CC = g++
STRIP = strip

CFLAGS = -g -O2 -Wall -I../distox

EXES = \
  tlx_dump_data \
  tlx_send_command \
  tlx_write_calib \
  tlx_read_calib \
  tlx_toggle_calib \
  tlx_set_calibmode \
  tlx_data2tlx \
  tlx_dump2data

SERIAL_OBJS = \
  ../distox/Serial.o

DISTOX_OBJS = \
  ../distox/Serial.o \
  ../distox/Protocol.o

VECTOR_OBJS = \
  ../distox/Vector.o \
  ../distox/Matrix.o

default: $(EXES)

all: $(EXES)


tlx_dump_data: dump_data.cpp $(DISTOX_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ -lrt -lpthread
	$(STRIP) $@

tlx_send_command: send_command.cpp $(DISTOX_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ -lrt -lpthread
	$(STRIP) $@

tlx_write_calib: write_calib.cpp $(SERIAL_OBJS)
	$(CC) $(CFLAGS) -o $@ $^
	$(STRIP) $@

tlx_read_calib: read_calib.cpp $(SERIAL_OBJS)
	$(CC) $(CFLAGS) -o $@ $^
	$(STRIP) $@

tlx_toggle_calib: toggle_calib.cpp $(SERIAL_OBJS)
	$(CC) $(CFLAGS) -o $@ $^
	$(STRIP) $@

tlx_set_calibmode: set_calibmode.cpp $(SERIAL_OBJS)
	$(CC) $(CFLAGS) -o $@ $^
	$(STRIP) $@

tlx_data2tlx: data2tlx.cpp 
	$(CC) $(CFLAGS) -o $@ $^ -lm
	$(STRIP) $@

tlx_dump2data: dump2data.c 
	$(CC) $(CFLAGS) -o $@ $^ -lm

clean:
	rm -f *.o $(EXES)

distclean:
	rm -f *.o $(EXES)
