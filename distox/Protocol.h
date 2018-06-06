/** @file Protocol.h
 *
 * @author marco corvi
 * @date jan 2009
 *
 * @brief serial disto A3X protocol
 * --------------------------------------------------------
 *  Copyright This sowftare is distributed under GPL-3.0 or later
 *  See the file COPYING.
 */
#ifndef PROTOCOL_H
#define PROTOCOL_H

#ifdef _WIN32
  #include "stdint.h"
#else
  #include <stdint.h>
#endif

#include "Serial.h"
#include "BufferQueue.h"

enum Command {
  CALIB_START,
  CALIB_STOP,
  SILENT_START,
  SILENT_STOP
};

enum ProtoError {
  PROTO_OK = 0,
  PROTO_READ,
  PROTO_WRITE,
  PROTO_COMMAND,
  PROTO_ADDR,
  PROTO_PACKET,
  PROTO_CONNECT,
  PROTO_TIMEOUT,
  PROTO_MAX
};

const char * ProtoErrorStr( ProtoError err );

#define DATA_2_DISTANCE( b ) ( ( ( (unsigned int)(b[0] & 0x40) ) << 10 ) | (unsigned int)(b[1]) | ( ( (unsigned int)(b[2]) ) << 8 ) )

#define DATA_2_COMPASS( b ) ( (unsigned int)(b[3]) | ( (unsigned int)(b[4]) ) << 8 )
#define DATA_2_CLINO( b ) ( (unsigned int)(b[5]) | ( (unsigned int)(b[6]) ) << 8 )

// this is specific to DistoX1 : roll is 8-bit
#define DATA_2_ROLL_X1( b ) ( (unsigned int)(b[7]) )

// this is specific to DistoX2 : roll is 16-bit
#define DATA_2_ROLL_X2( b1, b2 ) ( (unsigned int)(b2[7]) | ) ( (unsigned int)(b1[7] << 8 )

#define DATA_2_ACC( b ) ( (unsigned int)(b[1]) | ( (unsigned int)(b[2]) ) << 8 )
#define DATA_2_MAG( b ) ( (unsigned int)(b[3]) | ( (unsigned int)(b[4]) ) << 8 )
#define DATA_2_DIP( b ) ( (unsigned int)(b[5]) | ( (unsigned int)(b[6]) ) << 8 )

#define DISTANCE_METERS( d ) ( (d) / 1000.0)
#define COMPASS_DEGREES( b ) ( ((b) * 180.0) / 0x8000 )
#define CLINO_DEGREES( c ) \
  ( ((c) < 0x8000)? ( (c) * 90.0 ) / 0x4000 \
                  : ( (0x10000 - (c)) * -90.0 ) / 0x4000 )
// this is specific to DistoX1
#define ROLL_DEGREES_X1( r ) ( ((r) * 180.0) / 0x0080 )
// this is specific to DistoX2
#define ROLL_DEGREES_X2( r ) ( ((r) * 180.0) / 0x8000 )
#define DIP_DEGREES( d ) ( ( (d<32768)? (d) : ((d) - 65536) ) * 90.0 / 0x4000 )

#define CALIB_2_X( b ) (int16_t)( (uint16_t)(b[1]) | ( ( (uint16_t)(b[2]) ) << 8 ) )
#define CALIB_2_Y( b ) (int16_t)( (uint16_t)(b[3]) | ( ( (uint16_t)(b[4]) ) << 8 ) )
#define CALIB_2_Z( b ) (int16_t)( (uint16_t)(b[5]) | ( ( (uint16_t)(b[6]) ) << 8 ) )

#define CALIB_2_NLX( b ) ( COEFF2NL( b[48] ) )
#define CALIB_2_NLY( b ) ( COEFF2NL( b[49] ) )
#define CALIB_2_NLZ( b ) ( COEFF2NL( b[50] ) )

#define HEAD( b ) (((uint16_t)(b[0])) | (((uint16_t)(b[1])) << 8 ))
#define TAIL( b ) (((uint16_t)(b[2])) | (((uint16_t)(b[3])) << 8 ))

#define PACKET_TYPE( b ) (( b[0] & 0x3f ))
#define PACKET_DATA   0x01
#define PACKET_G      0x02
#define PACKET_M      0x03
#define PACKET_VECTOR 0x04
#define PACKET_REPLY  0x38

class Protocol
{
  private:
    Serial serial;
    BufferQueue< unsigned char [8] > data_queue;
    BufferQueue< unsigned char [8] > calib_queue;
    BufferQueue< unsigned char > command_queue;
    unsigned char sequence_bit; // DistoX2 sequence bit

  public:
    /** cstr
     * @param dev   serial communication device
     * @param log   whether to do log or not [default: false=no log]
     */
    Protocol( const char * dev, bool log = false );

    /** get the size of the data queue
     * @return the size of the data queue
     */
    unsigned int DataSize() const { return data_queue.Size(); }

    /** get the size of the calib queue
     * @return the size of the calib queue
     */
    unsigned int CalibSize() const { return calib_queue.Size(); }

    /** get the size of the command queue
     * @return the size of the command queue
     */
    unsigned int CommandSize() const { return command_queue.Size(); }

    /** check if the underlying serial line is open
     * @return true if the serial line is open
     */
    bool Open()
    { 
      return serial.Open();
    }

    /** check if the connection is open
     * @return true if the serial line is open
     */
    bool IsOpen() { return serial.IsOpen(); }

    /** close the connection with the device
     */
    void Close() { serial.Close(); }

    /** send a command
     * @param cmd command
     * @return protocol error code
     */
    ProtoError SendCommand( Command cmd );

    bool SendCommandByte( unsigned char byte );

    /** read from memory 4 bytes at a time the eight bytes at address 0x8000
     * @param mode   distox status byte (output)
     * @return true if successful
     */
    bool Read8000X1( unsigned char * mode );

    /** read head/tail of the distox internal data queue
     * @param head   head [output]
     * @param tail   tail [output]
     * @return true if successful
     */
    bool ReadHeadTailX1( uint16_t * head, uint16_t * tail );

    /** get the number of data on the DistoX
     * @return the number of data (-1 on error)
     */
    int ReadDataNumberX1()
    {
      uint16_t head;
      uint16_t tail;
      if ( ! ReadHeadTailX1( &head, &tail ) ) {
        return -1;
      }
      // data queue wraps at 0x8000,
      // 8 bytes per packet
      return (int)( (head + 0x8000 - tail) % 0x8000 )/8;
    }
 
    /** read a data packet
     * @return protocol error code
     */
    ProtoError ReadData( );

    /** write calibration data to the disto
     * @param calib   calibration data (48 of 54 bytes)
     * @param nc      number of calib bytes
     * @return protocol error code
     */
    ProtoError WriteCalibration( unsigned char * calib, size_t nc = 48 );

    /** read the calibration data from the disto
     * @param calib   calibration data (48 or 54 bytes) [output]
     * @param nc      number of calibration bytes
     * @return protocol error code
     */
    ProtoError ReadCalibration( unsigned char * byte, size_t nc = 48 );

    /** get the next data on the queue, DistoX1
     * @param b 8 byte array
     * @return true if there is a data on the queue
     */
    bool NextData( unsigned char (&b)[8] ) 
    { 
      return data_queue.Get( b ) && PACKET_TYPE( b ) == PACKET_DATA;
    }

    /** get the next data on the queue, DistoX2
     * @param b1   first 8 byte array
     * @param b2   second 8 byte array
     * @return true if there is a data on the queue
     */
    bool NextData( unsigned char (&b1)[8], unsigned char (&b2)[8] ) 
    { 
      while ( data_queue.Size() >= 2 ) {
        if ( ! data_queue.Get( b1 ) ) return false;
	if ( PACKET_TYPE( b1 ) == PACKET_DATA ) {
          if ( ! data_queue.Get( b2 ) ) return false;
	  return PACKET_TYPE( b2 ) == PACKET_VECTOR;
	}
      }
      return false;
    }

    /** get the next calib on the queue
     * @param b1  G 8 byte array
     * @param b2  M 8 byte array
     * @return true if there is a calib on the queue
     */
    bool NextCalib( unsigned char (&b1)[8], unsigned char (&b2)[8]  )
    {
      while ( calib_queue.Size() >= 2 ) {
        if ( ! calib_queue.Get( b1 ) ) return false;
	if ( PACKET_TYPE( b1 ) == PACKET_G ) {
          if ( ! calib_queue.Get( b2 ) ) return false;
	  return PACKET_TYPE( b2 ) == PACKET_M;
	}
      }
      return false;
    }

    /** put a command on the command queue
     * @param cmd   command
     */
    void PutCommand( unsigned char cmd ) 
    {
      command_queue.Put( cmd );
    }

    /** get the next command on the command queue
     * @param cmd   command
     */
    ProtoError WriteCommands( )
    {
      unsigned char cmd;
      ProtoError err = PROTO_OK;
      while ( command_queue.Size() > 0 ) {
        command_queue.Get( cmd );
        if ( cmd >= 0x30 && cmd <= 0x33 ) {
          fprintf(stderr, "writing command 0x%02x\n", cmd );
          if ( (err = WriteByte( cmd ) ) != PROTO_OK ) {
            return err;
          }
        } else {
          return PROTO_COMMAND;
        }
      }
      return PROTO_OK;
    }

    /** reconnect the serial line 
     */
    void Reconnect() 
    {
      serial.Reconnect();
    }

  private:
    /** write a byte 
     * @param byte t byte to write
     * @return error code
     */
    ProtoError WriteByte( unsigned char byte )
    {
      if ( serial.Write( &byte, 1 ) != 1) {
        return PROTO_WRITE;
      }
      return PROTO_OK;
    }

    /** acknowledge a data packet
     * @param byte  0-th byte of the data packet
     * @return protocol error code
     */
    ProtoError Acknowledge( unsigned char byte );

};

#endif // PROTOCOL_H
