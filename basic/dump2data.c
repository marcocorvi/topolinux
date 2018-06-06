/** @file dump2data.c
 *
 * @author marco corvi
 * @date sept 2009
 *
 * @brief convert memory dump to data file(s)
 * --------------------------------------------------------
 *  Copyright This sowftare is distributed under GPL-3.0 or later
 *  See the file COPYING.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

// #include <sys/types.h>

// taken from Protocol.h
#define DATA_2_DISTANCE( b ) \
   ( ( ( (unsigned int)(b[0] & 0x40) ) << 10 ) | \
     (unsigned int)(b[1]) | \
     ( ( (unsigned int)(b[2]) ) << 8 ) )

#define DATA_2_COMPASS( b ) \
   ( (unsigned int)(b[3]) | \
     ( (unsigned int)(b[4]) ) << 8 )

#define DATA_2_CLINO( b ) \
   ( (unsigned int)(b[5]) | \
     ( (unsigned int)(b[6]) ) << 8 )

#define DATA_2_ROLL( b ) \
   ( (unsigned int)(b[7]) )

#define DISTANCE_METERS( d ) ( (d) / 1000.0)
#define COMPASS_DEGREES( b ) ( ((b) * 180.0) / 0x8000 )
#define CLINO_DEGREES( c ) \
  ( ((c) < 0x8000)? ( (c) * 90.0 ) / 0x4000 \
                  : ( (0x10000 - (c)) * -90.0 ) / 0x4000 )
#define ROLL_DEGREES( r ) ( ((r) * 180.0) / 0x80 )


#define CALIB_2_X( b ) \
  (int16_t)( (uint16_t)(b[1]) | ( ( (uint16_t)(b[2]) ) << 8 ) )
#define CALIB_2_Y( b ) \
  (int16_t)( (uint16_t)(b[3]) | ( ( (uint16_t)(b[4]) ) << 8 ) )
#define CALIB_2_Z( b ) \
  (int16_t)( (uint16_t)(b[5]) | ( ( (uint16_t)(b[6]) ) << 8 ) )


void usage()
{
  printf("Usage: tlx_dump2data [options] <memory-dump> \n");
  printf("options: \n");
  printf("  -a            show all data [by default only hot data are shown]\n");
  printf("  -b            show measurements/calibration boundaries [default no]\n");
  printf("  -t            show transactions [default no]\n");
  printf("  -x            show addresses [default no]\n");
  printf("  -e start end  extract from start address to end address \n");
  printf("                addresses should be hex notation (eg, 0x0000)\n");
  printf("  -h            help\n");
  printf(" in case of option -a you can also specify: \n");
  printf("  -s     show byte type [default no]\n");
  printf("  -c     show only calibration data [default no]\n");
  printf("  -m     show only measurement data [default no]\n");
}

#define START 4
#define END   0
#define USED  1
#define FREE  2
#define HOT   3

#define TYPE_NONE    0
#define TYPE_DATA    1
#define TYPE_CALIB   2
#define TYPE_VECTOR  3
#define TYPE_UNKNOWN 4

const char * type_str[] = { "end", "used", "free", "hot", "start" };
const char * data_str[] = { "none", "data", "calib", "vector", "unknown" };

int main( int argc, char ** argv )
{
  int show_hot = 0;
  int show_all = 0;
  int show_trans = 0;
  int only_calib = 0;
  int only_meas  = 0;
  int show_bounds = 0;
  int extract = 0;
  int addresses = 0;

  FILE * fp = NULL;
  char addr[16];
  char b[8][3];  // eight bytes
  unsigned char bb[8];
  int prev_type;
  int curr_type = START;
  int prev_data;
  int data_type = TYPE_NONE;
  char * addr_start = NULL;
  char * addr_end = NULL; 

  if ( argc <= 1 ) {
    usage();
    return 1;
  }
  if ( argc > 2 ) {
    while ( argv[1][0] == '-' ) {
      if ( argv[1][1] == 's' ) {
        show_hot = 1;
      } else if ( argv[1][1] == 'a' ) {
        show_all = 1;
      } else if ( argv[1][1] == 'c' ) {
        only_calib = 1;
      } else if ( argv[1][1] == 'm' ) {
        only_meas = 1;
      } else if ( argv[1][1] == 't' ) {
        show_trans = 1;
      } else if ( argv[1][1] == 'b' ) {
        show_bounds = 1;
      } else if ( argv[1][1] == 'x' ) {
        addresses = 1;
      } else if ( argv[1][1] == 'e' ) {
        extract = 1;
        argv ++; argc --;
        if ( argv[1][0] == '0' && argv[1][1] == 'x' ) {
          addr_start = argv[1]+2;
        } else {
          usage();
          printf("\nInvalid start address %s\n", argv[1] );
          return 0;
        }
        argv ++; argc --;
        if ( argv[1][0] == '0' && argv[1][1] == 'x' ) {
          addr_end = argv[1]+2;
        } else {
          usage();
          printf("\nInvalid end address %s\n", argv[1] );
          return 0;
        }
      } else if ( argv[1][1] == 'h' ) {
        usage();
        return 0;
      }
      argv ++;
      argc --;
    }
  }

  if ( (fp = fopen( argv[1], "r" ) ) == NULL ) { 
    printf("Unable to open memory-dump file \"%s\"\n", argv[1] );
    return 2;
  }

  // if ( extract ) {
  //   printf("Extract from %s to %s\n", addr_start, addr_end );
  // }

  for ( ; ; ) {
    int k;
    char * line = NULL;
    size_t n;
    if ( getline( &line, &n, fp ) < 0 ) break;
    if ( sscanf( line, "%s %s %s %s %s %s %s %s %s", addr,
      b[0], b[1], b[2], b[3], b[4], b[5], b[6], b[7] ) != 9 ) break;
    free( line );
    prev_type = curr_type;
    prev_data = data_type;
    for ( k=0; k<8; ++k ) {
      bb[k] = 0;
      if ( b[k][0] <= '9' && b[k][0] >= '0' ) { bb[k] += 16 * (b[k][0]-'0'); }
      else if ( b[k][0] <= 'f' && b[k][0] >= 'a' ) { bb[k] += 16 * (10 + (b[k][0]-'a')); }
      else if ( b[k][0] <= 'F' && b[k][0] >= 'A' ) { bb[k] += 16 * (10 + (b[k][0]-'A')); }
      else { assert( 0 ); }
      if ( b[k][1] <= '9' && b[k][1] >= '0' ) { bb[k] += (b[k][1]-'0'); }
      else if ( b[k][1] <= 'f' && b[k][1] >= 'a' ) { bb[k] += (10 + (b[k][1]-'a')); }
      else if ( b[k][1] <= 'F' && b[k][1] >= 'A' ) { bb[k] += (10 + (b[k][1]-'A')); }
      else { assert( 0 ); }
    }
    if ( bb[0] == 0x00 ) { 
      curr_type = END;
      data_type = TYPE_NONE;
    } else if ( bb[0] == 0xff ) {
      curr_type = FREE; 
      data_type = TYPE_NONE;
    } else {
      if ( bb[0] > 0x00 && bb[0] < 0x80 ) { 
        curr_type = USED; 
      } else { 
        curr_type = HOT;
      }
     
      if ( b[0][1] == '1' ) {
        data_type = TYPE_DATA;
      } else if ( b[0][1] == '2' ) {
        data_type = TYPE_CALIB;
      } else if ( b[0][1] == '3' ) {
        data_type = TYPE_CALIB;
      } else if ( b[0][1] == '4' ) {
        data_type = TYPE_VECTOR;
      } else {
        data_type = TYPE_UNKNOWN;
      }
    }

    if ( extract ) {
      if (strncmp(addr_start, addr, 4) > 0 ) continue;
      if ( strncmp(addr_end, addr, 4) <= 0 ) break;
      printf("%s %s %s %s %s %s %s %s %s", addr,
        b[0], b[1], b[2], b[3], b[4], b[5], b[6], b[7] );
      if ( data_type == TYPE_DATA && only_meas == 1 ) {
        unsigned int id = DATA_2_DISTANCE( bb );
        unsigned int ib = DATA_2_COMPASS( bb );
        unsigned int ic = DATA_2_CLINO( bb );
        unsigned int ir = DATA_2_ROLL( bb );
        printf(" %6.2f  %6.2f %6.2f %6.2f", 
          DISTANCE_METERS( id ),
          COMPASS_DEGREES( ib ),
          CLINO_DEGREES( ic ),
          ROLL_DEGREES( ir ) 
        );
      } else if ( data_type == TYPE_CALIB && only_calib == 1 ) {
        int16_t ix = CALIB_2_X( bb );
        int16_t iy = CALIB_2_Y( bb );
        int16_t iz = CALIB_2_Z( bb );
        printf( " 0x%04x 0x%04x 0x%04x ", ix, iy, iz );
      }
      printf("\n");
    } else if ( show_bounds ) {
      if ( data_type != prev_data ) {
        printf("%s %s --> %s \n", addr, data_str[prev_data], data_str[data_type] );
      }
    } else if ( show_trans ) {
      if ( prev_type != curr_type ) {
        printf("%s %02x %s --> %s \n", addr, bb[0], type_str[prev_type], type_str[curr_type] );
      }
    } else if ( show_all == 1 || b[0][0] == '8' ) {
      if ( b[0][1] == '1' && (only_calib == 0) ) { 
        unsigned int id = DATA_2_DISTANCE( bb );
        unsigned int ib = DATA_2_COMPASS( bb );
        unsigned int ic = DATA_2_CLINO( bb );
        unsigned int ir = DATA_2_ROLL( bb );
        if ( show_hot ) printf( "%02x ", bb[0] );
        if ( addresses ) printf( "%s ", addr );
        printf( "0x%05x 0x%04x 0x%04x 0x%02x ", id, ib, ic, ir );
        printf( "%.2f %.2f %.2f %.2f\n", 
          DISTANCE_METERS( id ),
          COMPASS_DEGREES( ib ),
          CLINO_DEGREES( ic ),
          ROLL_DEGREES( ir )
        );
      } else if ( b[0][1] == '2' && (only_meas == 0) ) {
        int16_t ix = CALIB_2_X( bb );
        int16_t iy = CALIB_2_Y( bb );
        int16_t iz = CALIB_2_Z( bb );
        if ( show_hot ) printf( "%02x ", bb[0] );
        if ( addresses ) printf( "%s ", addr );
        printf( "0x%04x 0x%04x 0x%04x ", ix, iy, iz );
      } else if ( b[0][1] == '3' && (only_meas == 0) ) {
        int16_t ix = CALIB_2_X( bb );
        int16_t iy = CALIB_2_Y( bb );
        int16_t iz = CALIB_2_Z( bb );
        printf( "0x%04x 0x%04x 0x%04x \n", ix, iy, iz );
      }
    }
  }
  fclose( fp );
  return 0;
}
