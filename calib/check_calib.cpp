/** @file check_calib.cpp
 * 
 * @author marco corvi
 * @date march 2009
 * 
 * @brief check a Calibration against a set of raw data
 * --------------------------------------------------------
 *  Copyright This sowftare is distributed under GPL-3.0 or later
 *  See the file COPYING.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>

#include <vector>

#include "Factors.h"
#include "Vector.h"


void usage()
{
  static bool printed_usage = false;
  if ( ! printed_usage ) {
    fprintf(stderr, "Usage: check_calib2 [options] <calib_file_1> ... <calib_file_N> <test_file>\n");
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "   -v   verbose. Print max differences for each data.\n");
    fprintf(stderr, "   -h   help. Print this help.\n");
    fprintf(stderr, "The calibration files contain the calibration coefficients.\n");
    fprintf(stderr, "The test file is a raw (hex) calib data file with format\n");
    fprintf(stderr, "   gx gy gz mx my mz ...\n");
  }
  printed_usage = true;
}

/** 
 * syntax:
 *    check_calib <calib_file> <test_file>
 * where
 */
#include <time.h>

struct Data
{
  int16_t gx;
  int16_t gy;
  int16_t gz; 
  int16_t mx;
  int16_t my;
  int16_t mz;
  double compass;  // average compass
  double clino;    // average clino

  Data( unsigned int gx0, unsigned int gy0, unsigned int gz0,
        unsigned int mx0, unsigned int my0, unsigned int mz0 )
    : compass( 0.0 )
    , clino( 0.0 )
  { 
    gx = (int16_t)gx0;
    gy = (int16_t)gy0;
    gz = (int16_t)gz0;
    mx = (int16_t)mx0;
    my = (int16_t)my0;
    mz = (int16_t)mz0;
  }
};

struct Transform
{
  double mG[3][4]; // bGx aGx[x] aGx[y] aGx[z]
  double mM[3][4];
  double compass_err_avg;  // average error absolute diff
  double clino_err_avg;    // average error
  double compass_err_std;  // stddev error
  double clino_err_std;    // stddev error
  double err_compass_avg;  // average compass error (with sign)
  double err_compass_std;
  double err_compass_max;
  double err_clino_avg;
  double err_clino_std;
  double err_clino_max;


  Transform( const char * filename );

  void Compute( int16_t gx, int16_t gy, int16_t gz, 
                int16_t mx, int16_t my, int16_t mz, 
                double * compass, double * clino  );

  void Compute( struct Data & data, double * compass, double * clino  )
  {
    Compute( data.gx, data.gy, data.gz, data.mx, data.my, data.mz, compass, clino );
  }

  /**
   * @param nd   number of data
   */
  void EvalErrors( int nd );

  void Dump();
};


/** Transform read only the 48 bytes of the calibratin coeffs
 */
Transform::Transform( const char * filename )
  : compass_err_avg( 0.0 )
  , clino_err_avg( 0.0 )
  , compass_err_std( 0.0 )
  , clino_err_std( 0.0 )
  , err_compass_avg( 0.0 )
  , err_compass_std( 0.0 )
  , err_compass_max( 0.0 )
  , err_clino_avg( 0.0 )
  , err_clino_std( 0.0 )
  , err_clino_max( 0.0 )
{
  FILE * fp = fopen( filename, "r" );
  if ( fp == NULL ) {
    fprintf(stderr, "Cannot open calibration file \"%s\"\n", filename);
    throw;
  }
  unsigned int x0, x1;
  for ( int row = 0; row < 3; ++row ) {
    for (int col=0; col<4; ++col ) {
      if ( fscanf( fp, "%x %x", &x0, &x1 ) != 2 ) {
        fprintf(stderr, "Input error on calibration file \'%s\"\n", filename);
        throw;
      }
      int16_t value = (int16_t)( ( (x1<<8) | x0 ) & 0xffff );
      if ( col == 0 ) { mG[row][col] = value / FV; }
      else            { mG[row][col] = value / FM; }
    }
  }
  for ( int row = 0; row < 3; ++row ) {
    for (int col=0; col<4; ++col ) {
      if ( fscanf( fp, "%x %x", &x0, &x1 ) != 2 ) {
        fprintf(stderr, "Input error on calibration file \"%s\"\n", filename);
        throw;
      }
      int16_t value = (int16_t)( ( (x1<<8) | x0 ) & 0xffff );
      if ( col == 0 ) { mM[row][col] = value / FV; }
      else            { mM[row][col] = value / FM; }
    }
  }
  fclose( fp );
}

void
Transform::Dump()
{
  printf("G-matrix:\n");
  for (int k=0; k<3; ++k) {
    for (int j=0; j<4; ++j ) { 
      printf("%8.5f ", mG[k][j] );
    }
    printf("\n");
  }
  printf("M-matrix:\n");
  for (int k=0; k<3; ++k) {
    for (int j=0; j<4; ++j ) { 
      printf("%8.5f ", mM[k][j] );
    }
    printf("\n");
  }
}

void
Transform::EvalErrors( int nd )
{
  compass_err_avg /= nd;
  compass_err_std /= nd;
  clino_err_avg   /= nd;
  clino_err_std   /= nd;
  err_compass_avg /= nd;
  err_compass_std /= nd;
  err_clino_avg   /= nd;
  err_clino_std   /= nd;
  compass_err_std = sqrt( compass_err_std - compass_err_avg*compass_err_avg );
  clino_err_std   = sqrt( clino_err_std   - clino_err_avg*clino_err_avg );
  err_compass_std = sqrt( err_compass_std - err_compass_avg * err_compass_avg );
  err_clino_std   = sqrt( err_clino_std   - err_clino_avg * err_clino_avg );
}

void 
Transform::Compute( int16_t gx, int16_t gy, int16_t gz, int16_t mx, int16_t my, int16_t mz, double * compass, double * clino  )
{
  Vector g;
  Vector m;
  g.X() = mG[0][0] + mG[0][1] * gx + mG[0][2] * gy + mG[0][3] * gz;
  g.Y() = mG[1][0] + mG[1][1] * gx + mG[1][2] * gy + mG[1][3] * gz;
  g.Z() = mG[2][0] + mG[2][1] * gx + mG[2][2] * gy + mG[2][3] * gz;
  m.X() = mM[0][0] + mM[0][1] * mx + mM[0][2] * my + mM[0][3] * mz;
  m.Y() = mM[1][0] + mM[1][1] * mx + mM[1][2] * my + mM[1][3] * mz;
  m.Z() = mM[2][0] + mM[2][1] * mx + mM[2][2] * my + mM[2][3] * mz;
  g *= 1.0/g.length();
  m *= 1.0/m.length();
  Vector e( 1.0, 0.0, 0.0 );
  Vector m0 = g % (m % g);
  Vector e0 = g % (e % g);
  double clino0 = acos( g.X() / g.length() ) - M_PI/2;
  Vector em0 = e0 % m0;
  double s = em0.length() * ( ( em0*g > 0 ) ? -1.0 : 1.0 );
  double c = e0 * m0;
  double compass0 = atan2( s, c );
  compass0 *= RAD2GRAD;
  if ( compass0 < 0 ) compass0 += 360;
  clino0   *= RAD2GRAD;
  *compass = compass0;
  *clino   = clino0;
}

// --------------------------------------------------------------

int main( int argc, char ** argv )
{
  char * t_file = NULL;
  struct Transform ** transform;
  bool verbose = false;

  int ac = 1;
  while ( ac < argc ) {
    if ( argv[ac][0] == '-' ) {
      if ( argv[ac][1] == 'v' ) {
        verbose = true;
      } else if ( argv[ac][1] == 'h' ) {
        usage();
      } else {
        usage();
        return 0;
      }
      ++ ac;
    } else {
      break;
    }
  }

  if ( argc - ac < 3 ) {
    usage();
    return 0;
  }
  t_file = argv[argc-1];

  int nk = argc - ac - 1;

  if ( verbose ) {
    fprintf(stderr, "Calibration files: ");
    for ( int k=0; k<nk; ++k) fprintf(stderr, argv[ac+k]);
    fprintf(stderr, "\n");
    fprintf(stderr, "Test file \"%s\"\n", t_file );
  }
 
  transform = (struct Transform **)malloc( nk * sizeof(struct Transform*) );
  for (int k=0; k<nk; ++k ) {
    transform[k] = new Transform( argv[ac+k] );
    // transform[k]->Dump();
  }

  // ------------------------------------------------------
  // test
  std::vector< Data * > data;
  FILE * fp = fopen( t_file, "r" );
  if ( fp == NULL ) {
    fprintf(stderr, "ERROR: Cannot open test file \n");
    return 0;
  }
  // skip coeffs
  char line[256];
  while ( fgets( line, 256, fp ) != NULL ) {
    unsigned int gx0, gy0, gz0, mx0, my0, mz0;
    int grp;
    // use all the lines, even the "ignore" ones
    sscanf( line, "%x %x %x %x %x %x %d ",
      &gx0, &gy0, &gz0, &mx0, &my0, &mz0, &grp );
    Data * pd = new Data( gx0, gy0, gz0, mx0, my0, mz0 );
    data.push_back( pd );

    double compass_avg = 0.0; 
    double clino_avg   = 0.0;
    double compass, clino;
    double compass_min = 720.0, compass_max = -720.0;;
    double clino_min = -180.0,  clino_max = 180.0;
    for (int k=0; k<nk; ++k ) {
      transform[k]->Compute( *pd, &compass, &clino );
      clino_avg += clino;
      if ( k > 0 ) {
        if ( fabs( compass - compass_avg / k ) > 270.0 ) {
          if ( compass > 180.0 ) {
            compass -= 360.0; // average around 0
          } else {
            compass += 360; // average around 360
          }
        }
        if ( compass > compass_max ) compass_max = compass;
        if ( compass < compass_min ) compass_min = compass;
        if ( clino > clino_max ) clino_max = clino;
        if ( clino < clino_min ) clino_min = clino;
      } else {
        compass_min = compass_max = compass;
        clino_min   = clino_max   = clino;
      }
      compass_avg += compass;
    }
    compass_avg /= nk;
    clino_avg   /= nk;
    pd->compass = compass_avg;
    pd->clino   = clino_avg;

    for (int k=0; k<nk; ++k ) {
      transform[k]->Compute( *pd, &compass, &clino );
      double compass_err = fabs(compass - compass_avg);
      double clino_err   = fabs(clino - clino_avg);
      double err_compass = compass - compass_avg;
      double err_clino   = clino - clino_avg;
      if ( fabs( compass - compass_avg ) > 270.0 ) {
        if ( compass > 180.0 ) {
          compass -= 360.0; // average around 0
        } else {
          compass += 360; // average around 360
        }
        compass_err = fabs(compass - compass_avg);
        err_compass = compass - compass_avg;
      }
      transform[k]->compass_err_avg += compass_err;
      transform[k]->compass_err_std += compass_err*compass_err;
      transform[k]->clino_err_avg   += clino_err;
      transform[k]->clino_err_std   += clino_err*clino_err;
      transform[k]->err_compass_avg += err_compass;
      transform[k]->err_compass_std += err_compass*err_compass;
      if ( transform[k]->err_compass_max < fabs( err_compass ) )
        transform[k]->err_compass_max = fabs( err_compass );
      transform[k]->err_clino_avg   += err_clino;
      transform[k]->err_clino_std   += err_clino*err_clino;
      if ( transform[k]->err_clino_max < fabs( err_clino ) )
        transform[k]->err_clino_max = fabs( err_clino );
    }
    if ( verbose ) {
      fprintf(stderr, "%2d: %8.4f %8.4f \n", 
              grp, compass_max - compass_min, clino_max - clino_min );
    }
  }
  fclose( fp );

  int nd =  data.size();
  printf("    Errors avg-std %8s %8s %8s %8s %8s \n", 
    "Absolute", "", "Signed", "", "Max" );
    
  for (int k=0; k<nk; ++k ) {
    transform[k]->EvalErrors( nd );
    /*
    double s1 = M_PI * transform[k]->compass_err_avg * transform[k]->compass_err_avg;
    double s2 = 2*M_PI/(M_PI-2.0) * transform[k]->compass_err_std * transform[k]->compass_err_std;
    double s3 = M_PI * transform[k]->clino_err_avg * transform[k]->clino_err_avg;
    double s4 = 2*M_PI/(M_PI-2.0) * transform[k]->clino_err_std * transform[k]->clino_err_std;
    */
    printf("%2d: Errors compass %8.4f %8.4f %8.4f %8.4f %8.4f  \n", k,
      transform[k]->compass_err_avg, transform[k]->compass_err_std,
      transform[k]->err_compass_avg, transform[k]->err_compass_std,
      transform[k]->err_compass_max );
      // s1, s2 );
    printf("             clino %8.4f %8.4f %8.4f %8.4f %8.4f  \n",
      transform[k]->clino_err_avg, transform[k]->clino_err_std,
      transform[k]->err_clino_avg, transform[k]->err_clino_std,
      transform[k]->err_clino_max );
      // s3, s4 );
    printf("\n");
  }
  for (int k=0; k<nk; ++k ) {
    delete transform[k];
  }
  free( transform );
  for (int k=0; k<nd; ++k ) {
    delete data[k];
  }
  return 0;
}
