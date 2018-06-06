

# CC = arm-linux-g++
# STRIP = arm-linux-strip
CC = g++
# STRIP = strip
STRIP = echo

CFLAGS = -g -O0 -Wall -I../distox -I../basic

EXES = \
  tlx_dump_memory \
  tlx_dump_memory_proto \
  tlx_dump_memory_x310 \
  tlx_clear_memory \
  tlx_set_memory \
  tlx_reset_memory \
  tlx_read_queue \
  tlx_read_calib \
  tlx_bootloader_read \
  tlx_bootloader_write \
  tlx_firmware_read \
  tlx_firmware_write \
  memory2tlx \
  memory2tlx_proto

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


tlx_dump_memory: dump_memory.cpp $(SERIAL_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ 
	$(STRIP) $@

tlx_dump_memory_proto: dump_memory_proto.cpp $(SERIAL_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ 
	$(STRIP) $@

tlx_dump_memory_x310: dump_memory_x310.cpp $(SERIAL_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ 
	$(STRIP) $@

tlx_clear_memory: clear_memory.cpp $(SERIAL_OBJS)
	$(CC) $(CFLAGS) -o $@ $^
	$(STRIP) $@

tlx_reset_memory: reset_memory.cpp $(SERIAL_OBJS)
	$(CC) $(CFLAGS) -o $@ $^
	$(STRIP) $@

tlx_set_memory: set_memory.cpp $(SERIAL_OBJS)
	$(CC) $(CFLAGS) -o $@ $^
	$(STRIP) $@

tlx_read_queue: read_queue.cpp $(SERIAL_OBJS)
	$(CC) $(CFLAGS) -o $@ $^
	$(STRIP) $@

tlx_read_calib: read_calib.cpp $(SERIAL_OBJS)
	$(CC) $(CFLAGS) -o $@ $^
	$(STRIP) $@

tlx_bootloader_read: bootloader_read.cpp $(SERIAL_OBJS)
	$(CC) $(CFLAGS) -g -O0 -o $@ $^

tlx_bootloader_write: bootloader_write.cpp $(SERIAL_OBJS)
	$(CC) $(CFLAGS) -g -O0 -o $@ $^

tlx_firmware_read: firmware_read.cpp $(SERIAL_OBJS)
	$(CC) $(CFLAGS) -g -O0 -o $@ $^

tlx_firmware_write: firmware_write.cpp $(SERIAL_OBJS)
	$(CC) $(CFLAGS) -g -O0 -o $@ $^

memory2tlx: memory2tlx.cpp $(SERIAL_OBJS)
	$(CC) $(CFLAGS) -o $@ $^
	$(STRIP) $@

memory2tlx_proto: memory2tlx_proto.cpp $(SERIAL_OBJS)
	$(CC) $(CFLAGS) -o $@ $^
	$(STRIP) $@

clean:
	rm -f *.o $(EXES)

distclean:
	rm -f *.o $(EXES)

