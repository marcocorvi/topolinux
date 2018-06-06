/** @file read_calib.cpp
 *
 * @author marco corvi
 * @date jan 2009
 *
 * @brief read the calibration coefficients fron the DistoX
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
#include "Factors.h"

#define N_COEFF 48

#define C_2_D( c, k ) ((int16_t)(((uint16_t)(c[k])) | (uint16_t)(c[k+1])<< 8))

#define COEFF2BGX( c ) (  C_2_D( c,  0) / FV )
#define COEFF2AGXX( c ) ( C_2_D( c,  2) / FM )
#define COEFF2AGXY( c ) ( C_2_D( c,  4) / FM )
#define COEFF2AGXZ( c ) ( C_2_D( c,  6) / FM )
#define COEFF2BGY( c )  ( C_2_D( c,  8) / FV )
#define COEFF2AGYX( c ) ( C_2_D( c, 10) / FM )
#define COEFF2AGYY( c ) ( C_2_D( c, 12) / FM )
#define COEFF2AGYZ( c ) ( C_2_D( c, 14) / FM )
#define COEFF2BGZ( c )  ( C_2_D( c, 16) / FV )
#define COEFF2AGZX( c ) ( C_2_D( c, 18) / FM )
#define COEFF2AGZY( c ) ( C_2_D( c, 20) / FM )
#define COEFF2AGZZ( c ) ( C_2_D( c, 22) / FM )

#define COEFF2BMX( c )  ( C_2_D( c, 24) / FV )
#define COEFF2AMXX( c ) ( C_2_D( c, 26) / FM )
#define COEFF2AMXY( c ) ( C_2_D( c, 28) / FM )
#define COEFF2AMXZ( c ) ( C_2_D( c, 30) / FM )
#define COEFF2BMY( c )  ( C_2_D( c, 32) / FV )
#define COEFF2AMYX( c ) ( C_2_D( c, 34) / FM )
#define COEFF2AMYY( c ) ( C_2_D( c, 36) / FM )
#define COEFF2AMYZ( c ) ( C_2_D( c, 38) / FM )
#define COEFF2BMZ( c )  ( C_2_D( c, 40) / FV )
#define COEFF2AMZX( c ) ( C_2_D( c, 42) / FM )
#define COEFF2AMZY( c ) ( C_2_D( c, 44) / FM )
#define COEFF2AMZZ( c ) ( C_2_D( c, 46) / FM )


/** write to memory 4 bytes at a time the eight bytes at given address
 * @param serial serial line (communication channel)
 * @param addr address (should be multiple of 4)
 * @param byte eight-byte array to write at (addr,addr+4)
 *
 * @return true if memory has been changed, false otherwise
 * @note the previous content of the memory is written in byte[]
 */
bool
read_coeffs( Serial * serial, unsigned char byte[N_COEFF] )
{
  unsigned long addr = 0x8010;
  unsigned long reply_addr;
  unsigned char buf[8];
  int i;
  ssize_t nr;

  for (int k=0; k<N_COEFF/4; ++k) {
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
      for (i=0; i<4; ++i) byte[4*k+i] = buf[3+i];
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
    fprintf(stderr, "Usage: tlx_read_calib [-d device] [-h]\n");
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -d device serial device [%s]\n", DEFAULT_DEVICE );
    fprintf(stderr, "  -v        verbose\n");
    fprintf(stderr, "  -h        help\n");
  }
  printed_usage = true;
}
 
 
int main( int argc, char ** argv )
{
  bool verbose = false;
  const char * device = DEFAULT_DEVICE;
  unsigned char coeff[ N_COEFF ] = {
    0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40,
    0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40
  };
    
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

  Serial serial( device );
  if ( ! serial.Open( ) ) {
    fprintf(stderr, "ERROR: Failed to open device %s\n", device );
    return 1;
  }

  unsigned char * buf = &(coeff[0]);

  if ( read_coeffs( &serial, buf ) ) {
    for (int k = 0; k<48; ++k ) {
      fprintf(stdout, "0x%02x ", coeff[k] );
      if ( (k % 8) == 7 ) fprintf(stdout, "\n");
    }
    fprintf(stdout, "Calibration input data.\n");
    // skip input data
    fprintf(stdout, "Calibration coefficients.\n");
    fprintf(stdout, "BG:  %7.4f %7.4f %7.4f\n", 
      COEFF2BGX( buf ), COEFF2BGY( buf ), COEFF2BGZ( buf ) ); 
    fprintf(stdout, "AGx: %7.4f %7.4f %7.4f\n", 
      COEFF2AGXX( buf ), COEFF2AGXY( buf ), COEFF2AGXZ( buf ) ); 
    fprintf(stdout, "AGy: %7.4f %7.4f %7.4f\n", 
      COEFF2AGYX( buf ), COEFF2AGYY( buf ), COEFF2AGYZ( buf ) ); 
    fprintf(stdout, "AGz: %7.4f %7.4f %7.4f\n", 
      COEFF2AGZX( buf ), COEFF2AGZY( buf ), COEFF2AGZZ( buf ) ); 
    fprintf(stdout, "BM:  %7.4f %7.4f %7.4f\n", 
      COEFF2BMX( buf ), COEFF2BMY( buf ), COEFF2BMZ( buf ) ); 
    fprintf(stdout, "AMx: %7.4f %7.4f %7.4f\n", 
      COEFF2AMXX( buf ), COEFF2AMXY( buf ), COEFF2AMXZ( buf ) ); 
    fprintf(stdout, "AMy: %7.4f %7.4f %7.4f\n", 
      COEFF2AMYX( buf ), COEFF2AMYY( buf ), COEFF2AMYZ( buf ) ); 
    fprintf(stdout, "AMz: %7.4f %7.4f %7.4f\n", 
      COEFF2AMZX( buf ), COEFF2AMZY( buf ), COEFF2AMZZ( buf ) ); 
  }

  serial.Close();
  return 0;
}

