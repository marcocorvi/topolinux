/** @file inspect_calib.c
 *
 * @author marco corvi
 * @date 2010-10-11
 *
 * @brief inspect raw calibration data file
 *
 * Reads raw calibration data and, for each data record
 * writes the estimated azimuth, inclination, and angle
 * between G and M.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

double CompassAndClino( int16_t gx0, int16_t gy0, int16_t gz0,
                        int16_t mx0, int16_t my0, int16_t mz0,
                        double * compass, double * clino )
{
  double gx = gx0;
  double gy = gy0;
  double gz = gz0;
  double glen = sqrt( gx*gx + gy*gy + gz*gz );
  double mx = mx0;
  double my = my0;
  double mz = mz0;
  double mlen = sqrt( mx*mx + my*my + mz*mz );
  double angle = acos( (gx*mx + gy*my + gz*mz )/(glen*mlen) );
  double mgx = my * gz - mz * gy;
  double mgy = mz * gx - mx * gz;
  double mgz = mx * gy - my * gx;
  double gmgx = gy * mgz - gz * mgy;
  double gmgy = gz * mgx - gx * mgz;
  double gmgz = gx * mgy - gy * mgx;
  double egx = 0.0 * gz - 0.0 * gy;
  double egy = 0.0 * gx - 1.0 * gz;
  double egz = 1.0 * gy - 0.0 * gx;
  double gegx = gy * egz - gz * egy;
  double gegy = gz * egx - gx * egz;
  double gegz = gx * egy - gy * egx;
  double em0x = gegy * gmgz - gegz * gmgy;
  double em0y = gegz * gmgx - gegx * gmgz;
  double em0z = gegx * gmgy - gegy * gmgx;
  double s = sqrt( em0x*em0x + em0y*em0y + em0z*em0z );
  double em0g = gegx * gx + gegy * gy + gegz * gz;
  double c = gegx * gmgx + gegy * gmgy + gegz * gmgz;
  if ( em0g > 0 ) s *= -1.0;
  *clino = acos( gx / glen ) - M_PI/2;
  *compass = atan2( s, c );
  if ( *compass < 0.0 ) *compass += 2*M_PI; 
  return angle;
}

int main(int argc, char ** argv )
{
  FILE * fp;

  int16_t gx, gy, gz, mx, my, mz;
  int gx0, gy0, gz0, mx0, my0, mz0;
  double a, cl, cp;
  char * line;
  size_t n = 128;

  if ( argc <= 1 ) {
    fprintf(stderr, "Usage: %s <input_file>\n", argv[0] );
    return 0;
  }
  fp = fopen(argv[1], "r");
  if ( fp == NULL ) {
    fprintf(stderr, "Unable to open input file \"%s\"\n", argv[1] );
    return 0;
  }
  line = (char*)malloc( n );

  for ( ; ; ) {
    n = 128;
    if ( getline( &line, &n, fp ) < 0 ) break;

    // FIXME works only on raw data list
    sscanf(line, "%x %x %x %x %x %x", 
      &gx0, &gy0, &gz0, &mx0, &my0, &mz0 );
    if ( gx0 > 32768 ) gx0 = 65536 - gx0;
    if ( gy0 > 32768 ) gy0 = 65536 - gy0;
    if ( gz0 > 32768 ) gz0 = 65536 - gz0;
    if ( mx0 > 32768 ) mx0 = 65536 - mx0;
    if ( my0 > 32768 ) my0 = 65536 - my0;
    if ( mz0 > 32768 ) mz0 = 65536 - mz0;

    gx = (int16_t)gx0;
    gy = (int16_t)gy0;
    gz = (int16_t)gz0;
    mx = (int16_t)mx0;
    my = (int16_t)my0;
    mz = (int16_t)mz0;

    a = CompassAndClino( gx, gy, gz, mx, my, mz, &cp, &cl );
    printf("Compass %.2f Clino %.2f Angle %.2f \n",
      cp*180/M_PI, cl*180/M_PI, a*180/M_PI );
  }
  fclose( fp );
  free( line );
  return 0;
}

