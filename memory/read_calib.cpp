/** @file read_calib.cpp
 *
 * @author marco corvi
 * @date jan 2009
 *
 * @brief read calibration data from distox
 *
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

unsigned char coeff[48];


void read_memory( Serial * serial )
{
  unsigned long addr = 0x8010;
  unsigned long end_addr = addr + 48;
  unsigned long reply_addr;
  unsigned char buf[8];
  int k=0;
  int i;
  ssize_t nr;
  for ( ; addr < end_addr; addr += 4 ) 
  {
    buf[0] = 0x38;
    buf[1] = (unsigned char)( addr & 0xff );
    buf[2] = (unsigned char)( (addr>>8) & 0xff );
    nr = serial->Write( buf, 3 ); // read data
    nr = serial->Read( buf, 8 );
    if ( nr > 0 ) {
      if ( buf[0] != 0x38 ) {
        fprintf(stderr, 
                "ERROR: read wrong reply packet at addr %04lx \n", 
                addr);
        return;
      }
      reply_addr = ((unsigned long)(buf[2]))<<8 | buf[1];
      if ( reply_addr != addr ) {
        fprintf(stderr,
                "ERROR: read wrong reply addr %04lx at addr %04lx \n",
                reply_addr, addr);
        return;
      }
      // fprintf(stderr, "%04lx ", addr );
      // if ( buf[3] == 0 || ( buf[3] & 0x80 ) != 0 ) fprintf(stderr, "*** ");
      for (i=3; i<7; ++i) {
        fprintf(stderr, "%02x ", buf[i] );
        coeff[k++] = buf[i];
      }
    } else if ( nr < 0 ) {
      fprintf(stderr, "ERROR: serial read error\n");
      return;
    } else {
      fprintf(stderr, "ERROR: read returns 0 bytes\n");
      return;
    }
    if ( (addr % 8) != 0 ) {
      fprintf(stderr, "\n");
    }
  }
  fprintf(stderr, "\n");
}

void usage()
{
  fprintf(stderr, "Usage: read_calib [-d device] \n");
  fprintf(stderr, "where\n");
  fprintf(stderr, "  the device is usually %s\n", DEFAULT_DEVICE );
}

double vector2double( unsigned char b1, unsigned char b2 )
{
  int v = b1 | (((int)b2)<<8);
  if ( v > 32768 ) v = 65538 - v;
  return v / 24000.0;
}

double matrix2double( unsigned char b1, unsigned char b2 )
{
  int v = b1 | (((int)b2)<<8);
  if ( v > 32768 ) v = 65538 - v;
  return v / 16384.0;
}


int main( int argc, char ** argv )
{
  const char * device = DEFAULT_DEVICE;

  int ac = 1;
  if ( argc <= ac) {
    usage();
    return 0;
  }

  if ( strcmp(argv[ac], "-d" ) == 0 ) {
    device = argv[++ac];
    ++ac;
  }
  
  fprintf(stderr, "Device %s \n", device );

  Serial serial( device );

  if ( ! serial.Open( ) ) {
    fprintf(stderr, "Failed to open device %s\n", device );
    return 1;
  }

  read_memory( &serial );
  fprintf(stderr, "BG  %8.4f %8.4f %8.4f\n",
    vector2double( coeff[0], coeff[1]),
    vector2double( coeff[8], coeff[9]),
    vector2double( coeff[16], coeff[17])
  );
  fprintf(stderr, "AGx %8.4f %8.4f %8.4f\n",
    matrix2double( coeff[2], coeff[3]),
    matrix2double( coeff[4], coeff[5]),
    matrix2double( coeff[6], coeff[7])
  );
  fprintf(stderr, "  y %8.4f %8.4f %8.4f\n",
    matrix2double( coeff[10], coeff[11]),
    matrix2double( coeff[12], coeff[13]),
    matrix2double( coeff[14], coeff[15])
  );
  fprintf(stderr, "  z %8.4f %8.4f %8.4f\n",
    matrix2double( coeff[18], coeff[19]),
    matrix2double( coeff[20], coeff[21]),
    matrix2double( coeff[22], coeff[23])
  );
  fprintf(stderr, "BM  %8.4f %8.4f %8.4f\n",
    vector2double( coeff[24], coeff[25]),
    vector2double( coeff[32], coeff[33]),
    vector2double( coeff[40], coeff[41])
  );
  fprintf(stderr, "AMx %8.4f %8.4f %8.4f\n",
    matrix2double( coeff[26], coeff[27]),
    matrix2double( coeff[28], coeff[29]),
    matrix2double( coeff[30], coeff[31])
  );
  fprintf(stderr, "  y %8.4f %8.4f %8.4f\n",
    matrix2double( coeff[34], coeff[35]),
    matrix2double( coeff[36], coeff[37]),
    matrix2double( coeff[38], coeff[39])
  );
  fprintf(stderr, "  z %8.4f %8.4f %8.4f\n",
    matrix2double( coeff[42], coeff[43]),
    matrix2double( coeff[44], coeff[45]),
    matrix2double( coeff[46], coeff[47])
  );
  

  serial.Close();
  return 0;
}

