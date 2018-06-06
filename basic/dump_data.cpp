/** @file dump_data.cpp
 *
 * @author marco corvi
 * @date jan 2009
 *
 * @brief get the data from the DistoX
 *
 * --------------------------------------------------------
 *  Copyright This sowftare is distributed under GPL-3.0 or later
 *  See the file COPYING.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "defaults.h"
#include "Protocol.h"

void usage()
{
  fprintf(stderr, 
    "Usage: dump_data [options] number\n");
  fprintf(stderr, "Options:\n");
  fprintf(stderr, " -d device      RFCOMM serial device [%s]\n", DEFAULT_DEVICE );
  fprintf(stderr, " -c calib_file  calibration data output file [%s]\n", DEFAULT_CALIB_DATA_FILE );
  fprintf(stderr, " -m data_file   measurement data output file [%s]\n", DEFAULT_DATA_FILE );
  fprintf(stderr, " -v             verbose\n");
  fprintf(stderr, " number is the number of data to retrieve [default: all]\n");
}

int main( int argc, char ** argv )
{
  const char * device =  DEFAULT_DEVICE;
  const char * calib_file = DEFAULT_CALIB_DATA_FILE;
  const char * data_file  = DEFAULT_DATA_FILE;
  bool verbose = false;
  int number = -1;
  int ac = 1;
  while ( argc > ac ) {
    if ( strcmp( argv[ac], "-d" ) == 0 && argc >= ac+1 ) {
      device = argv[ac+1];
      ac += 2;
      fprintf(stderr, "using device %s\n", device );
    } else if ( strcmp( argv[ac], "-c" ) == 0 && argc >= ac+1 ) {
      calib_file = argv[ac+1];
      ac += 2;
    } else if ( strcmp( argv[ac], "-m" ) == 0 && argc >= ac+1 ) {
      data_file = argv[ac+1];
      ac += 2;
      fprintf(stderr, "using datafile %s\n", data_file );
    } else if ( strcmp( argv[ac], "-v" ) == 0 ) {
      verbose = true;
      ac += 1;
    } else {
      break;
    }
  }
  if ( ac < argc ) {
    number = atoi( argv[ac] );
    ac ++;
  }

  Protocol proto( device, true );
  proto.Open();
  if ( ! proto.IsOpen() ) {
    fprintf(stderr, "ERROR: failed to open protocol [device %s]\n", device);
    return 1;
  }

  ProtoError err = PROTO_OK;
/*  
  fprintf(stderr, "Connected to %s. Select command:\n", device);
  fprintf(stderr, "  [0] no command \n");
  fprintf(stderr, "  [1] CALIB_START \n");
  fprintf(stderr, "  [2] CALIB_STOP \n");
  fprintf(stderr, "  [3] SILENT_START \n");
  fprintf(stderr, "  [4] SILENT_STOP \n");
  int ch = getchar();
  switch( ch ) {
    case '1':
      err = proto.SendCommand( CALIB_START );
      break;
    case '2':
      err = proto.SendCommand( CALIB_STOP );
      break;
    case '3':
      err = proto.SendCommand( SILENT_START );
      break;
    case '4':
      err = proto.SendCommand( SILENT_STOP );
      break;
    default:
      break;
  }
  if ( err != PROTO_OK ) {
    fprintf(stderr, "Command send failed: %s\n", ProtoErrorStr(err) );
  }
*/

  int cnt = 0;
  // fprintf(stderr, "Reading: ");
  while ( ( err = proto.ReadData() ) == PROTO_OK ) {
    cnt ++;
    if ( verbose ) {
      if ( ( cnt % 10 ) == 0 ) {
        fprintf(stderr, "*");
      } else {
        fprintf(stderr, ".");
      }
      if ( ( cnt % 50 ) == 0 ) fprintf(stderr, "\n");
    }
    number --;
    if ( number == 0 ) break;
  }
  fprintf(stderr, "Read %d data\n", cnt );
  if ( err != PROTO_OK && err != PROTO_TIMEOUT ) {
    fprintf(stderr, "ERROR: Read failed: %s\n", ProtoErrorStr(err) );
  }

  // close the connection with the device
  proto.Close();

  unsigned int kc = proto.CalibSize();
  unsigned int kd = proto.DataSize();

  unsigned char b[8];

  if ( kd > 0 ) {
    fprintf(stderr, "Measurement data %d \n", kd );
    if ( data_file != NULL ) {
      FILE * fpd = fopen( data_file, "w");
      if ( fpd ) {
        fprintf(stderr, "Writing measurement data to \"%s\"\n", data_file );
        for (unsigned int k=0; k<kd; ++k ) {
          if ( ! proto.NextData( b ) ) break;
          unsigned int id = DATA_2_DISTANCE( b );
          unsigned int ib = DATA_2_COMPASS( b );
          unsigned int ic = DATA_2_CLINO( b );
          unsigned int ir = DATA_2_ROLL( b );
          // printf("%2d Data %.2f %.2f %.2f %.2f\n", k,
          //   DISTANCE_METERS( id ), COMPASS_DEGREES( ib ),
          //   CLINO_DEGREES( ic ), ROLL_DEGREES( ir )  );
          if ( fpd != NULL ) {
            fprintf(fpd, "0x%05x 0x%04x 0x%04x 0x%02x ", id, ib, ic, ir );
            fprintf(fpd, "%.2f %.2f %.2f %.2f \n", 
              DISTANCE_METERS( id ),
              COMPASS_DEGREES( ib ),
              CLINO_DEGREES( ic ),
              ROLL_DEGREES( ir )
            );
          }
        }
        fclose( fpd );
      } else {
        fprintf(stderr, "Cannot open data file \"%s\"\n", data_file );
      }
    } else {
      fprintf(stderr, "ERROR: null data file\n");
    }
  } 

  if ( kc > 0 ) {
    fprintf(stderr, "Calibration data %d \n", kc );
    if ( calib_file != NULL ) {
      FILE * fpc = fopen( calib_file, "w");
      if ( fpc ) {
        fprintf(stderr, "Writing calibration data to \"%s\"\n", calib_file );
        int nl = 0;
        for (unsigned int k=0; k<kc; ++k ) {
          if ( ! proto.NextCalib( b ) ) break;
          int16_t ix = CALIB_2_X( b );
          int16_t iy = CALIB_2_Y( b );
          int16_t iz = CALIB_2_Z( b );
          // printf("%2d Data 0x%04x 0x%04x 0x%04x \n", k, ix, iy, iz );
          fprintf(fpc, "0x%04x 0x%04x 0x%04x ", ix, iy, iz );
          nl ++;
          // group -1, ignore 0, no error
          if ( (nl % 2) == 0 ) fprintf(fpc,"-1 0\n"); 
        }
        fclose( fpc );
      } else {
        fprintf(stderr, "Cannot open calib file \"%s\"\n", calib_file );
      }
    } else {
      fprintf(stderr, "ERROR: null calib file\n");
    }
  }

  return 0;
}

