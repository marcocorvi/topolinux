/** @file set_calibmode.cpp
 *
 * @author marco corvi
 * @date jan 2009
 *
 * @brief set DistoX calibration mode on/off
 * --------------------------------------------------------
 *  Copyright This sowftare is distributed under GPL-3.0 or later
 *  See the file COPYING.
 */
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>    // read write close

#include "defaults.h"
#include "Serial.h"


/** read to memory 4 bytes at a time the eight bytes at address 0x8000
 * @param serial serial line (communication channel)
 * @param mode   mode byte (output)
 *
 * @return >0 number of read bytes
            0 zero byte read
           -1 wrong reply-packet
           -2 wring address
           -3 read error
 */
int
read_8000( Serial * serial, unsigned char * mode, bool do_write = true )
{
  unsigned long addr = 0x8000;
  unsigned long reply_addr;
  unsigned char buf[8];
  ssize_t nr;

  if ( do_write ) {
    buf[0] = 0x38;
    buf[1] = (unsigned char)( addr & 0xff );
    buf[2] = (unsigned char)( (addr>>8) & 0xff );
    nr = serial->Write( buf, 3 ); // read data
  }
  nr = serial->Read( buf, 8 );
  if ( nr > 0 ) {
    if ( buf[0] != 0x38 ) {
      fprintf(stderr, "ERROR: read() wrong reply packet at addr %04lx\n", addr);
      fprintf(stderr, "%02x %02x %02x %02x %02x %02x %02x %02x\n",
        buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7] ); 
      return -1;
    }
    reply_addr = ((unsigned long)(buf[2]))<<8 | buf[1];
    if ( reply_addr != addr ) {
      fprintf(stderr, "ERROR: read() wrong reply addr %04lx at addr %04lx\n", reply_addr, addr);
      return -2;
    }
    // for (i=0; i<4; ++i) old_byte[4*k+i] = buf[3+i];
    *mode = buf[3];
  } else if ( nr < 0 ) {
    fprintf(stderr, "ERROR: read() error %d\n", nr);
    return -3;
  } else {
    fprintf(stderr, "ERROR: read() returns 0 bytes\n");
    return 0;
  }
  return nr;
}

bool
send_command( Serial * serial, unsigned char byte )
{
  return ( serial->Write( &byte, 1 ) == 1);
    
}

void usage()
{
  static bool printed_usage = false;
  if ( ! printed_usage ) {
    fprintf(stderr, "Usage: set_calibmode [-d device] <on/off>\n");
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -d device serail device [%s]\n", DEFAULT_DEVICE);
    fprintf(stderr, "  -h        help\n");
  }
  printed_usage = true;
}
 
 
int main( int argc, char ** argv )
{
  const char * device = DEFAULT_DEVICE;
  int on_off = 1;

  int ac = 1;
  while ( ac < argc ) {
    if ( strncmp(argv[ac], "-d", 2 ) == 0 ) {
      device = argv[++ac];
      ++ac;
    } else if ( strncmp(argv[ac], "-h", 2 ) == 0 ) {
      usage();
      ++ ac;
    } else {
      break;
    }
  }

  if ( ac >= argc ) {
    usage();
    fprintf(stderr, "you must specify on/off (either 1 or 0)\n");
    return 0;
  }

  on_off = atoi( argv[ac] );
  if ( on_off != 0 && on_off != 1 ) {
    fprintf(stderr, "on/off must be either 1 [on] or 0 [off]\n");
    return 0;
  }

  Serial serial( device );

  if ( ! serial.Open( ) ) {
    fprintf(stderr, "ERROR: Failed to open device %s\n", device );
    return 1;
  }

  for ( int k=0; k<3; ++k ) {
    fprintf(stderr, "[%d] turning calib mode on ...\n", k );
    if ( on_off == 1 ) {
      send_command( &serial, 0x31 );
    } else {
      send_command( &serial, 0x30 );
    }
    sleep(1);
  }

  serial.Close();
  return 0;
}

