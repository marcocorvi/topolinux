/** @file check_coverage.cpp
 * 
 * @author marco corvi
 * @date march 2009
 * 
 * @brief check how a Calibration data set covers the 4 pi angle
 * --------------------------------------------------------
 *  Copyright This sowftare is distributed under GPL-3.0 or later
 *  See the file COPYING.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>

#include "Factors.h"
#include "Vector.h"


double Angle( double compass1, double clino1, double compass2, double clino2 )
{
  double c1 = cos( clino1 );
  double z1 = sin( clino1 );
  double x1 = c1 * cos( compass1 );
  double y1 = c1 * sin( compass1 );
  double c2 = cos( clino2 );
  double z2 = sin( clino2 );
  double x2 = c2 * cos( compass2 );
  double y2 = c2 * sin( compass2 );
  return x1*x2 + y1*y2 + z1*z2; // cosine of the angle
}

void usage()
{
  static bool printed_usage = false;
  if ( ! printed_usage ) {
    printf("Usage: check_coverage <calib_data_file>\n");
    printf("The data file contains the calibration raw data and groups.\n");
  }
  printed_usage = true;
}

struct Direction
{
  double compass;
  double clino;
  double value;
};

struct Direction * angles;

int clino_angles[ 19 ] = {
   90, 80, 70, 60, 50, 40, 30, 20, 10,  0, -10, -20, -30, -40, -50, -60, -70, -80, -90 
};

int t_size[ 19 ] = {
    1,  6, 12, 18, 24, 30, 36, 42, 48, 56,  48,  42,  36,  30,  24,  18,  12,   6,   1
};

int t_dim;

int t_offset[ 19 ];

void InitDirections( )
{
  t_offset[0] = 0;
  for (int k=1; k<19; ++k ) {
    t_offset[k] = t_offset[k-1] + t_size[k-1];
  }
  t_dim = t_offset[18] + t_size[18];

  angles = (struct Direction *)malloc( t_dim * sizeof(struct Direction) );
  for (int k = 0; k<19; ++k ){
    for (int j=t_offset[k]; j<t_offset[k]+t_size[k]; ++j ) {
      angles[j].clino   = clino_angles[k] * M_PI / 180.0;
      angles[j].compass = ( 2.0 * M_PI * (j - t_offset[k]) ) / t_size[k];
      angles[j].value   = 0.9;
    }
  }
}

void UpdateDirections( double compass, double clino )
{
  #ifdef POINTWISE
    int jmax = -1;
    double cmax = -2.0;
  #endif
  for (int j=0; j<t_dim; ++j ) {
    // compute angle with direction
    double c = Angle( compass, clino, angles[j].compass, angles[j].clino );
    #ifdef POINTWISE
      if ( c > cmax ) { cmax = c; jmax = j; }
    #else
      if ( c > 0.0 ) {
        angles[j].value -= c*c;
        if ( angles[j].value < 0.0 ) angles[j].value = 0.0;
      }
    #endif
  }
  #ifdef POINTWISE
    if ( jmax >= 0 ) {
      int k = 0;
      while ( k<19 && t_offset[k]+t_size[k] < jmax ) k++;
      // printf("jmax %d (%d %d) cmax %.2f\n", jmax, k, jmax-t_offset[k], cmax );
      angles[jmax].value = 0.0;
    } else {
      printf("Error jmax < 0 \n");
    }
  #endif
}

void PrintDirections()
{
  for (int k = 0; k<19; ++k ) { 
    int off = 56 - t_size[k];
    while ( off > 0 ) { printf(" "); --off; }
    for (int j=t_offset[k]; j<t_offset[k]+t_size[k]; ++j ) {
      #ifdef POINTWISE
        printf("%c ", (angles[j].value > 0.5)? '.' : 'o' );
      #else
        printf("%2d", (int)(10*angles[j].value) );
      #endif
    }
    printf("\n");
  }
}

int main( int argc, char ** argv )
{
  char * t_file = NULL;

  if ( argc < 2 ) {
    usage();
    return 0;
  }
  t_file = argv[1];

  fprintf(stderr, "Input calibration data file \"%s\"\n", t_file );
  
  InitDirections();

  // ------------------------------------------------------
  // test
  FILE * fp = fopen( t_file, "r" );
  if ( fp == NULL ) {
    fprintf(stderr, "ERROR: Cannot open test file \n");
    return 0;
  }
  // skip coeffs
  char line[256];
  int old_grp = -10;
  double compass_avg = 0.0;
  double clino_avg   = 0.0;
  int cnt_avg = 0;
  while ( fgets( line, 256, fp ) != NULL ) {
    unsigned int gx0, gy0, gz0, mx0, my0, mz0;
    int grp;
    int ignore;
    Vector g;
    Vector m;
    sscanf( line, "%x %x %x %x %x %x %d %d",
      &gx0, &gy0, &gz0, &mx0, &my0, &mz0, &grp, &ignore );
    if ( ignore == 1 ) continue;
    int16_t gx = (int16_t)gx0;
    int16_t gy = (int16_t)gy0;
    int16_t gz = (int16_t)gz0;
    int16_t mx = (int16_t)mx0;
    int16_t my = (int16_t)my0;
    int16_t mz = (int16_t)mz0;
    // printf("Raw data (%2d): %6d %6d %6d  %6d %6d %6d ",
    //   grp, gx, gy, gz, mx, my, mz );
    g.X() = gx;
    g.Y() = gy;
    g.Z() = gz;
    m.X() = mx;
    m.Y() = my;
    m.Z() = mz;
    g *= 1.0/g.length();
    m *= 1.0/m.length();
    Vector e( 1.0, 0.0, 0.0 );
    Vector m0 = g % (m % g);
    Vector e0 = g % (e % g);
    double clino = acos( g.X() / g.length() ) - M_PI/2;
    Vector em0 = e0 % m0;
    double s = em0.length() * ( ( em0*g > 0 ) ? -1.0 : 1.0 );
    double c = e0 * m0;
    double compass = atan2( s, c );
    if ( compass < 0 ) compass += 2.0 * M_PI;
    if ( grp >= 0 && grp == old_grp ) {
      if ( cnt_avg > 0 && ( fabs( compass - compass_avg / cnt_avg ) > 1.5*M_PI ) ) {
        if ( compass > M_PI ) {
          compass -= 2.0 * M_PI; // average around 0
        } else {
          compass += 2.0 * M_PI; // average around 360
        }
      }
      clino_avg   += clino;
      compass_avg += compass;
      cnt_avg     ++;
    } else {
      if ( cnt_avg > 0 ) {
          compass_avg /= cnt_avg;
          clino_avg   /= cnt_avg;
          // printf("%8.2f %8.2f cnt %d\n", compass_avg*RAD2GRAD, clino_avg*RAD2GRAD, cnt_avg );
          UpdateDirections( compass_avg, clino_avg );
      }
      clino_avg   = clino;
      compass_avg = compass;
      cnt_avg     = 1;
      old_grp = grp;
    }
  }
  if ( cnt_avg > 0 ) {
    compass_avg /= cnt_avg;
    clino_avg   /= cnt_avg;
    // printf("%8.2f %8.2f cnt %d\n", compass_avg*RAD2GRAD, clino_avg*RAD2GRAD, cnt_avg );
    UpdateDirections( compass_avg, clino_avg );
  }
  PrintDirections();
  fclose( fp );
  return 0;
}
