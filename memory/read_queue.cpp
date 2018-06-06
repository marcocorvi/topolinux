/** @file read_queue.cpp
 *
 * @author marco corvi
 * @date jan 2009
 *
 * @brief read the data queue head/tail fron the DistoX
 * --------------------------------------------------------
 *  Copyright This sowftare is distributed under GPL-3.0 or later
 *  See the file COPYING.
 */
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>   // uint16_t
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>    // read write close

#include "defaults.h"
#include "Serial.h"


/** write to memory 4 bytes at a time the eight bytes at given address
 * @param serial serial line (communication channel)
 * @param addr address (should be multiple of 4)
 * @param byte eight-byte array to write at (addr,addr+4)
 *
 * @return true if memory has been changed, false otherwise
 * @note the previous content of the memory is written in byte[]
 */
bool
read_queue( Serial * serial )
{
  unsigned long addr = 0xC020;
  unsigned long reply_addr;
  unsigned char buf[8];
  ssize_t nr;

  {
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
      // for (i=3; i<7; ++i) fprintf(stderr, "%02x ", buf[i] );
      // for (i=0; i<4; ++i) byte[4*k+i] = buf[3+i];
      uint16_t head = (uint16_t)(buf[3]) | ( ((uint16_t)(buf[4])) << 8 );
      uint16_t tail = (uint16_t)(buf[5]) | ( ((uint16_t)(buf[6])) << 8 );
      printf("Head %04x Tail %04x\n", head, tail );
    } else if ( nr < 0 ) {
      fprintf(stderr, "ERROR: read() error\n");
      return false;
    } else {
      fprintf(stderr, "ERROR: read() read returns 0 bytes\n");
      return false;
    }
    addr += 4;
  }
  return true;
}

void usage()
{
  static bool printed_usage = false;
  if ( ! printed_usage ) {
    fprintf(stderr, "Usage: read_queue [-d device] [-h]\n");
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -d device serial device [%s]\n", DEFAULT_DEVICE );
    // fprintf(stderr, "  -v        verbose\n");
    fprintf(stderr, "  -h        help\n");
  }
  printed_usage = true;
}
 
 
int main( int argc, char ** argv )
{
  // bool verbose = false;
  const char * device = DEFAULT_DEVICE;
    
  int ac = 1;
  while ( ac < argc ) {
    if ( strncmp(argv[ac], "-d", 2 ) == 0 ) {
      device = argv[++ac];
      ++ac;
    // } else if ( strncmp(argv[ac], "-v", 2 ) == 0 ) {
    //   verbose = true;
    //   ++ ac;
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

  if ( read_queue( &serial ) ) {
  }
  serial.Close();
  return 0;
}

