/** @file bootloader_read.cpp
 *
 * @author marco corvi
 * @date jan 2009
 *
 * @brief read firmware data from the DistoX
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
#include <unistd.h>    // read write close

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
bootloader_read( Serial * serial, unsigned int addr, FILE * fp )
{
  unsigned long reply_addr;
  unsigned char buf[8];
  ssize_t nr;
  
  if ( fp == NULL ) {
    printf( "bootloader read %04x\n", addr );
  }

  addr &= 0x00ff;
  buf[0] = 0x3a;
  buf[1] = (unsigned char)( addr & 0xff );      // bits 8..15 of address
  buf[2] = (unsigned char)( (addr>>8) & 0xff ); // bits 16..23 of address

  nr = serial->Write( buf, 3 ); // read data
  nr = serial->Read( buf, 8 );
  if ( nr > 0 ) {
    if ( buf[0] != 0x3a ) {
      fprintf(stderr, "ERROR: read() wrong reply packet at addr %04x\n", addr);
      fprintf(stderr, "%02x %02x %02x %02x %02x %02x %02x %02x\n",
               buf[0],  buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7] );
      return false;
    }
    reply_addr = ((unsigned long)(buf[2]))<<8 | buf[1];
    if ( reply_addr != addr ) {
      fprintf(stderr, "ERROR: read() wrong reply addr %04lx at addr %04x\n", reply_addr, addr);
      fprintf(stderr, "%02x %02x %02x %02x %02x %02x %02x %02x\n",
               buf[0],  buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7] );
      return false;
    }
  } else if ( nr < 0 ) {
    fprintf(stderr, "ERROR: read() 8-byte error\n");
    return false;
  } else {
    fprintf(stderr, "ERROR: read() read returns 0 bytes\n");
    return false;
  }
  if ( fp == NULL ) {
    for (int k=0; k<32; ++k) {
      nr = serial->Read( buf, 8 );
      // fprintf(stderr, "Read %d bytes \n", nr );
      if ( nr < 0 ) {
        fprintf(stderr, "ERROR: read() %d 8-byte error\n", k);
        return false;
      }
      fprintf(stderr, "%2d: %02x %02x %02x %02x %02x %02x %02x %02x\n", k,
              buf[0],  buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7] );
    }
  } else {
    for (int k=0; k<32; ++k) {
      nr = serial->Read( buf, 8 );
      fwrite( buf, 1, 8, fp );
      if ( nr < 0 ) {
        fprintf(stderr, "ERROR: read() %d 8-byte error\n", k);
        return false;
      }
    }
  }
  return true;
}

void usage()
{
  static bool printed_usage = false;
  if ( ! printed_usage ) {
    fprintf(stderr, "Usage: bootloader_read [options] reduced-address\n");
    fprintf(stderr, "Reduced address is a memory address diviced by 256\n");
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -d device serial device [%s]\n", DEFAULT_DEVICE );
    fprintf(stderr, "  -D        dump the whole memory (need binary output file)\n");
    fprintf(stderr, "  -o file   output binary file\n");
    fprintf(stderr, "  -v        verbose\n");
    fprintf(stderr, "  -h        help\n");
    fprintf(stderr, "Example: bootloader_read -d /dev/rfcomm2 0x68\n");
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
  const char * dump_file = NULL;
  const char * device = DEFAULT_DEVICE;
    
  int ac = 1;
  while ( ac < argc ) {
    if ( strncmp(argv[ac], "-d", 2 ) == 0 ) {
      device = argv[++ac];
      ++ac;
    } else if ( strncmp(argv[ac], "-D", 2 ) == 0 ) {
      do_dump = true;
      ++ ac;
    } else if ( strncmp(argv[ac], "-v", 2 ) == 0 ) {
      verbose = true;
      ++ ac;
    } else if ( strncmp(argv[ac], "-o", 2 ) == 0 ) {
      ++ ac;
      dump_file = argv[ac];
      ++ ac;
    } else if ( strncmp(argv[ac], "-h", 2 ) == 0 ) {
      usage();
      ++ ac;
    } else {
      break;
    }
  }
  if (  ac >= argc && dump_file == NULL ) {
    usage();
    return 0;
  }

  if ( verbose ) {
    fprintf(stderr, "dump %s (file %s)\n", (do_dump ? "true" : "false"), dump_file );
  }

  unsigned int address = 0;
  if ( ac < argc ) {
    sscanf( argv[ac], "%x", &address );
    address &= 0xff; // enforce high byte to 0
    if ( verbose ) {
      fprintf(stderr, "Address <%s> %04x00\n", argv[ac], address );
    }
  }

  Serial serial( device, true );
  if ( ! serial.Open( ) ) {
    fprintf(stderr, "ERROR: Failed to open device %s\n", device );
    return 1;
  }

  if ( dump_file == NULL ) {
    if ( bootloader_read( &serial, address, NULL ) ) {
    }
  } else {
    FILE * fp = fopen( dump_file, "w" );
    if ( do_dump ) {
      for ( address = 0; address < 0x0100; ++ address ) {
        if ( verbose ) fprintf(stderr, ".");
        if ( bootloader_read( &serial, address, fp ) ) {
        }
      }
      if ( verbose ) fprintf(stderr, "\n");
    } else {
      if ( bootloader_read( &serial, address, fp ) ) {
      }
    }
    fclose( fp );
  }
  serial.Close();
  return 0;
}

