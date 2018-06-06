/** @file firmware_write.cpp
 *
 * @author marco corvi
 * @date jan 2009 - sept. 2014
 *
 * @brief write firmware bin file to the DistoX2
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

#define DEFAULT_RFCOMM "/dev/rfcomm3"

/** write to memory 4 bytes at a time the eight bytes at given address
 * @param serial serial line (communication channel)
 * @param fp     firmware file pointer
 * @param end_addr end-address (already divided by 256: e.g. 0x3C fro v21 0x3f for v23)
 * @param dry_run
 *
 * @return true if memory has been changed, false otherwise
 * @note the previous content of the memory is written in byte[]
 */
bool
firmware_write( Serial * serial, FILE * fp, unsigned int end_addr, bool dry_run )
{
  unsigned long reply_addr;
  unsigned char buf[256+3];
  ssize_t nr;

  if ( fp == NULL ) {
    fprintf(stderr, "firmware_write no input file\n");
    return false;
  }
  
  buf[0] = 0x3b;
  buf[1] = 0; 
  buf[2] = 0; 
  for ( unsigned int addr=0; addr<end_addr; ++addr ) {
    memset( buf+3, 0, 256 );
    nr = fread( buf+3, 1, 256, fp );
    if ( addr < 0x08 ) {
      if ( dry_run ) {
        fprintf(stderr, "- 0x%02x\n", addr );
      } else {
        fprintf(stderr, "-");
      }
    } else {
      buf[1] = (unsigned char) ( addr & 0xff );
      buf[2] = 0; // not really necessary
      if ( dry_run ) {
        nr = 8;
        if ( nr > 0 ) fprintf(stderr, "0x%02x\n", addr );
      } else {
        nr = serial->Write( buf, 259 ); // write header
        if ( nr > 0 ) fprintf(stderr, "." );
        nr = serial->Read( buf, 8 );
      }
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
    }
  }
  return true;
}

void usage()
{
  static bool printed_usage = false;
  if ( ! printed_usage ) {
    fprintf(stderr, "Usage: firmware_write [options] firmware_file \n");
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -d device serial device [%s]\n", DEFAULT_RFCOMM );
    fprintf(stderr, "  -n        dry_run\n");
    // fprintf(stderr, "  -v        verbose\n");
    fprintf(stderr, "  -h        help\n");
    fprintf(stderr, "Example: firmware_write -d /dev/rfcomm3 firmware.bin\n");
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
  // bool verbose = false;
  bool dry_run = false;
  const char * in_file = NULL;
  const char * device = DEFAULT_RFCOMM;
    
  int ac = 1;
  while ( ac < argc ) {
    if ( strncmp(argv[ac], "-d", 2 ) == 0 ) {
      device = argv[++ac];
      ++ac;
    } else if ( strncmp(argv[ac], "-n", 2 ) == 0 ) {
      dry_run = true;
      ++ ac;
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
  if (  ac >= argc ) {
    usage();
    return 0;
  }

  in_file = argv[ac];

  struct stat stat_buf;
  if ( stat( in_file, &stat_buf ) == -1 ) {
    fprintf(stderr, "ERROR: cannot stat firmware file %s\n", in_file );
    return 1;
  }
  unsigned int end_addr = (stat_buf.st_size + 255)/256; // file size in bytes
  fprintf(stderr, "End address %02X\n", end_addr );

  Serial serial( device, true );
  if ( ! serial.Open( ) ) {
    fprintf(stderr, "ERROR: Failed to open device %s\n", device );
    return 1;
  }

  FILE * fp = fopen( in_file, "r" );
  if ( firmware_write( &serial, fp, end_addr, dry_run )  ) {
    fprintf(stderr, "firmware write success\n" );
  } else {
    fprintf(stderr, "firmware write fail\n" );
  }
  fclose( fp );
  serial.Close();
  return 0;
}

