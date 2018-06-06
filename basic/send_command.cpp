/** @file send_command.cpp
 *
 * @author marco corvi
 * @date jan 2009
 *
 * @brief send a command to the DistoX
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
    "Usage: dump_data [-d device] command\n");
  fprintf(stderr, "Options:\n");
  fprintf(stderr, " -d device      RFCOMM serial device [%s]\n",
    DEFAULT_DEVICE );
  fprintf(stderr, " command        command to send\n");
  fprintf(stderr, "                1: start calib,  2: stop calib\n");
  fprintf(stderr, "                3: start silent, 4: stop silent\n");
}

int main( int argc, char ** argv )
{
  const char * device = DEFAULT_DEVICE;
  int number = -1;
  int ac = 1;
  if ( argc > ac ) {
    if ( strcmp( argv[ac], "-d" ) == 0 && argc > ac+1 ) {
      device = argv[ac+1];
      ac += 2;
    }
  }
  if ( ac < argc ) {
    number = atoi( argv[ac] );
    ac ++;
  }
 
  if ( number < 1 || number > 4 ) {
    fprintf(stderr, "ERROR: command must be between one of:\n");
    fprintf(stderr, "   1 (calib) 2 (normal) 3 (silent) 4 (no-silent)\n");
    return 1;
  }

  Protocol proto( device );
  if ( ! proto.Open() ) {
    fprintf(stderr, "ERROR: failed to open protocol \n");
    return 1;
  }
  
  ProtoError err = PROTO_OK;
  switch( number ) {
    case 1:
      fprintf(stderr, "... SendCommand() CALIB_START\n");
      err = proto.SendCommand( CALIB_START );
      break;
    case 2:
      fprintf(stderr, "... SendCommand() CALIB_STOP\n");
      err = proto.SendCommand( CALIB_STOP );
      break;
    case 3:
      fprintf(stderr, "... SendCommand() SILENT_START\n");
      err = proto.SendCommand( SILENT_START );
      break;
    case 4:
      fprintf(stderr, "... SendCommand() SILENT_STOP\n");
      err = proto.SendCommand( SILENT_STOP );
      break;
    default:
      break;
  }
  if ( err != PROTO_OK ) {
    fprintf(stderr, "Command send failed: %s\n", ProtoErrorStr(err) );
    return 1;
  }

  return 0;
}

