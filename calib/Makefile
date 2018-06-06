

# CC = arm-linux-g++
# CCC = arm-linux-gcc
# STRIP = arm-linux-strip
CC = g++
CCC = gcc
STRIP = strip

# USE_GUI = 1

CFLAGS = -g -O2 -Wall -I../distox -I.

# CFLAGS += -DEXPERIMENTAL
# CFLAGS += -DARM

EXES = tlx_calib \
       tlx_pck2tlx \
       tlx_group_guess \
       tlx_check_calib \
       tlx_int2hex \
       tlx_check_coverage \
       tlx_two_calib \
       tlx_inspect_calib


VECTOR_OBJS = \
  ../distox/Vector.o \
  ../distox/Matrix.o 

ifdef USE_GUI
  CFLAGS += -DUSE_GUI
  XOBJ = \
    ../experimental/CalibrationGui.o \
    ../experimental/gui++.o 
  XLIB = -L/usr/X11R6/lib -lX11
else
  XOBJ =
  XLIB = 
endif

default: $(EXES)

all: $(EXES)


Calibration.o: Calibration.cpp
	$(CC) $(CFLAGS) -o $@ -c $^ 

CalibrationGui.o: CalibrationGui.cpp
	$(CC) $(CFLAGS) -o $@ -c $^ 

.o:.cpp
	$(CC) $(CFLAGS) -o $@ -c $^

tlx_calib: calib.cpp Calibration.o $(VECTOR_OBJS) $(XOBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(XLIB) -lm -lstdc++ 
	$(STRIP) $@

tlx_pck2tlx: pck2tlx.c
	$(CCC) $(CFLAGS) -o $@ $^ -lm 
	$(STRIP) $@

tlx_group_guess: group_guess.c
	$(CCC) $(CFLAGS) -o $@ $^ -lm 
	$(STRIP) $@

tlx_int2hex: int2hex.c
	$(CCC) $(CFLAGS) -o $@ $^ -lm 
	$(STRIP) $@

tlx_two_calib: two_calib.cpp 
	$(CC) $(CFLAGS) -o $@ $^ -lm -lstdc++
	$(STRIP) $@

tlx_check_calib: check_calib.cpp $(VECTOR_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ -lm -lstdc++
	$(STRIP) $@

tlx_check_calib0: check_calib0.cpp $(VECTOR_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ -lm -lstdc++
	$(STRIP) $@

tlx_check_coverage: check_coverage.cpp $(VECTOR_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ -lm -lstdc++
	$(STRIP) $@

tlx_inspect_calib: inspect_calib.c 
	$(CC) $(CFLAGS) -o $@ $^ -lm 
	$(STRIP) $@


clean:
	rm -f *.o $(EXES) $(EXTRA_EXES)

distclean:
	rm -f *.o $(EXES) $(EXTRA_EXES)
