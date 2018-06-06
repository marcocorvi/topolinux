/** @file set_memory.cpp
 *
 * @author marco corvi
 * @date jan 2009
 *
 * @brief set a portion of disto memory by swapping the hot bit on
 * --------------------------------------------------------
 *  Copyright This sowftare is distributed under GPL-3.0 or later
 *  See the file COPYING.
 */
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
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
 * @return true if memory has been changed, false otherwise
 */
bool
swap_hotbit( Serial * serial, unsigned long addr )
{
  unsigned long reply_addr;
  unsigned char buf[8];
  int i;
  ssize_t nr;
  addr = addr - (addr % 8);
  {
    buf[0] = 0x38;
    buf[1] = (unsigned char)( addr & 0xff );
    buf[2] = (unsigned char)( (addr>>8) & 0xff );
    nr = serial->Write( buf, 3 ); // read data
    nr = serial->Read( buf, 8 );
    if ( nr > 0 ) {
      if ( buf[0] != 0x38 ) {
        fprintf(stderr, 
                "read() wrong reply packet at addr %04lx\n", addr);
        return false;
      }
      reply_addr = ((unsigned long)(buf[2]))<<8 | buf[1];
      if ( reply_addr != addr ) {
        fprintf(stderr,
                "read() wrong reply addr %04lx at addr %04lx\n",
                reply_addr, addr);
        return false;
      }
      // for (i=3; i<7; ++i) fprintf(stderr, "%02x ", buf[i] );
    } else if ( nr < 0 ) {
      perror("read() error **** ");
      return false;
    } else {
      fprintf(stderr, "read() read returns 0 bytes\n");
      return false;
    }

    buf[0] = 0x39;
    // buf[1] = (unsigned char)( addr & 0xff );
    // buf[2] = (unsigned char)( (addr>>8) & 0xff );
    if ( buf[3] == 0x00 ) {
      fprintf(stderr, "WARNING: refusing to change address 0x%04lx\n", addr );
      return false;
    }  
    buf[3] |= 0x80; // RESET HOT BIT

    nr = serial->Write( buf, 7 ); // write data
    nr = serial->Read( buf, 8 );
    if ( nr > 0 ) {
      if ( buf[0] != 0x38 ) {
        fprintf(stderr, 
                "ERROR: write() wrong reply packet at addr %04lx\n", addr);
        return false;
      }
      reply_addr = ((unsigned long)(buf[2]))<<8 | buf[1];
      if ( reply_addr != addr ) {
        fprintf(stderr,
                "ERROR: write() wrong reply addr %04lx at addr %04lx\n",
                reply_addr, addr);
        return false;
      }
      fprintf(stdout, "%04lx: ", addr );
      for (i=3; i<7; ++i) fprintf(stdout, "%02x ", buf[i] );
      fprintf(stdout, "\n");
    } else if ( nr < 0 ) {
      fprintf(stderr, "ERROR: write() error");
      return false;
    } else {
      fprintf(stderr, "WARNING: write() read returns 0 bytes\n");
      return false;
    }
  }
  return true;
}

void usage()
{
  fprintf(stderr, "Usage: tlx_set_memory [-d device] start_addr end_addr\n");
  fprintf(stderr, "where\n");
  fprintf(stderr, "  the device is usually %s\n", DEFAULT_DEVICE );
  fprintf(stderr, "  addr is 0x0000 - 0x8000 for external EEPROM\n");
}
 
 
int main( int argc, char ** argv )
{
  const char * device = DEFAULT_DEVICE ;
  unsigned long addr = 0x0000;
  unsigned long end  = 0x0008;

  int ac = 1;
  if ( argc <= ac) {
    usage();
    return 0;
  }

  if ( strcmp(argv[ac], "-d" ) == 0 ) {
    device = argv[++ac];
    ++ac;
  }
  if ( argc <= ac ) {
    usage();
    fprintf(stderr, "ERROR: missing address specification\n");
    return 0;
  }
  sscanf( argv[ac], "%lx", &addr );
  ++ ac;
  addr = addr - (addr % 8); // 8-byte boundary;

  end = addr + 8;
  if ( argc > ac ) {
    sscanf( argv[ac], "%lx", &end );
  }
  if ( end <= addr ) {
    fprintf(stderr, "ERROR: end address must be larger than start address\n");
    return 0;
  }
  if ( addr >= 0x8000 ) {
    fprintf(stderr, "ERROR: start address must be less than 0x8000\n");
    return 0;
  }
  if ( end > 0x8000 ) {
    fprintf(stderr, "ERROR: end address must be less or equal to 0x8000\n");
    return 0;
  }

  fprintf(stdout, "Device %s addr 0x%04lx - 0x%04lx \n", device, addr, end );

  Serial serial( device );

  if ( ! serial.Open( ) ) {
    fprintf(stderr, "ERROR: Failed to open device %s\n", device );
    return 1;
  }

  while ( addr < end ) {
    if ( ! swap_hotbit( &serial, addr ) ) break;
    addr += 8;
  }

  serial.Close();
  return 0;
}

