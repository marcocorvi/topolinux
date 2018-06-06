/** @file clear_memory.cpp
 *
 * @author marco corvi
 * @date jan 2009
 *
 * @brief clear a portion of disto memory (DistoX1)
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


/** write to memory 4 bytes at a time
 * @param addr starting address
 * @param end  upper bound of memory to read
 */
void
write_memory( Serial * serial, unsigned long addr, unsigned long end )
{
  unsigned long reply_addr;
  unsigned char buf[8];
  unsigned int cnt = 0;
  int i;
  ssize_t nr;
  addr = addr - (addr % 8);
  end  = end  - (end % 8);
  for ( ; addr < end; addr += 4 ) {
    if ( ( cnt % 2 ) == 0 ) fprintf(stderr, "%04lx: ", addr);
    buf[0] = 0x38;
    buf[1] = (unsigned char)( addr & 0xff );
    buf[2] = (unsigned char)( (addr>>8) & 0xff );
    nr = serial->Write( buf, 3 ); // read data
    nr = serial->Read( buf, 8 );
    if ( nr > 0 ) {
      if ( buf[0] != 0x38 ) {
        fprintf(stderr, 
                "read_memory() wrong reply packet at addr %04lx cnt %d\n", 
                addr, cnt);
        break;
      }
      reply_addr = ((unsigned long)(buf[2]))<<8 | buf[1];
      if ( reply_addr != addr ) {
        fprintf(stderr,
                "read_memory() wrong reply addr %04lx at addr %04lx cnt %d\n",
                reply_addr, addr, cnt);
        break;
      }
      for (i=3; i<7; ++i) fprintf(stderr, "%02x ", buf[i] );
    } else if ( nr < 0 ) {
      perror("read_memory() error **** ");
      break;
    } else {
      fprintf(stderr, "read_memory() read returns 0 bytes\n");
      break;
    }

    /* NOW WRITE ff ff ff ff */

    buf[0] = 0x39;
    buf[1] = (unsigned char)( addr & 0xff );
    buf[2] = (unsigned char)( (addr>>8) & 0xff );
    buf[3] = 0xff;
    buf[4] = 0xff;
    buf[5] = 0xff;
    buf[6] = 0xff;
    nr = serial->Write( buf, 7 ); // write data
    nr = serial->Read( buf, 8 );
    if ( nr > 0 ) {
      if ( buf[0] != 0x38 ) {
        fprintf(stderr, 
                "write() wrong reply packet at addr %04lx cnt %d\n", 
                addr, cnt);
        break;
      }
      reply_addr = ((unsigned long)(buf[2]))<<8 | buf[1];
      if ( reply_addr != addr ) {
        fprintf(stderr,
                "write() wrong reply addr %04lx at addr %04lx cnt %d\n",
                reply_addr, addr, cnt);
        break;
      }
      for (i=3; i<7; ++i) fprintf(stderr, "%02x ", buf[i] );
    } else if ( nr < 0 ) {
      perror("write_memory() error **** ");
      break;
    } else {
      fprintf(stderr, "write_memory() read returns 0 bytes\n");
      break;
    }

    ++cnt;
    if ( ( cnt % 2 ) == 0 ) fprintf(stderr, "\n");
  }
}

void usage()
{
  fprintf(stderr, "Usage: tlx_clear_memory [-d device] addr [end] \n");
  fprintf(stderr, "writes ff ff ff ff to the memory\n");
  fprintf(stderr, "where\n");
  fprintf(stderr, "  the device is usually %s\n", DEFAULT_DEVICE );
  fprintf(stderr, "  addr is 0x0000 - 0x8000 for external EEPROM\n");
  fprintf(stderr, "          0x8000 - 0x8100 for internal EEPROM\n");
  fprintf(stderr, "          0xC000 - 0xC100 for RAM\n");
  fprintf(stderr, "  4 bytes are read if no end is specified \n");
}
 
 
int main( int argc, char ** argv )
{
  const char * device = DEFAULT_DEVICE;
  unsigned long addr = 0x0;
  unsigned long end;

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
    return 0;
  }
  sscanf( argv[ac], "%lx", &addr );
  ++ ac;
  addr = addr - (addr % 8);
  if ( addr >= 0x8100 && ( addr < 0xC000 || addr >= 0xC100 ) ) {
    fprintf(stderr, "address 0x%04lx is out of bounds\n", addr );
  }
  end = addr + 4;
  
  if ( ac <= argc ) {
    sscanf( argv[ac], "%lx", &end );
  }
  if ( addr < 0x8000 ) {
    if ( end > 0x8000 ) end = 0x8000;
  } else if ( addr < 0x8100 ) {
    if ( end > 0x8100 ) end = 0x8100;
  } else {
    if ( end > 0xC100 ) end = 0xC100;
  }
  fprintf(stderr, "Device %s addr 0x%04lx - 0x%04lx\n",
    device, addr, end );


  Serial serial( device );

  if ( ! serial.Open( ) ) {
    fprintf(stderr, "Failed to open device %s\n", device );
    return 1;
  }

  write_memory( &serial, addr, end );

  serial.Close();
  return 0;
}

