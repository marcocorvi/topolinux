/** @file conv.c
 *
 * --------------------------------------------------------
 *  Copyright This sowftare is distributed under GPL-3.0 or later
 *  See the file COPYING.
 */
#include <stdio.h>
#include <assert.h>

int main()
{
  FILE * ifp = fopen( "data.txt", "r");
  int gx, gy, gz, mx, my, mz, grp;

  while ( fscanf( ifp, "%d %d %d %d %d %d %d",
    &gx, &mx, &gy, &my, &gz, &mz, &grp ) == 7 ) {
    /*
    if ( gx < 0 ) gx = abs(gx); assert( gx < 24000 && gx >= 0 );
    if ( gy < 0 ) gy = abs(gy); assert( gy < 24000 && gy >= 0 );
    if ( gz < 0 ) gz = abs(gz); assert( gz < 24000 && gz >= 0 );
    if ( mx < 0 ) mx = abs(mx); assert( mx < 24000 && mx >= 0 );
    if ( my < 0 ) my = abs(my); assert( my < 24000 && my >= 0 );
    if ( mz < 0 ) mz = abs(mz); assert( mz < 24000 && mz >= 0 );
    */
    printf("0x%04x 0x%04x 0x%04x 0x%04x 0x%04x 0x%04x %d\n",
            gx, gy, gz, mx, my, mz, grp );
/*
      (uint16_t)gx,
      (uint16_t)gy,
      (uint16_t)gz,
      (uint16_t)mx,
      (uint16_t)my,
      (uint16_t)mz, grp );
*/
  }
  fclose( ifp );
  return 0;
}
