/** @file data2tlx.cpp
 *
 * @author marco corvi
 * @date apr 2009
 *
 * @brief convert data dumped from distox to topolinux format
 * --------------------------------------------------------
 *  Copyright This sowftare is distributed under GPL-3.0 or later
 *  See the file COPYING.
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

/*
 * FIXME not sure about how to compute the average roll
 */
void 
computeAverage( double * d0, double * b0, double * c0, double * r0,
                int cnt, 
                double & dave, double & bave, double & cave, double & rave )
{
  assert( cnt > 0 );
  dave = d0[0];
  bave = b0[0];
  cave = c0[0];
  rave = r0[0];
  for ( int k=1; k<cnt; ++k) {
    dave += d0[k];
    cave += c0[k];
    if ( bave/k < 90 && b0[k] > 270 ) {
      bave += (b0[k] - 360.0);
    } else if ( bave/k > 270 && b0[k] < 90 ) {
      bave += (b0[k] + 360.0);
    } else {
      bave += b0[k];
    }
    rave += r0[k];
  }
  dave /= cnt;
  bave /= cnt;
  cave /= cnt;
  rave /= cnt;
  if ( bave < 0.0 ) bave += 360.0;
  else if ( bave > 360.0 ) bave -= 360.0;
}

void
write( FILE * fp, int from, int to, 
       double * d0, double * b0, double * c0, double * r0,
       int cnt )
{
  assert( cnt > 0 );
  if ( cnt == 1 ) {
    fprintf(fp, "\"%d\" \"\" %.2f %.2f %.2f %.2f 0 0 1\n",
      from, d0[0], b0[0], c0[0], r0[0] );
  } else {
    double da, ba, ca, ra;
    computeAverage( d0, b0, c0, r0, cnt, da, ba, ca, ra );
    fprintf(fp, "\"%d\" \"%d\" %.2f %.2f %.2f %.2f 0 0 %d\n",
      from, to, da, ba, ca, ra, cnt );
    for (int k=0; k<cnt; ++k ) {
      fprintf(fp, "@ %.2f %.2f %.2f %.2f\n", d0[k], b0[k], c0[k], r0[k] );
    }
  }
}

bool 
isClose( double d1, double b1, double c1, double d2, double b2, double c2 )
{
  double thr = 2.0;
  double thr2 = 2 * thr;
  if ( fabs( c1 - c2 ) > thr ) return false;
  if ( fabs( d1 - d2 ) > thr*d1/60.0 ) return false;
  if ( b1 < thr2 && b2 > 360.0 - thr2 ) {
    if ( fabs( b1 - b2 + 360.0 ) > thr ) return false;
  } else if ( b1 > 360.0 - thr2 && b2 < thr2 ) {
    if ( fabs( b2 - b1 + 360.0 ) > thr ) return false;
  } else {
    if ( fabs( b1 - b2 ) > thr ) return false;
  }
  return true;
}

int main( int argc, char ** argv ) 
{
  bool forward = true; 

  char * date = NULL;

  while ( argc > 1 && argv[1][0] == '-' ) {
    if ( argv[1][1] == 'b' ) {
      forward = false;
    } else if ( argv[1][1] == 'd' ) {
      argc --;
      argv ++;
      if ( strlen( argv[1] ) >= 10 ) {
        date = argv[1];
        date[4] = ' ';
        date[7] = ' ';
        date[10] = 0;
      }
    }
    argc --;
    argv ++;
  }
  if ( argc < 2 ) {
    fprintf(stderr, "Usage: tlx_data2tlx <input_file> [<output_file>]\n");
    fprintf(stderr, "where the input_file is the output of dump_data.\n");
    fprintf(stderr, "If the output_file is not specified, output is \n");
    fprintf(stderr, "written to stdout.\n");
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -b    shots are backward\n");
    fprintf(stderr, "  -d YYYY.MM.DD survey date \n");
    return 1;
  }
  FILE * out = stdout;
  FILE * in = fopen( argv[1], "r" );
  if ( in == NULL ) {
    fprintf(stderr, "Error: cannot open input file \"%s\"\n", argv[1] );
    return 1;
  }
  if ( argc > 2 ) {
    out = fopen( argv[2], "w" );
    if ( out == NULL ) {
      fprintf(stderr, "Warning: cannot open output file \"%s\"\n", argv[2] );
      out = stdout;
    }
  }

  if ( date != NULL ) {
    fprintf( out, "# date %s\n", date );
  }

  char line[128];
  double d0[10], b0[10], c0[10], r0[10];
  double dave, bave, cave, rave;
  int cnt = 0;
  int from = 0;
  int to = 1;
  while ( fgets( line, 128, in ) != NULL ) {
    unsigned int xd, xb, xc, xr;
    double d, b, c, r;
    if ( sscanf( line, "%x %x %x %x %lf %lf %lf %lf", 
                 &xd, &xb, &xc, &xr, &d, &b, &c, &r ) != 8 ) {
      xr = 0;
      r = 0.0;
      if ( sscanf( line, "%x %x %x %lf %lf %lf ", 
                   &xd, &xb, &xc, &d, &b, &c) != 6 ) {
        // ERROR
      }
    }

    if ( cnt == 1 ) {
      dave = d0[0];
      bave = b0[0];
      cave = c0[0];
      rave = r0[0];
    } else if ( cnt > 1 ) {
      computeAverage( d0, b0, c0, r0, cnt, dave, bave, cave, rave );
    }
    if ( cnt > 0 ) {
      if ( cnt > 1 ) {
        from = to;
        ++ to;
      }
      if ( ! isClose( dave, bave, cave, d, b, c ) ) {
        if ( forward ) {
          write( out, from, to, d0, b0, c0, r0, cnt );
        } else {
          write( out, to, from, d0, b0, c0, r0, cnt );
        }
        cnt = 0;
      }
    }
    d0[cnt] = d;
    b0[cnt] = b;
    c0[cnt] = c;
    r0[cnt] = r;
    ++ cnt;
  }
  fclose( in );
  if ( cnt > 0 ) {
    if ( forward ) {
      write( out, from, to, d0, b0, c0, r0, cnt );
    } else {
      write( out, to, from, d0, b0, c0, r0, cnt );
    }
  }
  fclose( out );
  return 0;
}
