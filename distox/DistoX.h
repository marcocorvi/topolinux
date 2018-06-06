/** @file DistoX.h
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
#ifndef DISTO_X_H
#define DISTO_X_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef WIN32
  #define int16_t  signed short
  #define uint16_t unsigned short
#else
  #include <stdint.h>
#endif

#include "Protocol.h"
#include "Factors.h"

// size of calib coeffs [bytes]
//   linear uses 48, last four 0xff
//   non-linear uses 51, last 0xff 
#define N_COEFF 52

#define MAX_ADDRESS_A3    0x8000
#define MAX_ADDRESS_X310    1064

#define CALIB_BIT_X1 0x08 // DistoX1
#define CALIB_BIT_X2 0x20

#define STATUS_ADDR_X1 0x8000 
#define HEAD_TAIL_X1   0xc020 /* head-tail 0x38 0x20 0xc0 // addr 0xc020 */

// status    0xc006 0xc006 0xc006 0xc034 0xc044 0xc044
#define HEAD_TAIL_X2        0xe008 /* head-tail 0x38 0x08 0xe0 // addr 0xe008 */
#define FIRMWARE_ADDRESS_X2 0xe000 // DistoX2

// specific to DistoX2
#define SEGMENT_2_ADDR_X2( s ) ( ( (s)/56 ) * 1024 + ( (s)%56 ) * 18 )
#define PACKET_2_ADDR_X2( p ) ( SEGMENT_2_ADDR( p/2 ) )
#define PACKET_2_NUMBER_X2( p ) ( (p)%2 )
int INDEX_2_ADDR_X2( int i ) { int a =0; while ( i>=56) { i-=56, a+=0x400; } return a+18*i; }

#define MASK_DIST_UNIT    0x0007 // distance unit mask
#define BIT_ANGLE_UNIT    0x0008 // angle unit
#define BIT_ENDPIECE_REF  0x0010 // endpiece reference
#define BIT_CALIB_MODE    0x0020 // calib-mode on
#define BIT_DISPLAY_LIGHT 0x0040 // display illumination on
#define BIT_BEEP          0x0080 // beep on
#define BIT_TRIPLE_SHOT   0x0100 // triple shot check on (2.4+)
#define BIT_BLUETOOTH     0x0200 // bluetooth on
#define BIT_LOCKED_POWER  0x0400 // locked power off
#define BIT_CALIB_SESSION 0x0800 // new calibration session
#define BIT_ALKALINE      0x1000 // battery = alkaline
#define BIT_SILENT_MODE   0x2000 // silent mode
#define BIT_REVERSE_SHOT  0x4000 // reverse measurement (2.4+)

#define C_2_D( c, k ) ((int16_t)(((uint16_t)(c[k])) | (uint16_t)(c[k+1])<< 8))

/** Calibration coefficients transform from u16_t to real number
 */
#define COEFF2BGX( c ) (  C_2_D( c,  0) / FV )
#define COEFF2AGXX( c ) ( C_2_D( c,  2) / FM )
#define COEFF2AGXY( c ) ( C_2_D( c,  4) / FM )
#define COEFF2AGXZ( c ) ( C_2_D( c,  6) / FM )
#define COEFF2BGY( c )  ( C_2_D( c,  8) / FV )
#define COEFF2AGYX( c ) ( C_2_D( c, 10) / FM )
#define COEFF2AGYY( c ) ( C_2_D( c, 12) / FM )
#define COEFF2AGYZ( c ) ( C_2_D( c, 14) / FM )
#define COEFF2BGZ( c )  ( C_2_D( c, 16) / FV )
#define COEFF2AGZX( c ) ( C_2_D( c, 18) / FM )
#define COEFF2AGZY( c ) ( C_2_D( c, 20) / FM )
#define COEFF2AGZZ( c ) ( C_2_D( c, 22) / FM )

#define COEFF2BMX( c )  ( C_2_D( c, 24) / FV )
#define COEFF2AMXX( c ) ( C_2_D( c, 26) / FM )
#define COEFF2AMXY( c ) ( C_2_D( c, 28) / FM )
#define COEFF2AMXZ( c ) ( C_2_D( c, 30) / FM )
#define COEFF2BMY( c )  ( C_2_D( c, 32) / FV )
#define COEFF2AMYX( c ) ( C_2_D( c, 34) / FM )
#define COEFF2AMYY( c ) ( C_2_D( c, 36) / FM )
#define COEFF2AMYZ( c ) ( C_2_D( c, 38) / FM )
#define COEFF2BMZ( c )  ( C_2_D( c, 40) / FV )
#define COEFF2AMZX( c ) ( C_2_D( c, 42) / FM )
#define COEFF2AMZY( c ) ( C_2_D( c, 44) / FM )
#define COEFF2AMZZ( c ) ( C_2_D( c, 46) / FM )

double COEFF2NL( unsigned char c )
{
  int c0 = 1 + (int)c;
  if ( c0 > 128 ) c0 -= 256;
  return c0 / FN;
}

unsigned char double2NL( double x ) 
{
  x *= FN;
  int v = ( (x>=0)? (int)(x+0.5) : -(int)(-x+0.5 ) ) - 1;
  if ( v <= 0 ) v = 0x100 + v; // FIXME if v == 0 this has no effect
  return (unsigned char)(v & 0xff);
}

/** DistoX1 status bits
 */
#define STATUS_GRAD    0x01
#define STATUS_BT      0x02
#define STATUS_COMPASS 0x04
#define STATUS_CALIB   0x08
#define STATUS_SILENT  0x10

/** DistoX1 status byte tests
 */
#define IS_STATUS_GRAD( s )    ((( (s) & STATUS_GRAD ) == STATUS_GRAD ))
#define IS_STATUS_BT( s )      ((( (s) & STATUS_BT ) == STATUS_BT ))
#define IS_STATUS_COMPASS( s ) ((( (s) & STATUS_COMPASS ) == STATUS_COMPASS ))
#define IS_STATUS_CALIB( s )   ((( (s) & STATUS_CALIB ) == STATUS_CALIB ))
#define IS_STATUS_SILENT( s )  ((( (s) & STATUS_SILENT ) == STATUS_SILENT ))

class DistoXListener
{
  public:
    /** dstr
     * for the virtual table
     */
    virtual ~DistoXListener() {}

    /** reset callback
     */
    virtual void distoxReset() = 0;

    /** data received callback
     * @param nr number od disto data downloaded
     */
    virtual void distoxDownload( size_t nr ) = 0;

    /** "done" callback
     */
    virtual void distoxDone() = 0;
};

enum DistoXModel {
	A3 = 1,
	X310 = 2
};

class DistoX
{
  private:
    DistoXModel mModel;          //!< DistoX model
    Protocol    mProto;          //!< DistoX communication protocol
    DistoXListener * mListener;  //!< listener for notification

  public:
    /** cstr
     * @param device   DistoX device
     * @param log      whether to do log or not [default: false= no log]
     */
    DistoX( DistoXModel model, const char * device, bool log = false )
      : mModel( model )
      , mProto( device, log )
      , mListener( NULL )
    { }

    /** set the listener
     * @param listener  distoX listener
     */
    void setListener( DistoXListener * listener )
    {
      mListener = listener;
    }

    /** download the data
     * @param number   number of data to download [0: infinity]
     */
    bool download( int number = 0 )
    {
      // fprintf(stderr, "***** DistoX::download(%d)\n", number);
      if ( ! mProto.Open() ) {
        fprintf(stderr, "ERROR: failed to open protocol \n");
        return false;
      }

      ProtoError err = PROTO_OK;
      size_t cnt = 0;
      bool ask = ( number == -1 ); // whether to ask distox the number of data
      // fprintf( stderr, "***** ask number: %s\n", ask? "true" : "false" );
      if ( ask ) {
        number = mProto.ReadDataNumber();
        // fprintf(stderr, "***** number %d\n", number );
      } else {
        if ( number == 0 ) { // infinity
          number = -1;
        }
      }
      for ( int retry=0; ; ++retry) {
        while ( number != 0 && ( err = mProto.ReadData() ) == PROTO_OK ) {
          cnt ++;
          number --;
          if ( mListener ) {
            mListener->distoxDownload( cnt );
          }
          if ( number == 0 ) break;
        }
        if ( err == PROTO_TIMEOUT ) { // read timeout
          // fprintf(stderr, "timeout: retry n. %d\n", retry );
          if ( retry < 0 ) continue;
        }
        if ( err != PROTO_OK ) {
          fprintf(stderr, "ERROR: Read failed: %s\n", ProtoErrorStr(err) );
        }
        if ( ask ) {
          number = mProto.ReadDataNumber();
          // fprintf(stderr, "number %d\n", number );
          if ( number > 0 ) {
            -- retry;
            continue;
          } 
        }
        break;
      }
      if ( mListener ) {
        mListener->distoxDone();
      }

      // close the connection with the device
      mProto.Close();
      return true;
    }


    /** accessor: get the number of calibration data
     * @return the number of calibration packet
     */
    unsigned int calibrationSize() const { return mProto.CalibSize(); }
    
    /** accessor: get the number of measurement data
     * @return the number of measurement packet
     */
    unsigned int measurementSize() const { return mProto.DataSize(); }

    /** get the next measurement data, DistoX1
     * @param ...
     * @return true if successful
     */
    bool nextMeasurementX1( double & dist, double & compass, double & clino, double & roll )
    { 
      unsigned char b[8];
      if ( ! mProto.NextData( b ) ) return false;
      unsigned int id = DATA_2_DISTANCE( b );
      unsigned int ib = DATA_2_COMPASS( b );
      unsigned int ic = DATA_2_CLINO( b );
      unsigned int ir = DATA_2_ROLL_X1( b );
      dist    = DISTANCE_METERS( id ),
      compass = COMPASS_DEGREES( ib ),
      clino   = CLINO_DEGREES( ic );
      roll    = ROLL_DEGREES_X1( ir );
      return true;
    }

    /** get the next measurement data, DistoX2
     * @param ...
     * @return true if successful
     */
    bool nextMeasurementX2( double & dist, double & compass, double & clino, double & roll,
		            unsigned int & acc, unsigned int & mag, double & dip )
    { 
      unsigned char b1[8], b2[8];
      if ( ! mProto.NextData( b1, b2 ) ) return false;
      unsigned int id = DATA_2_DISTANCE( b1 );
      unsigned int ib = DATA_2_COMPASS( b1 );
      unsigned int ic = DATA_2_CLINO( b1 );
      unsigned int ir = DATA_2_ROLL_X2( b1, b2 );
      unsigned int ip = DATA_2_DIP( b2 );
      dist    = DISTANCE_METERS( id ),
      compass = COMPASS_DEGREES( ib ),
      clino   = CLINO_DEGREES( ic );
      roll    = ROLL_DEGREES_X2( ir );
      acc     = DATA_2_ACC( b2 );
      mag     = DATA_2_MAG( b2 );
      dip     = DIP_DEGREES( ip );
      return true;
    }

    /** get the next calibration data
     * @param ...
     * @return true if successful
     */
    bool nextCalibration( int16_t & gx, int16_t & gy, int16_t & gz,
                          int16_t & mx, int16_t & my, int16_t & mz )
    { 
      unsigned char b1[8], b2[8];
      if ( ! mProto.NextCalib( b1, b2 ) ) return false;
      gx = CALIB_2_X( b1 );
      gy = CALIB_2_Y( b1 );
      gz = CALIB_2_Z( b1 );
      mx = CALIB_2_X( b2 );
      my = CALIB_2_Y( b2 );
      mz = CALIB_2_Z( b2 );
      return true;
    }

    /** read DistoX user mode
     * @return neg. if failed, otherwise the mode
     *
     */
    int readMode( )
    {
      int ret = -1;
      if ( ! mProto.Open() ) return ret;
      unsigned char mode = 0x00;
      for (int k = 0; k<3; ++k ) {
        if ( mProto.Read8000( &mode ) ) {
          ret = mode;
        }
      }
      mProto.Close();
      return ret;
    }

    /** toggle Disto X calibration mode
     * @return 0 normal mode, 1 calib mode, -1 error
     */
    int toggleCalib()
    {
      int ret = -1;
      if ( ! mProto.Open() ) return ret;
      unsigned char mode = 0x00;
      for (int k = 0; k<3; ++k ) {
        if ( mProto.Read8000( &mode ) ) {
          break;
        }
      }
      if ( mode != 0x00 ) {
        unsigned char mode1 = 0x00;
        unsigned char mode2 = mode;
        if ( mode2 & STATUS_CALIB ) {
          mode2 &= 0xf7;
        } else {
          mode2 |= STATUS_CALIB;
        }
        for (int k = 0; k<3; ++k ) {
          if ( mode & STATUS_CALIB ) { // calib on: switch off
            mProto.SendCommandByte( 0x30 );
          } else {
            mProto.SendCommandByte( 0x31 );
          }
          if ( mProto.Read8000( &mode1 ) && mode1 != mode ) {
            break;
          }
        }
        if ( mode1 == mode2 ) {
          if ( mode & STATUS_CALIB ) {
            ret = 0;
            // fprintf(stdout, "DistoX in normal mode\n");
          } else {
            ret = 1;
            // fprintf(stdout, "DistoX in calibration mode\n");
          }
        } else {
          ret = -1;
          // fprintf(stdout, "Failed to switch DistoX mode\n");
        }
      }
      mProto.Close();
      return ret;
    }

    /** toggle Disto X calib mode
     * @param on   whether to turn calib mode on or off
     * @return 0 normal mode, 1 calib mode, -1 error
     */
    int setCalib( bool on )
    {
      int ret = -1;
      if ( ! mProto.Open() ) return ret;
      unsigned char mode = 0x00;
      // for (int k = 0; k<3; ++k ) 
      {
        if ( mProto.Read8000( &mode ) ) {
          bool calib = ( mode & STATUS_CALIB ) != 0;
          if ( calib != on) {
            unsigned char mode1 = 0x00;
            unsigned char mode2 = mode ^ STATUS_CALIB; // expected mode: toggle calib bit
            for (int k = 0; k<3; ++k ) {
              mProto.SendCommandByte( on ? 0x31 : 0x30 ); // start|stop calib
              if ( mProto.Read8000( &mode1 ) && mode1 == mode2 ) {
                ret = ( ( mode1 & STATUS_CALIB ) != 0 )? 1 : 0;
                break;
              }
            }
          } else {
            ret = calib ? 1 : 0;
          }
        }
      }
      mProto.Close();
      return ret;
    }

    /** toggle Disto X silent mode
     * @param on   whether to turn silent mode on or off
     * @return 0 normal mode, 1 silent mode, -1 error
     */
    int setSilent( bool on )
    {
      int ret = -1;
      if ( ! mProto.Open() ) return ret;
      unsigned char mode = 0x00;
      // for (int k = 0; k<3; ++k )
      {
        if ( mProto.Read8000( &mode ) ) {
          // fprintf(stderr, "mode %02x \n", mode );
          bool silent = ( mode & STATUS_SILENT ) == 0; 
          if ( silent != on) {
            unsigned char mode1 = 0x00;
            unsigned char mode2 = mode ^ STATUS_SILENT;
            for (int k = 0; k<3; ++k ) {
              mProto.SendCommandByte( on ? 0x33 : 0x32 ); // start|stop silent
              if ( mProto.Read8000( &mode1 ) && mode1 == mode2 ) {
                ret = ( ( mode1 & STATUS_SILENT ) != 0 )? 1 : 0;
                break;
              }
            }
          } else {
            ret = silent ? 1 : 0;
          }
        }
      }
      mProto.Close();
      return ret;
    }

    /** toggle Disto X grad mode [not supported]
     * @param on   whether to turn silent mode on or off
     * @return 0 normal mode, 1 silent mode, -1 error
     */
    int setGrad( bool on )
    {
      on = on; // silence gcc complaint
      return -1;
    }

    /** toggle Disto X compass/clino mode [not supported]
     * @param on   whether to turn silent mode on or off
     * @return 0 normal mode, 1 silent mode, -1 error
     */
    int setCompass( bool on )
    {
      on = on; // silence gcc complaint
      return -1;
    }


    /** read the calibration coeffs
     * @param byte eight-byte array to write the coeffs
     *
     * @return true if memory has been changed, false otherwise
     */
    bool readCoeffs( unsigned char * byte )
    {
      if ( ! mProto.Open() ) return false;
      bool ret = ( mProto.ReadCalibration( byte ) == PROTO_OK );
      mProto.Close();
      return ret;
    }


    /** write the calibration coeffs
     * @param byte eight-byte array to write the coeffs
     *
     * @return true if memory has been changed, false otherwise
     */
    bool writeCoeffs( unsigned char * byte )
    {
      if ( ! mProto.Open() ) return false;
      bool ret = ( mProto.WriteCalibration( byte ) == PROTO_OK );
      mProto.Close();
      return ret;
    }

    /** print the calibration coeffs
     * @param byte coeff array
     */
    void printCoeffs( unsigned char * buf )
    {
      fprintf(stdout, "Calibration coefficients.\n");
      fprintf(stdout, "BG:  %7.4f %7.4f %7.4f\n", 
        COEFF2BGX( buf ), COEFF2BGY( buf ), COEFF2BGZ( buf ) ); 
      fprintf(stdout, "AGx: %7.4f %7.4f %7.4f\n", 
        COEFF2AGXX( buf ), COEFF2AGXY( buf ), COEFF2AGXZ( buf ) ); 
      fprintf(stdout, "AGy: %7.4f %7.4f %7.4f\n", 
        COEFF2AGYX( buf ), COEFF2AGYY( buf ), COEFF2AGYZ( buf ) ); 
      fprintf(stdout, "AGz: %7.4f %7.4f %7.4f\n", 
        COEFF2AGZX( buf ), COEFF2AGZY( buf ), COEFF2AGZZ( buf ) ); 
      fprintf(stdout, "BM:  %7.4f %7.4f %7.4f\n", 
        COEFF2BMX( buf ), COEFF2BMY( buf ), COEFF2BMZ( buf ) ); 
      fprintf(stdout, "AMx: %7.4f %7.4f %7.4f\n", 
        COEFF2AMXX( buf ), COEFF2AMXY( buf ), COEFF2AMXZ( buf ) ); 
      fprintf(stdout, "AMy: %7.4f %7.4f %7.4f\n", 
        COEFF2AMYX( buf ), COEFF2AMYY( buf ), COEFF2AMYZ( buf ) ); 
      fprintf(stdout, "AMz: %7.4f %7.4f %7.4f\n", 
        COEFF2AMZX( buf ), COEFF2AMZY( buf ), COEFF2AMZZ( buf ) ); 
    }

};

#endif
 
