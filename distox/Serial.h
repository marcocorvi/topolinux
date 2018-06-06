/** @file Serial.h
 *
 * @author marco corvi
 * @date jan 2009
 *
 * @brief communication over a serial channel (SPP)
 * --------------------------------------------------------
 *  Copyright This sowftare is distributed under GPL-3.0 or later
 *  See the file COPYING.
 */
#ifndef SERIAL_H
#define SERIAL_H

#include <stdio.h>
#include <sys/types.h>

#ifdef WIN32
  #include <windows.h>
  typedef signed int ssize_t;
  #define CLOSE CloseHandle
  #define RW_TYPE DWORD
#else
  #include <termios.h>
  #include <unistd.h>
  #define INVALID_HANDLE_VALUE (-1)
  #define CLOSE close
  #define RW_TYPE ssize_t
#endif

class Serial
{
  private:
    char m_device[128];            //!< serial device
    FILE * log_fp;                 //!< log file pointer
    #ifdef WIN32
      HANDLE m_fd;
      DCB m_termios;
      DCB m_termios_save;
	  COMMTIMEOUTS m_timeouts;
	  COMMTIMEOUTS m_timeouts_save;
    #else
      int m_fd;                      //!< serial port file descriptor
      struct termios m_termios;      //!< port termios
      struct termios m_termios_save; //!< saved termios
    #endif



  public:
    /** cstr
     * @param dev  serial device
     * @param log  whether to do log or not [default: false=no log]
     */
    Serial( const char * dev, bool log = false );

    /** dstr 
     */
    ~Serial();

    /** check if the serial line is open 
     * @return true if the line is open
     */
    bool IsOpen( ) const { return m_fd != INVALID_HANDLE_VALUE; }

    /** open a serial connection in raw mode
     * @return true if successful
     */
    bool Open();

    /** close the connection with the device
     */
    void Close();

    /** close and reopen the connection
     * @return true if successsful
     */
    bool Reconnect();

    /** write to the serial port
     * @param buf buffer with the data to write
     * @param n   size of the buffer, ie, max number of bytes to write
     * @return number of bytes that hav ebeen written
     *         this is less than n on error
     */
    ssize_t Write( const unsigned char * buf, size_t n );

    /** read from the serial port
     * @param buf buffer where to put the read data 
     * @param n   size of the buffer, ie, max number of bytes to read
     * @return number of bytes that have been read
     */
    ssize_t Read( unsigned char * buf, size_t n );

}; // class Serial



#endif // SERIAL_H

