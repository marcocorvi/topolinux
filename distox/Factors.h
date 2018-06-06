/** @file Factors.h
 * 
 * @author marco corvi
 * @date dec 2008
 * 
 * @brief units factors and constants
 * --------------------------------------------------------
 *  Copyright This sowftare is distributed under GPL-3.0 or later
 *  See the file COPYING.
 */
#ifndef FACTORS_H
#define FACTORS_H

#include <math.h>

#ifdef WIN32
  #define M_PI 3.1415926536
#endif

#define DELTA_V 10   /* vector size increment */
#define FV 24000.0   /* vector scale factor */
#define FM 16384.0   /* matrix scale factor */
#define FN  2796.0
#define ZERO 32768 
#define NEG  65536
#define EPS 1.0e-7
#define RAD2GRAD (180.0/M_PI)
#define GRAD2RAD (M_PI/180.0)

#endif // FACTORS_H

