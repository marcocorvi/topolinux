/** @file toggle_calib.cpp
 *
 * @author marco corvi
 * @date jan 2009
 *
 * @brief toggle DistoX calibration mode
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
 * @return true if successful
 */
bool
read_8000( Serial * serial, unsigned char * mode )
{
  unsigned long addr = 0x8000;
  unsigned long reply_addr;
  unsigned char buf[8];
  ssize_t nr;

  buf[0] = 0x38;
  buf[1] = (unsigned char)( addr & 0xff );
  buf[2] = (unsigned char)( (addr>>8) & 0xff );
  nr = serial->Write( buf, 3 ); // read data
  nr = serial->Read( buf, 8 );
  if ( nr > 0 ) {
    if ( buf[0] != 0x38 ) {
      fprintf(stderr, 
              "ERROR: read() wrong reply packet at addr %04lx\n", addr);
      return false;
    }
    reply_addr = ((unsigned long)(buf[2]))<<8 | buf[1];
    if ( reply_addr != addr ) {
      fprintf(stderr,
              "ERROR: read() wrong reply addr %04lx at addr %04lx\n",
              reply_addr, addr);
      return false;
    }
    // for (i=0; i<4; ++i) old_byte[4*k+i] = buf[3+i];
    *mode = buf[3];
  } else if ( nr < 0 ) {
    fprintf(stderr, "ERROR: read() error \n");
    return false;
  } else {
    fprintf(stderr, "ERROR: read() returns 0 bytes\n");
    return false;
  }
  return true;
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
    fprintf(stderr, "Usage: toggle_calib [-d device]\n");
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -d device serail device [%s]\n", DEFAULT_DEVICE);
    fprintf(stderr, "  -h        help\n");
  }
  printed_usage = true;
}
 
 
int main( int argc, char ** argv )
{
  const char * device = DEFAULT_DEVICE;

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

  Serial serial( device );

  if ( ! serial.Open( ) ) {
    fprintf(stderr, "ERROR: Failed to open device %s\n", device );
    return 1;
  }

  unsigned char mode = 0x00;
  for (int k = 0; k<3; ++k ) {
    if ( read_8000( &serial, &mode ) ) {
      break;
    }
  }
  if ( mode != 0x00 ) {
    unsigned char mode1 = 0x00;
    unsigned char mode2 = mode;
    if ( mode2 & 0x08 ) {
      mode2 &= 0xf7;
    } else {
      mode2 |= 0x08;
    }
    for (int k = 0; k<3; ++k ) {
      if ( mode & 0x08 ) { // calib on: switch off
        send_command( &serial, 0x30 );
      } else {
        send_command( &serial, 0x31 );
      }
      if ( read_8000( &serial, &mode1 ) && mode1 != mode ) {
        break;
      }
    }
    if ( mode1 == mode2 ) {
      if ( mode & 0x08 ) {
        fprintf(stdout, "DistoX in normal mode\n");
      } else {
        fprintf(stdout, "DistoX in calibration mode\n");
      }
    } else {
      fprintf(stdout, "Failed to switch DistoX mode\n");
    }
  }
  serial.Close();
  return 0;
}

