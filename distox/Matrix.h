/** @file Matrix.h
 *
 * @author marco corvi
 * @date dec 2008
 * 
 * @brief 3x3 real matrices
 *
 * @note after the class Matrix.cs by B. Heeb
 * --------------------------------------------------------
 *  Copyright This sowftare is distributed under GPL-3.0 or later
 *  See the file COPYING.
 */
#ifndef MATRIX_H
#define MATRIX_H

#include "Vector.h"

  class Matrix
  {
    private:
      Vector x; //!< top row
      Vector y; //!< middle row
      Vector z; //!< bottom row
 
    public:
      static Matrix zero; //!< zero 3x3 matrix
      static Matrix one;  //!< identity matrix

      // accessors
      const Vector & X() const { return x; }
      const Vector & Y() const { return y; }
      const Vector & Z() const { return z; }
      Vector & X() { return x; }
      Vector & Y() { return y; }
      Vector & Z() { return z; }

    public:
      /** cstr
       * @param x0  first row
       * @param y0  second row
       * @param z0  third row
       */
      Matrix( const Vector & x0 = Vector::zero,
              const Vector & y0 = Vector::zero,
              const Vector & z0 = Vector::zero )
        : x( x0 )
        , y( y0 )
        , z( z0 )
      { }
  

      /** matrix addition
       * @param m another matrix
       * @return the matrix sum of this matrix and m
       */
      Matrix operator+ ( const Matrix & m )
      {
        return Matrix( x + m.x, y + m.y, z + m.z );
      }

      /** matrix addition
       * @param m another matrix
       * @return this matrix
       */
      Matrix & operator+= ( const Matrix & m )
      {
        x += m.x;
        y += m.y;
        z += m.z;
        return *this;
      }

      /** matrix sutraction
       * @param m another matrix
       * @return the matrix difference of this matrix and m
       */
      Matrix operator- ( const Matrix & m )
      {
        return Matrix( x - m.x, y - m.y, z - m.z );
      }
     
      /** matrix sutraction
       * @param m another matrix
       * @return this matrix
       */
      Matrix & operator-= ( const Matrix & m )
      {
        x -= m.x;
        y -= m.y;
        z -= m.z;
        return *this;
      }

      /** multiplication by a number
       * @param a number
       * @return the product of this matrix with the number a
       */
      Matrix operator* ( double a )
      {
        return Matrix( x * a, y * a, z * a );
      }

      /** multiplication by a number
       * @param a number
       * @return this matrix 
       */
      Matrix & operator*= ( double a )
      {
        x *= a;
        y *= a;
        z *= a;
        return *this;
      }

      /** product with a vector
       * @param v the vector
       * @return the product of this matrix and the vector v
       */
      Vector operator* ( const Vector & v ) const
      {
        return Vector( x * v, y * v, z * v );
      }

      /** matrix product with the transposed of another matrix
       * @param m another matrix
       * @return the matrix product of this matrix and m transposed
       */
      Matrix operator* ( const Matrix & m ) const
      {
        return Matrix( m * x, m * y, m * z );
      }

      /** matrix product with the transposed of another matrix
       * @param m another matrix
       * @return this matrix
       */
      Matrix & operator*= ( const Matrix & m )
      {
        x = m * x;
        y = m * y;
        z = m * z;
        return *this;
      }

      /** matrix determinant
       * @return the determinant of this matrix
       */
      double determinant( ) const
      {
        return x * ( y % z );
      }

      /** matrix inverse (only for symmetric matrices)
       * @return the inverse of this matrix
       */
      Matrix inverse( ) const
      {
        Matrix adj( y % z, z % x, x % y );
        double det = x * adj.x;
        adj *= 1.0/det;
        return adj;
      }

      /** maximum component difference
       * @param m another matrix
       * @return the maximum component difference of this matrix and m
       */
      double max_diff( const Matrix & m ) const
      {
        double m1 = x.max_diff( m.x );
        double m2 = y.max_diff( m.y );
        double m3 = z.max_diff( m.z );
        return ( m1 > m2 ) ? ( m1 > m3 ) ? m1 : m3
                           : ( m2 > m3 ) ? m2 : m3 ;
      }

  
  };

  /** outer product of two vectors
   * @param v1 first vector
   * @param v2 second vector
   */
  Matrix operator &( const Vector & v1, const Vector & v2 );


  

#endif // MATRIX_H

