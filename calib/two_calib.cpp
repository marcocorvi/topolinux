/** @file two_calib.cpp
 * 
 * @author marco corvi
 * @date jan 2010
 * 
 * @brief (azimuth,incl.) transform between two calibrations
 * --------------------------------------------------------
 *  Copyright This sowftare is distributed under GPL-3.0 or later
 *  See the file COPYING.
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#include "../distox/Factors.h"

double initial_delta = 0.1;

// ---------------------------------------------------
// linear algebra in 3D, vector and matrices

class Vec
{
  double v[3];

public:
  Vec()
  {
    for (int i=0; i<3; ++i) v[i] = 0.0;
  }

  Vec( double x, double y, double z )
  {
    v[0] = x;
    v[1] = y;
    v[2] = z;
  }

  double & operator[]( int k ) { return v[k]; }

  double operator[]( int k ) const { return v[k]; }

  void normalize()
  {
    double d = length();
    v[0] /= d;
    v[1] /= d;
    v[2] /= d;
  }

  double length() const 
  { 
    return sqrt( v[0]*v[0] + v[1]*v[1] + v[2]*v[2] );
  }

};

class Mat
{
  double m[9];
  
public:
  Mat()
  {
    for (int i=0; i<9; ++i) m[i] = 0.0;
  }

  Mat( double * m0 )
  {
    for (int i=0; i<9; ++i) m[i] = m0[i];
  }

  Mat( double m00, double m01, double m02, 
       double m10, double m11, double m12,
       double m20, double m21, double m22 )
  {
    m[0] = m00;   m[1] = m01;   m[2] = m02;
    m[3] = m10;   m[4] = m11;   m[5] = m12;
    m[6] = m20;   m[7] = m21;   m[8] = m22;
  }

  double & operator[](int k) { return m[k]; }

  double operator[](int k) const { return m[k]; }

  Mat inverse() const
  {
    Mat m0;
    for (int j=0; j<3; j++) {
      int j1 = (j+1)%3;
      int j2 = (j+2)%3;
      for (int i=0; i<3; i++) {
        int i1 = (i+1)%3;
        int i2 = (i+2)%3;
        m0[i*3+j] = m[j1*3+i1]*m[j2*3+i2] - m[j2*3+i1]*m[j1*3+i2];
      }
    }
    double det = m[0*3+0] * m0[0*3+0]
               + m[1*3+0] * m0[0*3+1]
               + m[2*3+0] * m0[0*3+2];
    for (int k=0; k<9; ++k) m0[k] /= det;
    return m0;
  }


};

Mat operator*( const Mat & m1, const Mat & m2 )
{
  Mat m0;
  for (int j=0; j<3; j++) { // j index of the row
    for (int i=0; i<3; ++i) { // i index of the column
      double s = 0.0;
      for (int k=0; k<3; k++) {
        s += m1[j*3+k] * m2[k*3+i];
      }
      m0[j*3+i] = s;
    }
  }
  return m0;
}

Vec operator%( const Vec & v1, const Vec & v2 )
{
  Vec v0( v1[1]*v2[2]-v1[2]*v2[1],
          v1[2]*v2[0]-v1[0]*v2[2],
          v1[0]*v2[1]-v1[1]*v2[0] );
  return v0;
}

/** product W = M * V
 * W[i] = Sum_k M[i*3+k] V[k]
 */
Vec operator*( const Mat & m, const Vec & v )
{
  Vec v0;
  for (int i=0; i<3; ++i) { // i index of the column
    double s = 0.0;
    for (int k=0; k<3; k++) {
      s += m[i*3+k] * v[k];
    }
    v0[i] = s;
  }
  return v0;
}

Vec operator+( const Vec & v1, const Vec & v2 )
{
  Vec v0( v1[0]+v2[0], v1[1]+v2[1], v1[2]+v2[2] );
  return v0;
}

Vec operator-( const Vec & v1, const Vec & v2 )
{
  Vec v0( v1[0]-v2[0], v1[1]-v2[1], v1[2]-v2[2] );
  return v0;
}

double operator*( const Vec & v1, const Vec & v2 ) 
{
  return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];
}

// -----------------------------------------------
// addition and subtraction for clino and azimuth

double ClinoAdd( double t, double d )
{
  t += d;
  if ( t > 90.0 ) t = 90.0;
  if ( t < -90.0 ) t = -90.0;
  return t;
}

double AzimuthAdd( double p, double d )
{
  p += d;
  if ( p < 0.0 ) p += 360.0;
  if ( p >= 360.0 ) p -= 360.0;
  return p;
}

double AzimuthDiff( double p1, double p2 )
{
  double d = ( p1 > p2 )? p1-p2 : p2-p1;
  if ( d > 180.0 ) d = 360.0 - d;
  return d;
}

double ClinoDiff( double t1, double t2 )
{
  return fabs( t1 - t2 );
}

/*
 * sample matrices and coeff vectors
 *
double aG1c[] = { 1.7668, -0.0168, -0.0120, 
                  0.0132, 1.7655, -0.0019, 
                  0.0132, -0.0019, 1.7654 };
double aM1c[] = { 1.1585, -0.0024, -0.0267, 
                  0.0105, 1.1069, 0.0144, 
                  0.0088, -0.0587, 0.9845 }; 
Vec bG1(  0.0043, -0.0699, -0.0274 );
Vec bM1(  -0.0011, 0.0027, 0.0097 );
Mat aG1( aG1c );
Mat aM1( aM1c );

Vec bG2 = bG1;
Vec bM2 = bM1;
Mat aG2 = aG1;
Mat aM2 = aM1;
*/

/** Transformation
 * 
 * The world axes are NEZ (north, east and Z-downward).
 * The disto axes are xyz, x is the laser direction, y is to the right,
 * and z down.
 * The relation between these coordinates is (unit vectors)
 *    x  = ( cc cb, cc sb, -sc )_NEZ
 *    y' = ( -sb,   cb,    0   )_NEZ
 *    z' = ( sc cb, sc sb, cc  )_NEZ
 * where y' and z' are the intermediate axes with no roll.
 *
 *              \ x        E
 *              . \       /  
 *            x"   .\   / , ' y'
 *          N <-------+ '
 *                   .|
 *                  . |
 *               z'.  v Z
 *
 *    x" =  cb N + sb E
 *    y" = -sb N + cb E = y'
 *    z" = Z
 *
 * The roll is positive when the y-z plane is rotated right-handed
 * around the x-axis, ie, 
 *    y =  cr y' + sr z'
 *    z = -sr y' + cr z'
 *
 *    y' <------+  x (x coming out)
 *          , ' |. 
 *      y '     | .
 *              v  .z
 *            z'
 * 
 *    y =  cr y' + sr z'
 *    z = -sr y' + cr z'
 *    y' = ( -sb,   cb,    0   )_NEZ
 *    z' = ( sc cb, sc sb, cc  )_NEZ
 *
 * In conclusion we have
 *    x = ( cc cb,             cc sb,             -sc )_NEZ
 *    y = ( -cr sb + sr sc cb, cr cb + sr sc sb,  sr cc )_NEZ
 *    z = ( sr sb + cr sc cb,  -sr cb + cr sc sb, cr cc )_NEZ
 *
 * The coefficient of these relations are the entries on the transformation
 * matrix from the world coordinates to the Disto coordinates.
 * For example G = (0,0,1) = Z gives
 *    G = (-sc, sr cc, cr cc )_xyz
 * therefore the clino is given by
 *    - sin(c) = Gx / |G|
 * that is
 *    c = acos( Gx / |G| ) - PI/2
 *
 * The unit vectors Gx(MxG) = N and GxM = E = (0,1,0) become in the xyz frame
 *    N = ( cc cb,  -cr sb + sr sc cb,  sr sb + cr sc cb )
 *    E = ( cc sb,  cr cb + sr sc sb,   -sr cb + cr sc sb )
 * Therefore the azimuth is
 *    b = atan2( Ex, Nx )
 *
 * Finally the roll can be obtained either from N or from E,
 *    Ny = -sb cr   + sc cb sr
 *    Nz = sc cb cr + sb sr
 * inverting the system
 *    cr = (sb Ny - sc cb Nz)/det
 *    sr = (-sc cb Ny - sb Nz)/det
 * From these we get r (N.B. the signs are important to find the proper
 * quadrant).
 * Similarly we can use E,
 *    Ey = cb cr    + sc sb sr
 *    Ez = sc sb cr - cb sr
 * thus
 *    cr = (-cb Ey - sc sb Ez)/det
 *    sr = (-sc sb Ey + cb Ez)/det
 *
 */
class Transform
{
  private: 
    double alpha; //!< dip angle of M
    double mG1[3][4]; // bGx aGx[x] aGx[y] aGx[z] first set of coeffs
    double mM1[3][4];
    double mG2[3][4]; // bGx aGx[x] aGx[y] aGx[z] second set of coeffs
    double mM2[3][4];
    Vec bG1, bM1, bG2, bM2;
    Mat aG1, aM1, aG2, aM2;
    Mat aG1inv, aM1inv, aG2inv, aM2inv;

  public:
    /** cstr
     * @param a       magnetic field dip angle
     * @param coeff1  set of the first coefficients
     * @param coeff2  set of the second coefficients
     * @param print   verbose when loading the coeffs
     */
    Transform( double a, const char * coeff1, const char * coeff2, bool print )
      : alpha( a )
    {
      LoadCalib( coeff1, 1, print );
      LoadCalib( coeff2, 2, print );
    }

    ~Transform()
    { }

   
  private:
    void LoadCalib( const char * filename, int k, bool verbose = false );

    /** compute the compass/clino for a pair of sensor vectors
     */
    void Compute( int k, Vec & g0, Vec & m0,
                  double * compass, double * clino, double * roll );

    /** given a direction of the disto (in world frame)
     * with the first TRUE calibration, get the sensor values
     * and with the second USED calibration, get the measured calues 
     * @param k    index of the first TRUE calibration
     * @param c1   clino in first calibration
     * @param b1   azimuth
     * @param r1   roll
     * @param c2   clino in second calibration [output]
     * @param b2   azimuth in the second calibration [output]
     * @param r2   roll in the second calibration [output]
     */
    void Compute( int k, 
                  double c1, double b1, double r1, 
                  double & c2, double & b2, double & r2 );

    double ComputeError( int k,
                         double c1, double b1, double r1,
                         double c2, double b2, double r2 )
    {
      int n = 0;
      double err = 0.0;
      double b2a, c2a, r2a;
      for ( double r1 = 0.0; r1 < 359.0; r1 += 30.0 ) {
        Compute( k, c1, b1, r1, c2a, b2a, r2a );
        err += ClinoDiff(c2a, c2) + AzimuthDiff(b2a, b2);
        ++n;
      }
      return err / n;
    }
      
  public:
    /** map a direction in the second USED calibration to a direction
     * for the first TRUE calibration
     * @param k    index of the first TRUE calibration
     * @param c1   clino in first calibration [output]
     * @param b1   azimuth
     * @param r1   roll
     * @param c2   clino in second USED calibration [input]
     * @param b2   azimuth
     * @param r2   roll
     */
    double Map( int k,
                double & c1, double & b1, double & r1,
                double c2, double b2, double r2 );
};

#define DBMIN 0.00001
#define DCMIN 0.00001
#define DRMIN 0.001


double
Transform::Map( int k,
                double & c1, double & b1, double & r1,
                double c2, double b2, double r2 )
{
  c1 = c2; 
  b1 = b2;
  r1 = r2;
  double error = ComputeError( k, c1, b1, r1, c2, b2, r2 );
  double db = initial_delta;
  double dc = initial_delta;
  double dr = initial_delta;
  int cnt = 0;
  while ( db > DBMIN && dc > DCMIN && dr > DRMIN ) {
    double b1a = b1;
    double c1a = c1;
    double r1a = r1;
    bool update = true;

    b1a = b1 + db;
    double e1 = ComputeError( k, c1a, b1a, r1a, c2, b2, r2 );
    if ( e1 < error ) {
      b1 = b1a;
      error = e1;
      ++cnt;
      update = false;
    } else {
      b1a = b1 - db;
      e1 = ComputeError( k, c1a, b1a, r1a, c2, b2, r2 );
      if ( e1 < error ) {
        b1 = b1a;
        error = e1;
        ++cnt;
        update = false;
      } else if ( db > DBMIN ) {
        // db /= 2.0;
      }
    }
    c1a = c1 + dc;
    e1 = ComputeError( k, c1a, b1a, r1a, c2, b2, r2 );
    if ( e1 < error ) {
      c1 = c1a;
      error = e1;
      ++cnt;
      update = false;
    } else {
      c1a = c1 - dc; 
      e1 = ComputeError( k, c1a, b1a, r1a, c2, b2, r2 );
      if ( e1 < error ) {
        c1 = c1a;
        error = e1;
        ++cnt;
        update = false;
      } else if ( dc > DCMIN ) {
        // dc /= 2.0;
      }
    }
    r1a = r1 + dr;
    e1 = ComputeError( k, c1a, b1a, r1a, c2, b2, r2 );
    if ( e1 < error ) {
      r1 = r1a;
      error = e1;
      ++cnt;
      update = false;
    } else {
      r1a = r1 - dr; 
      e1 = ComputeError( k, c1a, b1a, r1a, c2, b2, r2 );
      if ( e1 < error ) {
        r1 = r1a;
        error = e1;
        ++cnt;
        update = false;
      } else if ( dr > DRMIN ) {
        // dr /= 2.0;
      }
    }
    if ( update ) {
      db /= 2.0;
      dc /= 2.0;
      dr /= 2.0;
    }
  }
  // printf("updates %d\n", cnt );
  return error;
}

void
Transform::LoadCalib( const char * filename, int k, bool verbose )  
{
  FILE * fp = fopen( filename, "r" );
  if ( fp == NULL ) {
    fprintf(stderr, "Cannot open calibration file \"%s\"\n", filename);
    throw;
  }
  unsigned int x0, x1;
  for ( int row = 0; row < 3; ++row ) {
    for (int col=0; col<4; ++col ) {
      if ( fscanf( fp, "%x %x", &x0, &x1 ) != 2 ) {
        fprintf(stderr, "Input error on calibration file \'%s\"\n", filename);
        throw;
      }
      int16_t value = (int16_t)( ( (x1<<8) | x0 ) & 0xffff );
      if ( k == 1 ) {
        if ( col == 0 ) { mG1[row][col] = value / FV; }
        else            { mG1[row][col] = value / FM; }
      } else {
        if ( col == 0 ) { mG2[row][col] = value / FV; }
        else            { mG2[row][col] = value / FM; }
      }
    }
  }
  for ( int row = 0; row < 3; ++row ) {
    for (int col=0; col<4; ++col ) {
      if ( fscanf( fp, "%x %x", &x0, &x1 ) != 2 ) {
        fprintf(stderr, "Input error on calibration file \"%s\"\n", filename);
        throw;
      }
      int16_t value = (int16_t)( ( (x1<<8) | x0 ) & 0xffff );
      if ( k == 1 ) {
        if ( col == 0 ) { mM1[row][col] = value / FV; }
        else            { mM1[row][col] = value / FM; }
      } else {
        if ( col == 0 ) { mM2[row][col] = value / FV; }
        else            { mM2[row][col] = value / FM; }
      }
    }
  }
  fclose( fp );

  if ( verbose ) {
    if ( k == 1 ) {
      fprintf(stdout, "TRUE calibration:\n");
      fprintf(stdout, "bG %8.4f %8.4f %8.4f\n", mG1[0][0], mG1[1][0], mG1[2][0] );
      fprintf(stdout, "aG %8.4f %8.4f %8.4f\n", mG1[0][1], mG1[0][2], mG1[0][3] );
      fprintf(stdout, "   %8.4f %8.4f %8.4f\n", mG1[1][1], mG1[1][2], mG1[1][3] );
      fprintf(stdout, "   %8.4f %8.4f %8.4f\n", mG1[2][1], mG1[2][2], mG1[2][3] );
      fprintf(stdout, "bM %8.4f %8.4f %8.4f\n", mM1[0][0], mM1[1][0], mM1[2][0] );
      fprintf(stdout, "aM %8.4f %8.4f %8.4f\n", mM1[0][1], mM1[0][2], mM1[0][3] );
      fprintf(stdout, "   %8.4f %8.4f %8.4f\n", mM1[1][1], mM1[1][2], mM1[1][3] );
      fprintf(stdout, "   %8.4f %8.4f %8.4f\n", mM1[2][1], mM1[2][2], mM1[2][3] );
    } else {
      fprintf(stdout, "USED calibration:\n");
      fprintf(stdout, "bG %8.4f %8.4f %8.4f\n", mG2[0][0], mG2[1][0], mG2[2][0] );
      fprintf(stdout, "aG %8.4f %8.4f %8.4f\n", mG2[0][1], mG2[0][2], mG2[0][3] );
      fprintf(stdout, "   %8.4f %8.4f %8.4f\n", mG2[1][1], mG2[1][2], mG2[1][3] );
      fprintf(stdout, "   %8.4f %8.4f %8.4f\n", mG2[2][1], mG2[2][2], mG2[2][3] );
      fprintf(stdout, "bM %8.4f %8.4f %8.4f\n", mM2[0][0], mM2[1][0], mM2[2][0] );
      fprintf(stdout, "aM %8.4f %8.4f %8.4f\n", mM2[0][1], mM2[0][2], mM2[0][3] );
      fprintf(stdout, "   %8.4f %8.4f %8.4f\n", mM2[1][1], mM2[1][2], mM2[1][3] );
      fprintf(stdout, "   %8.4f %8.4f %8.4f\n", mM2[2][1], mM2[2][2], mM2[2][3] );
    }
  }
  


  if ( k == 1 ) {
    bG1 = Vec( mG1[0][0], mG1[1][0], mG1[2][0] );
    aG1 = Mat( mG1[0][1], mG1[0][2], mG1[0][3],
               mG1[1][1], mG1[1][2], mG1[1][3],
               mG1[2][1], mG1[2][2], mG1[2][3] );
    bM1 = Vec( mM1[0][0], mM1[1][0], mM1[2][0] );
    aM1 = Mat( mM1[0][1], mM1[0][2], mM1[0][3],
               mM1[1][1], mM1[1][2], mM1[1][3],
               mM1[2][1], mM1[2][2], mM1[2][3] );
    aG1inv = aG1.inverse();
    aM1inv = aM1.inverse();
  } else {
    bG2 = Vec( mG2[0][0], mG2[1][0], mG2[2][0] );
    aG2 = Mat( mG2[0][1], mG2[0][2], mG2[0][3],
               mG2[1][1], mG2[1][2], mG2[1][3],
               mG2[2][1], mG2[2][2], mG2[2][3] );
    bM2 = Vec( mM2[0][0], mM2[1][0], mM2[2][0] );
    aM2 = Mat( mM2[0][1], mM2[0][2], mM2[0][3],
               mM2[1][1], mM2[1][2], mM2[1][3],
               mM2[2][1], mM2[2][2], mM2[2][3] );
    aG2inv = aG2.inverse();
    aM2inv = aM2.inverse();
  }
}


void 
Transform::Compute( int k,
                    Vec & g0, Vec & m0,
                    double * compass, double * clino, double * roll )
{
  Vec g;
  Vec m;
  if ( k == 1 ) {
    g[0] = mG1[0][0] + mG1[0][1] * g0[0] + mG1[0][2] * g0[1] + mG1[0][3] * g0[2];
    g[1] = mG1[1][0] + mG1[1][1] * g0[0] + mG1[1][2] * g0[1] + mG1[1][3] * g0[2];
    g[2] = mG1[2][0] + mG1[2][1] * g0[0] + mG1[2][2] * g0[1] + mG1[2][3] * g0[2];
    m[0] = mM1[0][0] + mM1[0][1] * m0[0] + mM1[0][2] * m0[1] + mM1[0][3] * m0[2];
    m[1] = mM1[1][0] + mM1[1][1] * m0[0] + mM1[1][2] * m0[1] + mM1[1][3] * m0[2];
    m[2] = mM1[2][0] + mM1[2][1] * m0[0] + mM1[2][2] * m0[1] + mM1[2][3] * m0[2];
  } else {
    g[0] = mG2[0][0] + mG2[0][1] * g0[0] + mG2[0][2] * g0[1] + mG2[0][3] * g0[2];
    g[1] = mG2[1][0] + mG2[1][1] * g0[0] + mG2[1][2] * g0[1] + mG2[1][3] * g0[2];
    g[2] = mG2[2][0] + mG2[2][1] * g0[0] + mG2[2][2] * g0[1] + mG2[2][3] * g0[2];
    m[0] = mM2[0][0] + mM2[0][1] * m0[0] + mM2[0][2] * m0[1] + mM2[0][3] * m0[2];
    m[1] = mM2[1][0] + mM2[1][1] * m0[0] + mM2[1][2] * m0[1] + mM2[1][3] * m0[2];
    m[2] = mM2[2][0] + mM2[2][1] * m0[0] + mM2[2][2] * m0[1] + mM2[2][3] * m0[2];
  }

  g.normalize();
  m.normalize();
  Vec e( 1.0, 0.0, 0.0 );
  Vec m1 = g % (m % g);
  Vec e1 = g % (e % g);
  double clino0 = acos( g[0] / g.length() ) - M_PI/2;
  Vec em1 = e1 % m1;
  double s = em1.length() * ( ( em1*g > 0 ) ? -1.0 : 1.0 );
  double c = e1 * m1;
  double compass0 = atan2( s, c ) * RAD2GRAD;
  double roll0 = atan2( e1[1], e1[2] ) * RAD2GRAD;
  if ( roll0 < 0 ) roll0 += 360;
  if ( compass0 < 0 ) compass0 += 360;
  clino0   *= RAD2GRAD;
  *compass = compass0;
  *clino   = clino0;
  *roll    = roll0;
}

void 
Transform::Compute( int k,
                    double c1, double b1, double r1,
                    double & c2, double & b2, double & r2 )
{
  c1 *= M_PI/180.0;
  b1 *= M_PI/180.0;
  r1 *= M_PI/180.0;
  double cc1 = cos(c1);
  double sc1 = sin(c1);
  double cb1 = cos(b1);
  double sb1 = sin(b1);
  double cr1 = cos(r1);
  double sr1 = sin(r1);

  Mat mw2d;
  mw2d[0*3+0] =  cc1 * cb1;
  mw2d[0*3+1] =  sb1 * cc1;
  mw2d[0*3+2] = -sc1;
  mw2d[1*3+0] =  sb1 * cr1 + sc1 * cb1 * sr1;
  mw2d[1*3+1] = -cb1 * cr1 + sc1 * sb1 * sr1;
  mw2d[1*3+2] =  cc1 * sr1;
  mw2d[2*3+0] =  sb1 * sr1 - sc1 * cb1 * cr1;
  mw2d[2*3+1] = -cb1 * sr1 - sc1 * sb1 * cr1;
  mw2d[2*3+2] = -cc1 * cr1;

  Vec gw( 0.0, 0.0, 1.0 );
  Vec mw( sin(alpha), 0.0, cos(alpha) );

  Vec gd = mw2d * gw; // vectors Disto frame
  Vec md = mw2d * mw;

  Vec g1, m1, g2, m2;

  if ( k == 1 ) {
    g1 = aG1inv * ( gd - bG1 );
    m1 = aM1inv * ( md - bM1 );
    g2 = bG2 + aG2 * g1;
    m2 = bM2 + aM2 * m1;
  } else {
    g1 = aG2inv * ( gd - bG2 );
    m1 = aM2inv * ( md - bM2 );
    g2 = bG1 + aG1 * g1;
    m2 = bM1 + aM1 * m1;
  }

/** 
 * Note y',z' are rotated by the roll angle, around the x direction.
 *
 *                  \ x        E.
 *                   \        .  
 *           `  .   c \     . b , '
 *             b  `  . \  . , '  y'
 *   N <----------------+ '
 *                     /|
 *                    /c|
 *                 z'/  |
 *                      v Z
 *
 * The vector G x (M x G) = M1 is in the plane M-G and orthogonal to G
 * therefore it is N.
 * The vector G x (E0 x G) is in the plane x-G and orthogonal to G, it is
 * in the direction of the projection of x in the N-E plane. Therefore it
 * makes an angle b with N. 
 * From its y-z components we get the roll: y-comp. is proportional to sin(r)
 * and z-component is proportional to cos(r).
 */

  g2.normalize();
  m2.normalize();
  Vec e2( 1.0, 0.0, 0.0 );
  Vec m0 = g2 % ( m2 % g2 );
  Vec e0 = g2 % ( e2 % g2 );
  Vec em0 = e0 % m0;
  double clino0 = acos( g2[0] / g2.length() ) - M_PI/2;
  clino0 *= RAD2GRAD;
  double roll0 = atan2( e0[1], e0[2] ) * RAD2GRAD;
  if ( roll0 < 0 ) roll0 += 360;
  double s = em0.length() * ( ( em0*g2 > 0 ) ? -1.0 : 1.0 );
  double c = e0 * m0;
  double compass0 = atan2( s, c ) * RAD2GRAD;
  if ( compass0 < 0 ) compass0 += 360;
  c2 = clino0;
  b2 = compass0;
  r2 = roll0;
}

void Usage( const char * cmd )
{
  static bool done = false;
  if ( ! done ) {
    printf("Usage: %s [options] true_calib used_calib shot_file\n", cmd );
    printf("Options:\n");
    printf("  -a dip    M dip angle\n");
    printf("  -d delta  angles initial delta [default 0.1]\n");
    printf("  -v        verbose (write shot uncertainties on stderr)\n");
    printf("  -h        help\n");
    printf("Arguments:\n");
    printf("  true_calib    proper calibration file\n");
    printf("  used_calib    calibration coeff. used to take the shots\n");
    printf("  shot_file     survey file (TLX format)\n");
    done = true;
  }
}

int main( int argc, char ** argv )
{
  bool verbose = false;
  double alpha = 30.0 * M_PI/180.0; // dip angle
  const char * program = argv[0];

  while ( argc > 1 && argv[1][0] == '-' ) {
    if ( argv[1][1] == 'v' ) {
      verbose = true;
    } else if ( argv[1][1] == 'h' ) {
      Usage( program );
    } else if ( argv[1][1] == 'a' ) {
      argv ++;
      argc --;
      alpha = atof( argv[1] ) * M_PI/180.0;
    } else if ( argv[1][1] == 'd' ) {
      argv ++;
      argc --;
      initial_delta = atof( argv[1] );
    }
    argv ++;
    argc --;
  }

  if ( argc < 4 ) {
    Usage( program );
    return 0;
  }

  Transform trans( alpha, argv[1], argv[2], verbose );

  FILE * fp = fopen( argv[3], "r" );
  if ( fp == NULL ) {
    fprintf(stderr, "Unable to open survey file \"%s\"\n", argv[3] );
    return 0;
  }
  char * line = NULL;
  size_t nline;
  while ( getline( &line, &nline, fp ) >= 0 ) {
    if ( line[0] == '#' ) {
      printf( line );
      free( line );
      line = NULL;
      continue;
    }
    // if ( line[0] == '"' ) {
       char * ch = line;
    //   for (int k=0; k<3; ++k ) {
    //     ch++;
    //     while ( ch && *ch != '"' && *ch != 0 ) ++ch;
    //   }
    //   if ( ch == 0 || *ch == 0 ) {
    //     fprintf(stderr, "ERROR on line <<%s>>\n", line );
    //     break;
    //   }
    //   ++ch;
    //   *ch = 0;
    //   ++ch;
      double b1, c1, r1, d, b2, c2, r2;
      char rem[512];
      sscanf(ch, "%lf %lf %lf %lf %[^\n]s", &d, &b2, &c2, &r2, rem );

      double err = trans.Map( 1, c1, b1, r1, c2, b2, r2 );
      if ( verbose ) {
        // fprintf(stdout, "%.2f %.2f %.2f error %.2f\n", b1, c1, r1, err );
        printf("%s %.4f %.4f %.4f %.2f error %.2f %s\n", line, d, b1, c1, r1, err, rem);
      } else {
        printf("%s %.4f %.4f %.4f %.2f %s\n", line, d, b1, c1, r1, rem);
      }
    // }
    free( line );
    line = NULL;
  }
  fclose( fp );
  return 0;
}
