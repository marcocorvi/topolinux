/** @file write_calib.cpp
 *
 * @author marco corvi
 * @date jan 2009
 *
 * @brief write a set of calibration coefficients
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

#define N_COEFF 48


/** write to memory 4 bytes at a time the eight bytes at given address
 * @param serial serial line (communication channel)
 * @param addr address (should be multiple of 4)
 * @param byte eight-byte array to write at (addr,addr+4)
 *
 * @return true if memory has been changed, false otherwise
 * @note the previous content of the memory is written in byte[]
 */
bool
write_memory( Serial * serial, unsigned long addr, unsigned char byte[8] )
{
  unsigned long reply_addr;
  unsigned char buf[8];
  unsigned char old_byte[8];
  int i;
  ssize_t nr;

  addr = addr - (addr % 8);
  assert( addr >= 0x8010 && addr < 0x8010+ N_COEFF  );
  for (int k=0; k<2; ++k) {
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
      for (i=0; i<4; ++i) old_byte[4*k+i] = buf[3+i];
    } else if ( nr < 0 ) {
      fprintf(stderr, "ERROR: read() error \n");
      return false;
    } else {
      fprintf(stderr, "WARNING: read() read returns 0 bytes\n");
      return false;
    }

    buf[0] = 0x39;
    // buf[1] = (unsigned char)( addr & 0xff );
    // buf[2] = (unsigned char)( (addr>>8) & 0xff );
    for (i=0; i<4; ++i) buf[3+i] = byte[4*k+i];
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
      // fprintf(stderr, "%04lx: ", addr );
      // for (i=3; i<7; ++i) fprintf(stderr, "%02x ", buf[i] );
      // fprintf(stderr, "\n");
    } else if ( nr < 0 ) {
      fprintf(stderr, "ERROR: write() error \n");
      return false;
    } else {
      fprintf(stderr, "WARNING: write() read returns 0 bytes\n");
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
    fprintf(stderr, "Usage: tlx_write_calib [-d device] [-z] [-h] coeff_file\n");
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -d device serail device [%s]\n", DEFAULT_DEVICE );
    fprintf(stderr, "  -z        default calibration coeffs\n");
    fprintf(stderr, "  -h        help\n");
  }
  printed_usage = true;
}
 
 
int main( int argc, char ** argv )
{
  const char * device = DEFAULT_DEVICE;
  unsigned long addr = 0x8010;
  char * coeff_file = NULL;
  unsigned char coeff[ N_COEFF ] = {
    0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40,
    0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40
  };
    
  bool zero_calib = false;

  int ac = 1;
  if ( argc <= ac) {
    usage();
    return 0;
  }

  while ( ac < argc ) {
    if ( strncmp(argv[ac], "-d", 2 ) == 0 ) {
      device = argv[++ac];
      ++ac;
    } else if ( strncmp(argv[ac], "-z", 2 ) == 0 ) {
      zero_calib = true;
      ++ ac;
    } else if ( strncmp(argv[ac], "-h", 2 ) == 0 ) {
      usage();
      ++ ac;
    } else {
      break;
    }
  }

  if ( ! zero_calib && argc <= ac ) {
    usage();
    return 0;
  } else {
    coeff_file = argv[ac];
    ++ ac;
  }

  if ( coeff_file != NULL ) {  
    FILE * fp = fopen( coeff_file, "r" );
    if ( fp == NULL ) {
      fprintf(stderr, "ERROR: Failed to open coeff file \"%s\"\n", coeff_file );
      return 0;
    }
    for (int k=0; k< N_COEFF; ++k ) {
      unsigned int tmp;
      if ( fscanf(fp, "%x", &tmp ) != 1 ) {
        fprintf(stderr, "ERROR: Failed to read coeff (index %d)\n", k );
        fclose( fp );
        return 0;
      }
      coeff[k] = (unsigned char)(tmp);
    }
    fclose( fp );
  }

  unsigned long end = addr +  N_COEFF ;
  Serial serial( device );

  if ( ! serial.Open( ) ) {
    fprintf(stderr, "ERROR: Failed to open device %s\n", device );
    return 1;
  }

  unsigned char * buf = &(coeff[0]);
  while ( addr < end ) {
    if ( ! write_memory( &serial, addr, buf ) ) break;
    fprintf(stdout, "%04lx: ", addr );
    for (int i=0; i<8; ++i) fprintf(stdout, "0x%02x ", buf[i] );
    fprintf(stdout, "\n");
    addr += 8;
    buf += 8;
  }

  serial.Close();
  return 0;
}

