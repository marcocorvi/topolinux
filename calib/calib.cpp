/** @file calib.cpp
 * 
 * @author marco corvi
 * @date march 2009
 * 
 * @brief execute the Calibration algorithm
 *
 * Input file data lines:
 *    0x.. 0x.. 0x.. 0x.. 0x.. 0x.. rem int int
 *    &gx0, &gy0, &gz0, &mx0, &my0, &mz0, rem, &grp, &ignore
 *
 * --------------------------------------------------------
 *  Copyright This sowftare is distributed under GPL-3.0 or later
 *  See the file COPYING.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "Factors.h"
#include "Calibration.h"

enum CalinMode {
  MODE_PT = 1,
#ifdef EXPERIMENTAL
  MODE_GRAD,
  MODE_DELTA,
  MODE_FIX_G,
  MODE_ITER,
#ifdef USE_GUI
  MODE_PT_DISPLAY,
  MODE_DELTA_DISPLAY,
#endif
#endif
  MODE_MAX,
};

void usage()
{
  static bool printed_usage = false;
  if ( ! printed_usage ) {
    fprintf(stderr, "Usage: calib [-i max_iter] [-p] [-h] <input_file> [output_file]\n");
    fprintf(stderr, "Options: \n");
    fprintf(stderr, "  -i max_iter   max nr. iterations [default 2000]\n");
#ifdef EXPERIMENTAL
    fprintf(stderr, "  -m mode       optimization mode\n");
    fprintf(stderr, "  -d delta      required delta (mode %d ", MODE_DELTA );
    #ifdef USE_GUI 
      fprintf(stderr, "or %d ", MODE_DELTA_DISPLAY );
    #endif
    fprintf(stderr, "only, default 0.5)\n");
#endif
    fprintf(stderr, "  -p            print input data\n");
    fprintf(stderr, "  -v            verbose\n");
    fprintf(stderr, "  -h            print usage\n");
    fprintf(stderr, "The input file contains the calibration data\n");
    fprintf(stderr, "one record per line, in the format: \n");
    fprintf(stderr, "   Gx Gy Gz Mx My Mz group ignore\n");
    fprintf(stderr, "where the G and M are hex, group and ignore are decimal\n");
#ifdef EXPERIMENTAL
    fprintf(stderr, "The mode is one of:\n");
    fprintf(stderr, "  %d PocketTopo algorithm with angular error instead of rms error\n", MODE_PT);
    fprintf(stderr, "  %d Gradient algorithm \n", MODE_GRAD );
    fprintf(stderr, "  %d PocketTopo algorithm iterated until the error is below delta\n", MODE_DELTA );
    fprintf(stderr, "  %d optimize only M (fixed G)\n", MODE_FIX_G );
    fprintf(stderr, "  %d optimize iteratively (experimental)\n", MODE_ITER );
#ifdef USE_GUI
    fprintf(stderr, "  %d mode 1 with graphical display of set planes\n", MODE_PT_DISPLAY );
    fprintf(stderr, "  %d mode 3 with graphical display of set planes\n", MODE_DELTA_DISPLAY );
#endif
#endif
    fprintf(stderr, "The calibration coefficients are written to the output_file\n");
    fprintf(stderr, "if specified.\n");
  }
  printed_usage = true;
}

/** 
 * syntax:
 *    calib <input_file>
 * where
 */
#include <time.h>

int main( int argc, char ** argv )
{
  char * in_file = NULL;
  char * out_file = NULL;

  unsigned int max_it = 2000; // Max nr. of iterations
  bool print_input = false;
  bool verbose = false;
  int mode = MODE_PT;  // 1: PocketTopo Optimize
                 // 2: Optimize2
                 // 3: OptimizeBest
                 // 4: PocketTopo Optimize with optimize_axis
                 // 5: OptimizeBest with optimize_axis
                 // 6: Optimize M at fixed G 
                 // 7: Optimize iterative
  double delta = 0.5; // required delta for OptimizeBest
  double error;
  unsigned int iter = 0;

  int ac = 1;
  while ( ac < argc ) {
    if ( strncmp(argv[ac],"-i",2) == 0 ) {
      max_it = atoi( argv[ac+1]);
      if ( max_it < 200 ) max_it = 200;
      ac += 2;
#ifdef EXPERIMENTAL
    } else if ( strncmp(argv[ac],"-m", 2) == 0 ) {
      mode = atoi( argv[ac+1] );
      if ( mode < MODE_PT || mode >= MODE_MAX ) mode = MODE_PT;
      ac += 2;
    } else if ( strncmp(argv[ac],"-d", 2) == 0 ) {
      delta = atof( argv[ac+1] );
      if ( delta < 0.001 ) delta = 0.5;
      ac += 2;
#endif
    } else if ( strncmp(argv[ac],"-p", 2) == 0 ) {
      print_input = true;
      ac ++;
    } else if ( strncmp(argv[ac],"-v", 2) == 0 ) {
      verbose = true;
      ac ++;
    } else if ( strncmp(argv[ac],"-h", 2) == 0 ) {
      usage();
      ac ++;
    } else {
      break;
    }
  }

  if ( ac >= argc ) {
    usage();
    return 0;
  }
  in_file = argv[ac];
  ac ++;
  if ( argc > ac ) {
    out_file = argv[ac];
  }
  
  if ( verbose ) {
    fprintf(stderr, "Calibration data file \"%s\"\n", in_file );
    if ( out_file ) {
      fprintf(stderr, "Calibration coeff file \"%s\"\n", out_file );
    }
    fprintf(stderr, "Max nr. iterations %d \n", max_it );
    #ifdef EXPERIMENTAL
      fprintf(stderr, "Optimization mode: nr. %d\n", mode );
    #endif
  }

  FILE * fp = fopen( in_file, "r" );
  if ( fp == NULL ) {
    fprintf(stderr, "ERROR: cannot open input file \"%s\"\n", in_file);
    return 0;
  }

  Calibration calib;
  int16_t gx, gy, gz, mx, my, mz;
  int grp;
  int ignore;
  int cnt = 0;
  char line[256];
  if ( fgets(line, 256, fp) == NULL ) { // empty file
    fclose( fp );
    return false;
  }
  if ( line[0] == '0' && line[1] == 'x' ) { // calib-coeff format
    if ( verbose ) {
      fprintf(stderr, "Input file format: calib-coeff\n");
    }
    int gx0, gy0, gz0, mx0, my0, mz0;
    // skip coeffs;
    for (int k=1; k<6; ++k ) fgets(line, 256, fp);
    // skip one more line
    fgets(line, 256, fp);
    // printf("reading after: %s\n", line);
    while ( fgets(line, 255, fp ) ) {
      // fprintf(stderr, line );
      char rem[32];
      if ( line[0] == '#' ) continue;
      sscanf( line, "G: %d %d %d M: %d %d %d %s %d %d",
          &gx0, &gy0, &gz0, &mx0, &my0, &mz0, rem, &grp, &ignore);
      gx = (int16_t)( gx0  );
      gy = (int16_t)( gy0  );
      gz = (int16_t)( gz0  );
      mx = (int16_t)( mx0  );
      my = (int16_t)( my0  );
      mz = (int16_t)( mz0  );
      calib.AddValues( gx, gy, gz, mx, my, mz, cnt, grp, ignore );
      ++cnt;
      if ( verbose ) {
        fprintf(stderr, 
          "%d G: %d %d %d  M: %d %d %d  [%d]\n",
          cnt, gx, gy, gz, mx, my, mz, grp );
      }
    }
  } else {
    if ( verbose ) {
      fprintf(stderr, "Input file format: calib-data\n");
    }
    rewind( fp );
    unsigned int gx0, gy0, gz0, mx0, my0, mz0;
    while ( fgets(line, 255, fp ) ) {
      // fprintf(stderr, line );
      if ( line[0] == '#' ) continue;
      sscanf( line, "%x %x %x %x %x %x %d %d",
          &gx0, &gy0, &gz0, &mx0, &my0, &mz0, &grp, &ignore);
      gx = (int16_t)( gx0 & 0xffff );
      gy = (int16_t)( gy0 & 0xffff );
      gz = (int16_t)( gz0 & 0xffff );
      mx = (int16_t)( mx0 & 0xffff );
      my = (int16_t)( my0 & 0xffff );
      mz = (int16_t)( mz0 & 0xffff );
      calib.AddValues( gx, gy, gz, mx, my, mz, cnt, grp, ignore );
      ++cnt;
      if ( verbose ) {
        fprintf(stderr, 
          "%d G: %d %d %d  M: %d %d %d  [%d]\n",
          cnt, gx, gy, gz, mx, my, mz, grp );
      }
    }
  }
  fclose( fp );


  if ( print_input ) {
    calib.PrintValues();
    // { int i; scanf("%d", &i); }
    // calib.CheckInput();
    // { int i; scanf("%d", &i); }
    calib.PrintGroups();
  }

  calib.PrepareOptimize();
  
  switch ( mode ) {
  case MODE_PT:
    iter = calib.Optimize( delta, error, max_it );
    break;
#ifdef EXPERIMENTAL
  case MODE_GRAD:
    iter = calib.Optimize2( delta, error, max_it );
    break;
  case MODE_DELTA:
    iter = calib.OptimizeBest( delta, error, max_it );
    break;
  case MODE_FIX_G:
    {
      // TODO set the G coeffs
      Vector b( 0.0037, -0.0725, -0.0300);
      Vector ax( 1.7678, -0.0013, 0.0119);
      Vector ay( -0.0023, 1.7657, -0.0016);
      Vector az(-0.0112, -0.0016, 1.7660);
      Matrix a( ax, ay, az );
      iter = calib.Optimize( delta, error, max_it );
      fprintf(stdout, "Iterations %d\n",  iter);
      fprintf(stdout, "Delta      %.4f\n", delta );
      fprintf(stdout, "Max error  %.4f\n", error );
      calib.PrintCoeffs();
      delta = 0.5;
      iter = 0;
      error = 0.0;
      calib.SetGCoeffs( a, b );
      iter = calib.OptimizeM( delta, error, max_it );
    }
    break;
  case MODE_ITER:
    /*
    Vector b( 0.0037, -0.0725, -0.0300);
    Vector ax( 1.7678, -0.0013, 0.0119);
    Vector ay( -0.0023, 1.7657, -0.0016);
    Vector az(-0.0112, -0.0016, 1.7660);
    Matrix a( ax, ay, az );
    calib.SetGCoeffs( a, b );
    */
    iter = calib.OptimizeIterative( delta, error, max_it );
    break;
#ifdef USE_GUI
  case MODE_PT_DISPLAY:
    calib.SetShowGui( true );
    iter = calib.Optimize( delta, error, max_it );
    // OptimizeExp( delta, error, max_it, true );
    break;
  case MODE_DELTA_DISPLAY:
    calib.SetShowGui( true );
    iter = calib.OptimizeBest( delta, error, max_it, true );
    break;
#endif
#endif // EXPERIMENTAL
  default:
    fprintf(stderr, "Unexpected calibration mode %d\n", mode );
    break;
  }

  fprintf(stdout, "Iterations  %d\n",  iter);
  fprintf(stdout, "Delta       %.4f\n", delta );
  fprintf(stdout, "Max error   %.4f\n", error );
  fprintf(stderr, "M dip angle %.2f\n", calib.GetDipAngle() );
  if ( verbose ) {
    calib.PrintCoeffs();
    // calib.CheckInput();
  }

#if 0
  unsigned char data[48];
  calib.GetCoeff( data );
  for (int k=0; k<48; ++k) {
    printf("0x%02x ", data[k] );
    if ( ( k % 4 ) == 3 ) printf("\n");
  }
#endif

  if ( out_file != NULL ) {
    calib.PrintCalibrationFile( out_file );
  }

  return 0;
}
