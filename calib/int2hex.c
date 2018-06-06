/** @file int2hex.cpp
 *
 * @author marco corvi
 * @date march 2009
 * 
 * @brief convert calibration data from int to hex
 *
 * Extract the raw hex data and group from a calib coeff file
 * Usage:
 *        hex2int input_coeff_file > data_file
 * --------------------------------------------------------
 *  Copyright This sowftare is distributed under GPL-3.0 or later
 *  See the file COPYING.
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>

void usage()
{
  static int printed_usage = 0;
  if ( ! printed_usage ) {
    fprintf(stderr, "Usage: int2hex <calib_file> \n");
    fprintf(stderr, "The calibration file contains the calibration coefficients.\n");
    fprintf(stderr, "Output is written on stdout\n");
  }
  printed_usage = 1;
}

/** 
 * syntax:
 *    int2hex <calib_file> <data_file>
 * where
 */

int main( int argc, char ** argv )
{
  char * c_file = NULL;
  FILE * fp;
  char line[256];

  if ( argc < 2 ) {
    usage();
    return 0;
  }
  c_file = argv[1];

  fprintf(stderr, "Calibration file \"%s\"\n", c_file );
  
  fp = fopen( c_file, "r" );
  if ( fp == NULL ) {
    fprintf(stderr, "Cannot open calibration file \n");
    return 0;
  }

  // ------------------------------------------------------
  // skip coeffs
  while ( fgets( line, 256, fp ) != NULL ) {
    if ( line[0] == 'G' ) {
      char rem[32];
      int gx, gy, gz, mx, my, mz;
      int grp, ignore;
      // G: xxx xxx xxx M: xxx xxx xxx Grp: xx xx 
      // N.B. error is not read
      sscanf( line, "%s %d %d %d %s %d %d %d %s %d %d",
        rem, &gx, &gy, &gz, rem, &mx, &my, &mz, rem, &grp, &ignore );
      {
        int16_t gx0 = (int16_t)gx;
        int16_t gy0 = (int16_t)gy;
        int16_t gz0 = (int16_t)gz;
        int16_t mx0 = (int16_t)mx;
        int16_t my0 = (int16_t)my;
        int16_t mz0 = (int16_t)mz;
        // no error
        printf("0x%04x 0x%04x 0x%04x 0x%04x 0x%04x 0x%04x %2d %d\n",
          (uint16_t)gx0, (uint16_t)gy0, (uint16_t)gz0,
          (uint16_t)mx0, (uint16_t)my0, (uint16_t)mz0, grp, ignore );
      }
    }
  }
  fclose( fp );
  return 0;
}
