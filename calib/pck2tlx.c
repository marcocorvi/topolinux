/** @file pck2tlx.c
 *
 * @author marco corvi
 * @date march 2009
 *
 * @brief Convert PocketTopo calib file to TopoLinux calib data file
 * 
 * Usage:
 *        pck2tlx <input-file>
 * output is written to stdout. Redirect if needed.
 * input file is a calibration file exported by PocketTopo as text
 * --------------------------------------------------------
 *  Copyright This sowftare is distributed under GPL-3.0 or later
 *  See the file COPYING.
 */
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void
Usage()
{
  printf("Usage: pck2tlx <input-file>\n");
  printf("Convert PocketTopo calibration file to TopoLinux format.\n");
  printf("Output is written to stdout. Redirect if needed.\n");
  printf("Input file is a calibration file exported by PocketTopo as text.\n");
}

int main( int argc, char ** argv )
{
  char * line = NULL;
  unsigned int n = 0;
  FILE * fp = NULL;
  int index = 0;
  int group = 1;

  if ( argc <= 1 || strcmp(argv[1], "-h") == 0 ) {
    Usage();
    return 0;
  }
  fp = fopen( argv[1], "r" );
  if ( fp == NULL ) {
    fprintf(stderr, "%s: cannot open file %s\n", argv[0], argv[1] );
    return 0;
  }

  getline( &line, &n, fp ); // skip header line
  while ( getline( &line, &n, fp ) >= 0 ) {
    char * ch = line;
    int gx, gy, gz, mx, my, mz;
    while ( ch && *ch && isspace(*ch) ) ch++;
    if ( strlen(ch) == 0 ) break;
    if ( ch && ( *ch == 'A' || *ch == 'B' ) ) ch++;
    while ( ch && *ch && isspace(*ch) ) ch++;
    sscanf( ch, "%d %d %d %d %d %d", &gx, &gy, &gz, &mx, &my, &mz);
    // TODO for calib-coeff file it is different
    printf("0x%02x%02x 0x%02x%02x 0x%02x%02x 0x%02x%02x 0x%02x%02x 0x%02x%02x %d 0\n",
      (unsigned char)((gx>>8) & 0xff), (unsigned char)((gx) & 0xff), 
      (unsigned char)((gy>>8) & 0xff), (unsigned char)((gy) & 0xff), 
      (unsigned char)((gz>>8) & 0xff), (unsigned char)((gz) & 0xff), 
      (unsigned char)((mx>>8) & 0xff), (unsigned char)((mx) & 0xff), 
      (unsigned char)((my>>8) & 0xff), (unsigned char)((my) & 0xff), 
      (unsigned char)((mz>>8) & 0xff), (unsigned char)((mz) & 0xff), 
      group );
    ++ index;
    if ( index % 4 == 0 ) {
      if ( index >= 16 ) group = -1;
      else ++group;
    }
    free( line );
    line = NULL;
  }
  fclose( fp );
  return 0;
}
  
  
