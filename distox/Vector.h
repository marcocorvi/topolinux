/** @file Vector.h
 * 
 * @author marco corvi
 * @date dec 2008
 * 
 * @brief 3D real vector
 *
 * @note after the Vector.cs class by B. Heeb
 * --------------------------------------------------------
 *  Copyright This sowftare is distributed under GPL-3.0 or later
 *  See the file COPYING.
 */
#ifndef VECTOR_H
#define VECTOR_H

#include <stdlib.h>
#include <math.h>

  /** a 3D vector is defined by its components in a certain basis
   * ie, three real numbers
   */
  class Vector
  {
    private:
      double x;  //!< X component
      double y;  //!< Y component
      double z;  //!< Z component

    public:
      static Vector zero; //!< zero vector, ie, (0,0,0)

      // accessor
      double X() const { return x; }
      double Y() const { return y; }
      double Z() const { return z; }
      double & X() { return x; }
      double & Y() { return y; }
      double & Z() { return z; }

    public:
      /** cstr
       * @param x0   X component [default 0.0]
       * @param y0   Y component [default 0.0]
       * @param z0   Z component [default 0.0]
       */
      Vector( double x0=0.0, double y0=0.0, double z0=0.0 )
        : x( x0 )
        , y( y0 )
        , z( z0 )
      { }

      /** vector sum
       * @param v added vector
       * @return the vector sum of this vector and v
       */
      Vector operator+ ( const Vector & v ) const
      {
        return Vector( x + v.x, y + v.y, z + v.z );
      }

      /** vector sum of this vector withanother one
       * @param v another vector
       * @return this vector
       */
      Vector & operator+= ( const Vector & v )
      {
        x += v.x;
        y += v.y;
        z += v.z;
        return *this;
      }


      /** vector difference
       * @param v vector to subtract
       * @return the vector difference of this vector and v
       */
      Vector operator- ( const Vector & v ) const
      {
        return Vector( x - v.x, y - v.y, z - v.z );
      }

      /** vector subtraction of this vector with another one
       * @param v another vector
       * @return this vector
       */
      Vector & operator-= ( const Vector & v )
      {
        x -= v.x;
        y -= v.y;
        z -= v.z;
        return *this;
      }

      /** dot product
       * @param v another vector 
       * @return the dot product of this vector and v
       */
      double operator* ( const Vector & v ) const
      {
        return ( x * v.x + y * v.y + z * v.z );
      }

      /** multplication by a number
       * @param a number that multiplies the vector
       * @return the vector multiplication of this vector and the number
       */
      Vector operator* ( double a ) const
      {
        return Vector( x * a, y * a, z * a );
      }

      /** multplication by a number
       * @param a number that multiplies this vector
       * @return this vector
       */
      Vector & operator*= ( double a )
      {
        x *= a;
        y *= a;
        z *= a;
        return *this;
      }

      /** cross product of this vector with another one
       * @param v another vector
       * @return the cross product of this vector and v
       */
      Vector operator% ( const Vector & v ) const
      {
        return Vector( y * v.z - z * v.y,
                       z * v.x - x * v.z,
                       x * v.y - y * v.x );
      }

      /** length of the vector
       * @return the euclidean length of the vector
       */
      double length( ) const
      {
        return sqrt( x*x + y*y + z*z );
      }

      /** rotation around the X axis by an angle
       * @param s sine of the angle
       * @param c cosine of the angle
       * @return this vector rotated by a around the X axis
       */
      Vector turnX( double s, double c ) const
      {
        // double c = cos( a );
        // double s = sin( a );
        return Vector( x, c*y - s*z, c*z + s*y );
      }

      /** rotation around the Y axis by an angle
       * @param s sine of the angle
       * @param c cosine of the angle
       * @return this vector rotated by a around the Y axis
       */
      Vector turnY( double s, double c ) const
      {
        return Vector( c*x + s*z, y, c*z - s*x );
      }

      /** rotation around the Z axis by an angle
       * @param s sine of the angle
       * @param c cosine of the angle
       * @return this vector rotated by a around the Z axis
       */
      Vector turnZ( double s, double c ) const
      {
        return Vector( c*x - s*y, c*y + s*x, z );
      }

      /** normalize this vector
       */
      void normalize()
      {
        double d = this->length();
        x /= d;
        y /= d;
        z /= d;
      }

      /** maximum coponent difference
       * @param v another vector
       * @return the maximum component difference between this vector and v
       */
      double max_diff( const Vector & v ) const
      {
        double m1 = fabs( x - v.x );
        double m2 = fabs( y - v.y );
        double m3 = fabs( z - v.z );
        return ( m1 > m2 ) ? ( m1 > m3 ) ? m1 : m3 
                           : ( m2 > m3 ) ? m2 : m3 ;
      }

      /** external vector
       * @return the external vector
       */
      Vector Ext() const
      {
        return Vector( y - z, z - x, x - y );
      }

  }; // class Vector

    


#endif // VECTOR_H

