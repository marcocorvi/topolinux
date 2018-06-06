/** @file dump_memory.cpp
 *
 * @author marco corvi
 * @date jan 2009
 *
 * @brief dump a portion of disto memory (DistoX1)
 *
 * Usage: @see usage()
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


/** reads from memory at addr C020
 *
 * @param serial serial line (communication channel)
 * @return true if memory has been changed, false otherwise
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
                "ERROR: read_queue() wrong reply packet at addr %04lx\n", addr);
        fprintf(stderr, "  %02x %02x %02x %02x %02x %02x %02x %02x \n",
           buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7] );
        return false;
      }
      reply_addr = ((unsigned long)(buf[2]))<<8 | buf[1];
      if ( reply_addr != addr ) {
        fprintf(stderr,
                "ERROR: read_queue() wrong reply addr %04lx at addr %04lx\n",
                reply_addr, addr);
        return false;
      }
      // for (i=3; i<7; ++i) fprintf(stderr, "%02x ", buf[i] );
      // for (i=0; i<4; ++i) byte[4*k+i] = buf[3+i];
      uint16_t head = (uint16_t)(buf[3]) | ( ((uint16_t)(buf[4])) << 8 );
      uint16_t tail = (uint16_t)(buf[5]) | ( ((uint16_t)(buf[6])) << 8 );
      fprintf(stdout, "Head %04x Tail %04x\n", head, tail );
    } else if ( nr < 0 ) {
      fprintf(stderr, "ERROR: read_queue() error\n");
      return false;
    } else {
      fprintf(stderr, "ERROR: read_queue() read returns 0 bytes\n");
      return false;
    }
    addr += 4;
  }
  return true;
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
  unsigned int cnt = 0;
  int i;
  ssize_t nr;
  addr = addr - (addr % 8);
  end  = end  - (addr % 8);
  for ( ; addr < end; addr += 4 ) {
    if ( ( cnt % 2 ) == 0 ) {
      if ( fp ) {
        fprintf(fp, "%04lx: ", addr);
      }
      fprintf(stdout, "%04lx: ", addr);
    }
    buf[0] = 0x38;
    buf[1] = (unsigned char)( addr & 0xff );
    buf[2] = (unsigned char)( (addr>>8) & 0xff );
    nr = serial->Write( buf, 3 );
    nr = serial->Read( buf, 8 );
    if ( nr > 0 ) {
      if ( buf[0] != 0x38 ) {
        fprintf(stderr, 
                "read_memory() wrong reply packet at addr %04lx cnt %d\n", 
                addr, cnt);
        fprintf(stderr, "  %02x %02x %02x %02x %02x %02x %02x %02x \n",
           buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7] );
        break;
      }
      reply_addr = ((unsigned long)(buf[2]))<<8 | buf[1];
      if ( reply_addr != addr ) {
        fprintf(stderr,
                "read_memory() wrong reply addr %04lx at addr %04lx cnt %d\n",
                reply_addr, addr, cnt);
        break;
      }
      for (i=3; i<7; ++i) {
        if ( fp ) {
          fprintf(fp, "%02x ", buf[i] );
        }
        fprintf(stdout, "%02x ", buf[i] );
      }
    } else if ( nr < 0 ) {
      perror("read_memory() error **** ");
      break;
    } else {
      fprintf(stderr, "read_memory() read returns 0 bytes\n");
      break;
    }
    ++cnt;
    if ( ( cnt % 2 ) == 0 ) {
      if ( fp ) {
        fprintf(fp, "\n");
      } 
      fprintf(stdout, "\n");
    }
  }
}

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
  fprintf(stderr, "  -q          print DistoX queue bounds and exit\n");
  fprintf(stderr, "  -n          no address bound check\n");
  fprintf(stderr, "  -v          verbose\n");
  fprintf(stderr, "  -h          this help\n");
}
  
 
 
int main( int argc, char ** argv )
{
  const char * device = DEFAULT_DEVICE ;
  char * outfile = NULL;
  FILE * fp = NULL;
  unsigned long addr = 0x0;
  unsigned long end;
  bool verbose = false;
  bool queue = false;
  bool no_address_limit = false;

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
      case 'n':
        no_address_limit = true;
        break;
      case 'q':
        queue = true;
        break;
      case 'v':
        verbose = true;
        break;
    }      
    ++ac;
  }
  if ( ! queue && argc <= ac) {
    usage();
    return 0;
  }
  if ( verbose ) {
    fprintf( stderr, "DistoX memory.\n");
    fprintf( stderr, "  device:   %s\n", device );
    fprintf( stderr, "  reading:  %s\n", queue ? "queue" : "data" );
    if ( outfile ) fprintf( stderr, "  output file: %s\n", outfile );
  }

  Serial serial( device );
  if ( ! serial.Open( ) ) {
    fprintf(stderr, "Error. Failed to open device %s\n", device );
    return 1;
  } else if ( verbose ) {
    fprintf(stderr, "... connected to the DistoX\n");
  }

  if ( queue ) {
    if ( read_queue( &serial ) ) {
    }
    serial.Close();
    return 0;
  }

  if ( outfile ) {
    fp = fopen( outfile, "w+" );
    if ( fp == NULL ) {
      fprintf(stderr, "Warning. Cannot open outfile \"%s\"\n", outfile );
    }
  }

  sscanf( argv[ac], "%lx", &addr );
  ++ ac;
  addr = addr - (addr % 8);
  bool valid = addr < 0x8100
             || ( addr >= 0xC000 && addr < 0xC100 )
             || ( addr >= 0xE000 && addr < 0xE008 )
             || no_address_limit;
  if ( ! valid ) {
    fprintf(stderr, "address 0x%04lx is out of bounds\n", addr );
    fprintf(stderr, "allowed ranges: 0x0000-0x8100 0xC000-0xC100 and 0xE000-0xE008\n");
  }
  end = addr + 4;
  
  if ( ac <= argc ) {
    sscanf( argv[ac], "%lx", &end );
  }
  if ( ! no_address_limit ) {
    if ( addr < 0x8000 ) {
      if ( end > 0x8000 ) end = 0x8000;
    } else if ( addr < 0x8100 ) {
      if ( end > 0x8100 ) end = 0x8100;
    } else if ( addr < 0xC100 ) {
      if ( end > 0xC100 ) end = 0xC100;
    } else if ( addr < 0xE008 ) {
      if ( end > 0xE008 ) end = 0xE008;
    }
  }
  if ( verbose ) {
    fprintf(stderr, "Device %s addr 0x%04lx - 0x%04lx\n",
      device, addr, end );
  }

  read_memory( &serial, addr, end, fp );
  if ( fp ) fclose( fp );
  serial.Close();

  return 0;
}

