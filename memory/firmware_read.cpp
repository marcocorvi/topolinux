/** @file firmware_read.cpp
 *
 * @author marco corvi
 * @date jan 2009
 *
 * @brief read DistoX2 firmware
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
 *
 * @note dump from address 0x0000 to 0x4000
 */
bool
firmware_read( Serial * serial, FILE * fp )
{
  unsigned long reply_addr;
  unsigned char buf[8];
  unsigned char buffer[256];
  if ( fp == NULL ) {
    fprintf(stderr, "no output firmware file \n");
    return false;
  }

  buf[0] = 0x3a;
  buf[1] = 0;
  buf[2] = 0;

  for ( unsigned int addr = 0; addr < 0x40; ++addr ) {
    buf[1] = (unsigned char)(addr & 0xff);

    int nr = serial->Write( buf, 3 ); // read data
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
    nr = serial->Read( buffer, 256 );
    if ( nr < 0 ) {
      fprintf(stderr, "ERROR: read() 256-byte error\n");
      return false;
    }
    fwrite( buffer, 1, 256, fp );
    fprintf(stderr, "." );
  }
  fprintf(stderr, "\n" );
  return true;
}

void usage()
{
  static bool printed_usage = false;
  if ( ! printed_usage ) {
    fprintf(stderr, "Usage: firmware_read [options] output-file\n");
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -d device serial device [%s]\n", DEFAULT_DEVICE );
    fprintf(stderr, "  -v        verbose\n");
    fprintf(stderr, "  -h        help\n");
    fprintf(stderr, "Example: firmware_read -d /dev/rfcomm2 firmware.bin\n\n");
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
  const char * device = DEFAULT_DEVICE;
  const char * outfile = NULL;
    
  int ac = 1;
  while ( ac < argc ) {
    if ( strncmp(argv[ac], "-d", 2 ) == 0 ) {
      device = argv[++ac];
      ++ac;
    } else if ( strncmp(argv[ac], "-v", 2 ) == 0 ) {
      verbose = true;
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
  outfile = argv[ac];

  if ( verbose ) {
    fprintf(stderr, "firmware-read: output-file %s\n", outfile );
  }

  Serial serial( device, true );
  if ( ! serial.Open( ) ) {
    fprintf(stderr, "ERROR: Failed to open device %s\n", device );
    return 1;
  }

  FILE * fp = fopen( outfile, "w" );
  if ( firmware_read( &serial, fp ) ) {
  }
  fclose( fp );
  serial.Close();
  return 0;
}

