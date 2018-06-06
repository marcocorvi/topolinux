/** @file bootloader_write.cpp
 *
 * @author marco corvi
 * @date jan 2009
 *
 * @brief write firmware data into the DistoX
 *
 * @note the device must be in "bootloader mode"
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
#include <unistd.h>    // read write close usleep

#include "defaults.h"
#include "Serial.h"


/** write to memory 4 bytes at a time the eight bytes at given address
 * @param serial serial line (communication channel)
 * @param addr address (already divided by 256)
 * @param byte eight-byte array to write at (addr,addr+4)
 *
 * @return true if memory has been changed, false otherwise
 * @note the previous content of the memory is written in byte[]
 */
bool
bootloader_write( Serial * serial, unsigned int addr, unsigned char * buffer )
{
  unsigned long reply_addr;
  unsigned char buf[256+3];
  ssize_t nr;
  
  addr &= 0x00ff;
  buf[0] = 0x3b;
  buf[1] = (unsigned char)( addr & 0xff );      // bits 8..15 of address
  buf[2] = 0; // (unsigned char)( (addr>>8) & 0xff ); // bits 16..23 of address
  memcpy( buf+3, buffer, 256 );

  nr = serial->Write( buf, 259 ); // write header
  fprintf(stderr, "%ld:", nr);

  nr = serial->Read( buf, 8 );
  fprintf(stderr, "%ld\n", nr);
  if ( nr > 0 ) {
    if ( buf[0] != 0x3b ) {
      fprintf(stderr, "ERROR: write() wrong reply packet at addr %04x\n", addr);
      fprintf(stderr, "%02x %02x %02x %02x %02x \n", buf[0], buf[1], buf[2], buf[3], buf[4] );
      return false;
    }
    reply_addr = ((unsigned long)(buf[2]))<<8 | buf[1];
    if ( reply_addr != addr ) {
      fprintf(stderr, "ERROR: write() wrong reply addr %04lx at addr %04x\n", reply_addr, addr);
      fprintf(stderr, "%02x %02x %02x %02x %02x\n", buf[0], buf[1], buf[2], buf[3], buf[4]  );
      return false;
    }
  } else if ( nr < 0 ) {
    fprintf(stderr, "ERROR: write() 5-byte reply error\n");
    return false;
  } else {
    fprintf(stderr, "ERROR: write() reply returns 0 bytes\n");
    return false;
  }
  return true;
}

void usage()
{
  static bool printed_usage = false;
  if ( ! printed_usage ) {
    fprintf(stderr, "Usage: bootloader_write [options] binary_file reduced-address\n");
    fprintf(stderr, "Reduced address is a memory address diviced by 256\n");
    fprintf(stderr, "To load a firmware do not provide the address: bootloader_write binary_file\n");
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -d device serial device [%s]\n", DEFAULT_DEVICE );
    fprintf(stderr, "  -n        dry-run \n");
    fprintf(stderr, "  -v        verbose\n");
    fprintf(stderr, "  -h        help\n");
    fprintf(stderr, "Example: bootloader_write -d /dev/rfcomm2 -f 68.bin 0x68\n");
    fprintf(stderr, "Flash memory map:\n");
    fprintf(stderr, "0x00.00.00 - 0x00.07.FF  bootloader (read-only)\n");
    fprintf(stderr, "0x00.08.00 - 0x00.3F.ff  code\n");
    fprintf(stderr, "0x00.40.00 - 0x00.63.ff  free\n");
    fprintf(stderr, "0x00.64.00 - 0x00.67.ff  option store\n");
    fprintf(stderr, "0x00.68.00 - 0x00.6b.ff  config store\n");
    fprintf(stderr, "0x00.6c.00 - 0x00.ff.ff  data store\n");
  }
  printed_usage = true;
}
 
 
int main( int argc, char ** argv )
{
  bool verbose = false;
  bool do_dump = false;
  bool dry_run = false;
  const char * bin_file = NULL;
  const char * device = DEFAULT_DEVICE;
  unsigned char buffer[256];
    
  int ac = 1;
  while ( ac < argc ) {
    if ( strncmp(argv[ac], "-d", 2 ) == 0 ) {
      device = argv[++ac];
      ++ac;
    } else if ( strncmp(argv[ac], "-v", 2 ) == 0 ) {
      verbose = true;
      ++ ac;
    } else if ( strncmp(argv[ac], "-n", 2 ) == 0 ) {
      dry_run = true;
      ++ ac;
    } else if ( strncmp(argv[ac], "-h", 2 ) == 0 ) {
      usage();
      ++ ac;
    } else {
      break;
    }
  }
  if (  ac >= argc ) {
    usage();
    return 0;
  }

  unsigned int address = 0x40;
  if ( ac < argc ) {
    bin_file = argv[ac];
    ++ ac;
  }

  if ( ac < argc ) {
    sscanf( argv[ac], "%x", &address );
    address &= 0xff; // enforce high byte to 0
    if ( verbose ) {
      fprintf(stderr, "Address <%s> %04x00\n", argv[ac], address );
    }
  } else {
    do_dump = true;
  }
  if ( verbose ) {
    fprintf(stderr, "bin-file \"%s\"\n", bin_file );
    fprintf(stderr, "dump %s \ndry-run %s \n", (do_dump? "true" : "false"), (dry_run? "true" : "false") );
  }

  Serial serial( device, true );
  if ( ! serial.Open( ) ) {
    fprintf(stderr, "ERROR: Failed to open device %s\n", device );
    return 1;
  }

  int n;
  FILE * fp = fopen( bin_file, "r" );
  if ( do_dump ) {
    address = 0x08; // skip 2 KB
    for ( ; ; ) {
      memset( buffer, 0, 256 );
      n = fread( buffer, 1, 256, fp );
      if ( verbose ) {
        fprintf(stderr, "sending %d bytes address %04x\n", n, address );
      } else {
        fprintf(stderr, "." );
      }
      if ( n <= 0 ) break;
      if ( (! dry_run) && bootloader_write( &serial, address, buffer ) ) {
      }
      ++ address;
      if ( address >= 0x0100 ) break;
    }
  } else if ( address >= 0x08 && address < 0x0100 ) {
    memset( buffer, 0, 256 );
    n = fread( buffer, 1, 256, fp );
    if ( verbose ) {
      fprintf(stderr, "sending %d bytes at address %04x\n", n, address );
    }
    if ( n > 0 ) {
      if ( (! dry_run) && bootloader_write( &serial, address, buffer ) ) {
      }
    }
  }
  if ( ! verbose ) {
    fprintf(stderr, "\n");
  }
  fclose( fp );
  serial.Close();
  return 0;
}

