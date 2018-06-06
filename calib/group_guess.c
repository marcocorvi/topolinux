/** @file group_guess.c
 *
 * @author marco corvi
 * @date march 2009
 *
 * @brief compute from hex data file the compass/clino and evaluate the groups guess
 *
 * Usage:
 *        hex2int <input-file>
 *
 * Output on stdout. Redirect if necessary.
 * To use the ouput file edit and fix the group numbers.
 *
 * --------------------------------------------------------
 *  Copyright This sowftare is distributed under GPL-3.0 or later
 *  See the file COPYING.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

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
  return acos( x1*x2 + y1*y2 + z1*z2 ) * 180.0 / M_PI;
}

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

void
print_usage()
{
  fprintf(stderr, "Usage: group_guess [options] hex_data_file\n");
  fprintf(stderr, "Options:\n");
  fprintf(stderr, "  -v   verbose: prints compass and clino values \n");
  fprintf(stderr, "  -h   help\n");
}
  

int main( int argc, char ** argv )
{
  FILE * fp;
  int print_compass_clino =0;
  int ac = 1;
  int nr = 0;
  double angle = 0.0;
  FILE * fpout = stdout;

  while ( ac < argc && argv[ac][0] == '-' ) {
    if ( argv[ac][1] == 'v' ) {
      print_compass_clino = 1;
    } else if ( argv[ac][1] == 'h' ) {
      print_usage();
    } else {
      print_usage();
      return 0;
    }
    ++ ac;
  }

  if ( argc < ac+1 ) {
    print_usage();
    return 0;
  }

  if ( argv[ac+1] && strlen(argv[ac+1]) > 0 ) {
    fp = fopen( argv[ac+1], "w");
    if ( fp != NULL ) fpout = fp;
  }

  fp = fopen( argv[ac], "r" );
  if ( fp == NULL ) {
    fprintf(stderr, "ERROR: cannot open input file \"%s\"\n", argv[ac] );
    return 0;
  }

  {
    unsigned int gx, gy, gz, mx, my, mz;
    // int group;
    int guess = 1;
    int cnt = 0;
    double compass1 = 0.0, clino1=0.0, compass2=0.0, clino2=0.0;
    char line[256];

    while ( fgets(line, 256, fp ) ) {
      double a;
      int16_t gxs, gys, gzs, mxs, mys, mzs;
      sscanf( line, "%x %x %x %x %x %x", &gx, &gy, &gz, &mx, &my, &mz );
      gxs = (int16_t)( gx & 0xffff );
      gys = (int16_t)( gy & 0xffff );
      gzs = (int16_t)( gz & 0xffff );
      mxs = (int16_t)( mx & 0xffff );
      mys = (int16_t)( my & 0xffff );
      mzs = (int16_t)( mz & 0xffff );
      a = CompassAndClino( gxs, gys, gzs, mxs, mys, mzs, &compass1, &clino1 );
      angle += a;
      ++nr;
      if ( cnt == 0 ) {
        compass2 = compass1;
        clino2   = clino1;
      } else {
        if ( Angle( compass1, clino1, compass2, clino2 ) > 20.0 ) {
          compass2 = compass1;
          clino2   = clino1;
          ++ guess;
        }
      }
      ++ cnt;
      // ignore 0, no error
      fprintf(fpout, "0x%04x 0x%04x 0x%04x 0x%04x 0x%04x 0x%04x %2d 0",
        gx, gy, gz, mx, my, mz, guess );
      if ( print_compass_clino ) {
        fprintf(fpout, " %7.1f %7.1f %.2f\n", 
          compass1 * 180.0/M_PI, clino1 * 180.0/M_PI, a*180.0/M_PI );
      } else {
        fprintf(fpout, "\n");
      }
    }
  }
  fclose( fp );
  if ( fpout != stdout ) {
    fclose( fpout );
  }
  if ( print_compass_clino ) {
    printf("Average G-M angle: %.2f \n", angle/nr*180.0/M_PI );
  }
  return 0;
}

