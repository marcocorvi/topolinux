/** @file dump_memory_x310.cpp
 *
 * @author marco corvi
 * @date jan 2009
 *
 * @brief dump a portion of disto X310 memory 
 *
 * Usage: @see usage()
 *
 * FIXME unfinished
 *
 * --------------------------------------------------------
 *  Copyright This sowftare is distributed under GPL-3.0 or later
 *  See the file COPYING.
 */
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>   // uint16_t
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>    // read write close

#include "defaults.h"
#include "Serial.h"

#define DATA_PER_BLOCK 56
#define BYTE_PER_DATA  18

unsigned long index2addr( unsigned int index )
{
  unsigned int addr = 0;
  while ( index >= DATA_PER_BLOCK ) {
    index -= DATA_PER_BLOCK;
    addr += 0x400;
  }
  addr += BYTE_PER_DATA * index;
  return addr;
}

unsigned int addr2index( unsigned long addr )
{
  unsigned int index = 0;
  addr = addr - ( addr % 8 );
  while ( addr >= 0x400 ) {
    addr -= 0x400;
    index += DATA_PER_BLOCK;
  }
  index += (int)(addr/BYTE_PER_DATA);
  return index;
}


/** reads from memory 4 bytes at a time
 * @param addr starting address
 * @param end  upper bound of memory to read
 */
void
read_memory( Serial * serial, unsigned long addr, unsigned long end,
             FILE * fp )
{
  unsigned long reply_addr;
  unsigned char buf[8];
  int i;
  ssize_t nr;
  // addr = addr - (addr % 8);
  // end  = end  - (addr % 8);
  if ( fp ) {
    fprintf(fp, "%04lx [%4ld]: ", addr, addr);
  }
  fprintf(stderr, "%04lx [%4ld]: ", addr, addr);
  for ( ; addr < end; addr += 4 ) {
    buf[0] = 0x38;
    buf[1] = (unsigned char)( addr & 0xff );
    buf[2] = (unsigned char)( (addr>>8) & 0xff );
    nr = serial->Write( buf, 3 );
    nr = serial->Read( buf, 8 );
    if ( nr > 0 ) {
      if ( buf[0] != 0x38 ) {
        fprintf(stderr, 
                "read_memory() wrong reply packet at addr %04lx\n", addr);
        fprintf(stderr, "  %02x %02x %02x %02x %02x %02x %02x %02x \n",
           buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7] );
        break;
      }
      reply_addr = ((unsigned long)(buf[2]))<<8 | buf[1];
      if ( reply_addr != addr ) {
        fprintf(stderr,
                "read_memory() wrong reply addr %04lx at addr %04lx\n",
                reply_addr, addr);
        break;
      }
      for (i=3; i<7; ++i) {
        if ( fp ) {
          fprintf(fp, "%02x ", buf[i] );
        }
        fprintf(stderr, "%02x ", buf[i] );
      }
    } else if ( nr < 0 ) {
      perror("read_memory() error **** ");
      break;
    } else {
      fprintf(stderr, "read_memory() read returns 0 bytes\n");
      break;
    }
  }
  if ( fp ) {
    fprintf(fp, "\n");
  } 
  fprintf(stderr, "\n");
}

/**
 * FIXME usage efers to DistoX1 and needs fixing
 */
void usage()
{
  static bool usaged = false;
  if ( usaged ) return;
  usaged = true;
  fprintf(stderr, "Usage: dump_memory [options] addr [end] \n");
  fprintf(stderr, "where\n");
  fprintf(stderr, "  addr is 0x0000 - 0x8000 for external EEPROM\n");
  fprintf(stderr, "          0x8000 - 0x8100 for internal EEPROM\n");
  fprintf(stderr, "          0xC000 - 0xC100 for RAM\n");
  fprintf(stderr, "  4 bytes are read if no end is specified \n");
  fprintf(stderr, "Options:\n");
  fprintf(stderr, "  -o outfile  write output to file as well\n");
  fprintf(stderr, "  -d device   distox device [default %s]\n", DEFAULT_DEVICE );
  // fprintf(stderr, "  -q          print DistoX queue bounds and exit\n");
  // fprintf(stderr, "  -n          no address bound check\n");
  fprintf(stderr, "  -v          verbose\n");
  fprintf(stderr, "  -h          this help\n");
}
  
 
 
int main( int argc, char ** argv )
{
  const char * device = DEFAULT_DEVICE ;
  char * outfile = NULL;
  FILE * fp = NULL;
  int first; // start index
  int last;  // end index
  bool verbose = false;
  // bool queue = false;
  // bool no_address_limit = false;

  int ac = 1;

  while ( ac < argc && argv[ac][0] == '-' ) {
    switch ( argv[ac][1] ) {
      case 'd':
        device = argv[++ac];
        break;
      case 'o':
        outfile = argv[++ac];
        break;
      case 'h':
        usage();
        break;
      // case 'n':
      //   no_address_limit = true;
      //   break;
      // case 'q':
      //   queue = true;
      //   break;
      case 'v':
        verbose = true;
        break;
    }      
    ++ac;
  }
  // if ( ! queue && argc <= ac) {
  //   usage();
  //   return 0;
  // }
  if ( verbose ) {
    fprintf( stderr, "DistoX memory.\n");
    fprintf( stderr, "  device:   %s\n", device );
    // fprintf( stderr, "  reading:  %s\n", queue ? "queue" : "data" );
    if ( outfile ) fprintf( stderr, "  output file: %s\n", outfile );
  }

  Serial serial( device );
  if ( ! serial.Open( ) ) {
    fprintf(stderr, "Error. Failed to open device %s\n", device );
    return 1;
  } else if ( verbose ) {
    fprintf(stderr, "... connected to the DistoX\n");
  }

  // if ( queue ) {
  //   if ( read_queue( &serial ) ) {
  //   }
  //   serial.Close();
  //   return 0;
  // }

  if ( outfile ) {
    fp = fopen( outfile, "w+" );
    if ( fp == NULL ) {
      fprintf(stderr, "Warning. Cannot open outfile \"%s\"\n", outfile );
    }
  }

  sscanf( argv[ac], "%d", &first );
  ++ ac;
  if ( first < 0 ) first = 0;
  // if ( first >= 448 ) first = 448-1;
  last = first + 1;
  
  if ( ac <= argc ) {
    sscanf( argv[ac], "%d", &last );
    if ( last < first ) last = first + 1;
  }
  if ( verbose ) {
    fprintf(stderr, "Device %s range %d - %d \n", device, first, last );
  }

  for ( int k=first; k < last; ++k ) {
    unsigned long addr = index2addr( k );
    unsigned long end  = addr + BYTE_PER_DATA;
    read_memory( &serial, addr, end, fp );
  }
  if ( fp ) fclose( fp );
  serial.Close();

  return 0;
}

