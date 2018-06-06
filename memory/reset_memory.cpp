/** @file reset_memory.cpp
 *
 * @author marco corvi
 * @date jan 2009
 *
 * @brief reset a portion of disto memory
 * --------------------------------------------------------
 *  Copyright This sowftare is distributed under GPL-3.0 or later
 *  See the file COPYING.
 */
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>    // read write close

#include "defaults.h"
#include "Serial.h"

/** DistoX memory block types
 - FREE: 00 ...
         ff ...
 - HOT:  80 ... - fe ...
 - USED: 01 ... - 7f ...

head := 

tail := 

*/

unsigned char reset_block[8] = {
  0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff 
};

unsigned char clear_block[8] = {
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff 
};

unsigned char int_eeprom[16] = {
  0x06, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xb7, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff 
};

unsigned char ext_eeprom[40] = {
  0x01, 0x94, 0x01, 0xeb, 0x92, 0x7b, 0x19, 0xbf,
  0x02, 0xef, 0xfe, 0x72, 0xfe, 0x75, 0x36, 0x00,
  0x03, 0x61, 0x13, 0xde, 0xe2, 0x6a, 0x43, 0x00,
  0x01, 0x93, 0x01, 0xcb, 0x24, 0xc5, 0x00, 0x01,
  0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff 
};


/** write to memory 4 bytes at a time the eight bytes at given address
 * @param serial serial line (communication channel)
 * @param addr address (should be multiple of 4)
 * @param byte eight-byte array to write at (addr,addr+4)
 */
void
write_memory( Serial * serial, unsigned long addr, unsigned char * byte )
{
  unsigned long reply_addr;
  unsigned char buf[8];
  int i, k;
  ssize_t nr;
  addr = addr - (addr % 8);
  for (k=0; k<2; ++k) {
    buf[0] = 0x38;
    buf[1] = (unsigned char)( addr & 0xff );
    buf[2] = (unsigned char)( (addr>>8) & 0xff );
    nr = serial->Write( buf, 3 ); // read data
    nr = serial->Read( buf, 8 );
    if ( nr > 0 ) {
      if ( buf[0] != 0x38 ) {
        fprintf(stderr, 
                "ERROR: read wrong reply packet at addr %04lx cnt %d\n", 
                addr, k);
        break;
      }
      reply_addr = ((unsigned long)(buf[2]))<<8 | buf[1];
      if ( reply_addr != addr ) {
        fprintf(stderr,
                "ERROR: read wrong reply addr %04lx at addr %04lx cnt %d\n",
                reply_addr, addr, k);
        break;
      }
      for (i=3; i<7; ++i) fprintf(stderr, "%02x ", buf[i] );
    } else if ( nr < 0 ) {
      fprintf(stderr, "ERROR: serial read error\n");
      break;
    } else {
      fprintf(stderr, "ERROR: read returns 0 bytes\n");
      break;
    }

    buf[0] = 0x39;
    buf[1] = (unsigned char)( addr & 0xff );
    buf[2] = (unsigned char)( (addr>>8) & 0xff );
    buf[3] = byte[0+k*4];
    buf[4] = byte[1+k*4];
    buf[5] = byte[2+k*4];
    buf[6] = byte[3+k*4];

    nr = serial->Write( buf, 7 ); // write data
    nr = serial->Read( buf, 8 );
    if ( nr > 0 ) {
      if ( buf[0] != 0x38 ) {
        fprintf(stderr, 
                "write() wrong reply packet at addr %04lx cnt %d\n", 
                addr, k);
        break;
      }
      reply_addr = ((unsigned long)(buf[2]))<<8 | buf[1];
      if ( reply_addr != addr ) {
        fprintf(stderr,
                "write() wrong reply addr %04lx at addr %04lx cnt %d\n",
                reply_addr, addr, k);
        break;
      }
      for (i=3; i<7; ++i) fprintf(stderr, "%02x ", buf[i] );
    } else if ( nr < 0 ) {
      perror("write_memory() error **** ");
      break;
    } else {
      fprintf(stderr, "write_memory() read returns 0 bytes\n");
      break;
    }
    addr += 4;
  }
  fprintf(stderr, "\n");
}


/** turn memory hot/used/calib at the given address
 * @param serial serial line (communication channel)
 * @param addr address (should be multiple of 4)
 * @param mode  whether to turn the memory hot or used or calib
 */
void
turn_memory( Serial * serial, unsigned long addr, int mode )
{
  unsigned long reply_addr;
  unsigned char buf[8];
  int i;
  ssize_t nr;
  addr = addr - (addr % 8);
  {
    buf[0] = 0x38;
    buf[1] = (unsigned char)( addr & 0xff );
    buf[2] = (unsigned char)( (addr>>8) & 0xff );
    nr = serial->Write( buf, 3 ); // read data
    nr = serial->Read( buf, 8 );
    if ( nr > 0 ) {
      if ( buf[0] != 0x38 ) {
        fprintf(stderr, 
                "ERROR: read wrong reply packet at addr %04lx \n", 
                addr);
        return;
      }
      reply_addr = ((unsigned long)(buf[2]))<<8 | buf[1];
      if ( reply_addr != addr ) {
        fprintf(stderr,
                "ERROR: read wrong reply addr %04lx at addr %04lx \n",
                reply_addr, addr);
        return;
      }
      fprintf(stderr, "%04lx ", addr );
      if ( buf[3] == 0 || ( buf[3] & 0x80 ) != 0 ) fprintf(stderr, "*** ");
      for (i=3; i<7; ++i) fprintf(stderr, "%02x ", buf[i] );
    } else if ( nr < 0 ) {
      fprintf(stderr, "ERROR: serial read error\n");
      return;
    } else {
      fprintf(stderr, "ERROR: read returns 0 bytes\n");
      return;
    }

    buf[0] = 0x39;
    buf[1] = (unsigned char)( addr & 0xff );
    buf[2] = (unsigned char)( (addr>>8) & 0xff );

    if ( mode == 1 ) { // HOT: turn on bit 7 of the first bye
      buf[3] |= 0x80;
      if ( buf[3] < 0x81 || buf[3] > 0x83 ) buf[3] = 0x81;
    } else if ( mode == 2 ) {     // USED: turn off bit 7 of the first byte
      // buf[3] &= 0x7f;
      // if ( buf[3] == 0 ) buf[3] = 0x01;
      buf[3] = 0x01;
    } else if ( mode == 3 ) {
      buf[3] = 0x02;
    } else if ( mode == 4 ) {
      buf[3] = 0x03;
    }
    // buf[4] ... buf[6] unchanged

    nr = serial->Write( buf, 7 ); // write data
    nr = serial->Read( buf, 8 );
    if ( nr > 0 ) {
      if ( buf[0] != 0x38 ) {
        fprintf(stderr, 
                "write() wrong reply packet at addr %04lx \n", 
                addr);
        return;
      }
      reply_addr = ((unsigned long)(buf[2]))<<8 | buf[1];
      if ( reply_addr != addr ) {
        fprintf(stderr,
                "write() wrong reply addr %04lx at addr %04lx \n",
                reply_addr, addr);
        return;
      }
      fprintf(stderr, " --> ");
      for (i=3; i<7; ++i) fprintf(stderr, "%02x ", buf[i] );
    } else if ( nr < 0 ) {
      perror("turn_memory() error **** ");
      return;
    } else {
      fprintf(stderr, "turn_memory() read returns 0 bytes\n");
      return;
    }
  }
  fprintf(stderr, "\n");
}

void usage()
{
  fprintf(stderr, "Usage: reset_memory [-d device] addr [end_addr] byte\n");
  fprintf(stderr, "where\n");
  fprintf(stderr, "  the device is usually %s\n", DEFAULT_DEVICE );
  fprintf(stderr, "  addr is 0x0000 - 0x8000 for external EEPROM\n");
  fprintf(stderr, "          0x8000 - 0x8100 for internal EEPROM\n");
  fprintf(stderr, "          0xC000 - 0xC100 for RAM\n");
  fprintf(stderr, "  byte is one of: \n");
  fprintf(stderr, "     - clear    ff ff ff ff ff ff ff ff ff\n");
  fprintf(stderr, "     - reset    00 ff ff ff ff ff ff ff ff\n");
  fprintf(stderr, "     - hot      81 ... \n");
  fprintf(stderr, "     - used     01 ... \n");
  fprintf(stderr, "     - int_eeprom  06 00 ff ff ff ff ff ff ff\n");
  fprintf(stderr, "                   b7 00 ff ff ff ff ff ff ff\n");
  fprintf(stderr, "     - ext_eeprom  first five bytes of external eeprom\n");
  fprintf(stderr, " The end_address is used only with byte \"clear\", \"hot\" and \"used\" \n");
}

int
check_addresses( unsigned long addr, unsigned long end_addr )
{
  if (end_addr > 0x8000 ) end_addr = 0x8000;
  if ( end_addr <= addr ) {
    fprintf(stderr,
      "End address 0x%04lx, larger than start address 0x%04lx\n",
      end_addr, addr );
    return -1;
  } 
  if ( addr >= 0x8000 ) {
    fprintf(stderr, "address 0x%04lx is out of ext EEPROM bounds\n", addr );
    return -1;
  }
  return (end_addr - addr)/8;
}
 
 
int main( int argc, char ** argv )
{
  const char * device = DEFAULT_DEVICE;
  unsigned long addr = 0x0;
  unsigned long end_addr = 0x0;
  unsigned char * byte = NULL;
  int cnt = 0;
  int mode = -1; // -1 undefined
                 // 0 clear|reset|eeprom
                 // 1 turn hot
                 // 2 turn used
                 // 3 turn calib

  int ac = 1;
  if ( argc <= ac) {
    usage();
    return 0;
  }

  if ( strcmp(argv[ac], "-d" ) == 0 ) {
    device = argv[++ac];
    ++ac;
  }
  if ( argc <= ac ) {
    usage();
    fprintf(stderr, "missing address specification\n");
    return 0;
  }
  sscanf( argv[ac], "%lx", &addr );
  ++ ac;
  addr = addr - (addr % 8);

  if ( argc <= ac ) {
    usage();
    fprintf(stderr, "missing byte specification\n");
    return 0;
  }
  if (strncmp( argv[ac], "0x", 2) == 0 ) {
    sscanf( argv[ac], "%lx", &end_addr );
    end_addr = end_addr - (end_addr % 8);
    ++ac;
  } else {
    end_addr = addr + 8;
  }

  if (strncmp( argv[ac], "clear", 5) == 0) {
    mode = 0;
    byte = clear_block;
    cnt = check_addresses( addr, end_addr );
    if ( cnt <= 0 ) return 0;

  } else if ( strncmp( argv[ac], "hot", 3) == 0 ) {
    mode = 1;
    cnt = check_addresses( addr, end_addr );
    if ( cnt <= 0 ) return 0;
  } else if ( strncmp( argv[ac], "used", 4) == 0 ) {
    mode = 2;
    cnt = check_addresses( addr, end_addr );
    if ( cnt <= 0 ) return 0;
  } else if ( strncmp( argv[ac], "calib", 5) == 0 ) {
    mode = 3;
    cnt = check_addresses( addr, end_addr ) / 2;
    if ( cnt <= 0 ) return 0;

  } else if ( strncmp( argv[ac], "reset", 5) == 0 ) {
    mode = 0;
    byte = reset_block;
    cnt = 1;
    if ( addr >= 0x8000 ) {
      fprintf(stderr, "address 0x%04lx is out of ext EEPROM bounds\n", addr );
      return 0;
    }

  } else if ( strncmp( argv[ac], "ext_eeprom", 10) == 0 ) {
    mode = 0;
    byte = ext_eeprom;
    cnt = 5;
    if ( addr != 0x0000 ) {
      fprintf(stderr, "wrong address: must be 0x0000 for ext. EEPROM 5\n");
      return 0;
    }

  } else if ( strncmp( argv[ac], "int_eeprom", 10) == 0 ) {
    mode = 0;
    byte = int_eeprom;
    cnt = 2;
    if ( addr != 0x8000 ) {
      fprintf(stderr, "wrong address: must be 0x8000 for int. EEPROM 0\n");
      return 0;
    }
  }

  if ( mode == -1 ) {
    usage();
    fprintf(stderr, "wrong byte specification\n");
    return 0;
  }

  if ( addr >= 0x8100 && ( addr < 0xC000 || addr >= 0xC100 ) ) {
    fprintf(stderr, "address 0x%04lx is out of bounds\n", addr );
    return 0;
  }
  
  fprintf(stderr, "Device %s addr 0x%04lx \n", device, addr );

  Serial serial( device );

  if ( ! serial.Open( ) ) {
    fprintf(stderr, "Failed to open device %s\n", device );
    return 1;
  }

  if ( mode == 0 ) {
    for ( int c = 0; c<cnt; ++c ) {
      write_memory( &serial, addr, byte );
      addr += 8;
      if ( byte != clear_block ) { // eeprom
        byte += 8;
      }
    }
  } else { // mode == 1 (hot) or 2 (used) or 3 (calib)
    for ( int c = 0; c<cnt; ++c ) {
      turn_memory( &serial, addr, mode );
      addr += 8;
      if ( mode == 3 ) {
        turn_memory( &serial, addr, 4 );
        addr += 8;
      }
    }
  } 

  serial.Close();
  return 0;
}

