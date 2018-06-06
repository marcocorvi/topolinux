/** @file Calibration.cpp
 *
 * @author marco corvi
 * @date dec 2008
 * 
 * @brief Beat Heeb calibration algorithm
 *
 * @note after the class CalibAlgorithm.cs by B. Heeb
 *
 * --------------------------------------------------------
 *  Copyright This sowftare is distributed under GPL-3.0 or later
 *  See the file COPYING.
 */
#include <stdio.h>
#include <math.h>
#include <assert.h>

#include "Calibration.h"
#include "Factors.h"

#ifdef WIN32
  int round( double x ) 
  {
    return ( x > 0.0 )? (int)(x+0.5) : (int)(x-0.5);
  }
#endif

// #define DEBUG
#ifdef DEBUG
  #undef DEBUG
#endif

// const unsigned int 
// Calibration::MAX_IT = 20000;

#ifdef USE_GUI
  #include "../experimental/CalibrationGui.h"
#endif

void 
Calibration::Clear()
{
  num = vG.size();
  for ( unsigned int k=0; k<num; ++k) {
    vG[k] = Vector::zero;
    vM[k] = Vector::zero;
    vGroup[k] = NOT_USED;
  }
  num = 0;

  // clear the vector of groups
  vGroups.clear();
  vD.clear();
}

void 
Calibration::GetCoeff( unsigned char coeff[ 48 ] )
{
  PutCoeff( coeff+ 0, bG.X() * FV );
  PutCoeff( coeff+ 2, aG.X().X() * FM );
  PutCoeff( coeff+ 4, aG.X().Y() * FM );
  PutCoeff( coeff+ 6, aG.X().Z() * FM );
  PutCoeff( coeff+ 8, bG.Y() * FV );
  PutCoeff( coeff+10, aG.Y().X() * FM );
  PutCoeff( coeff+12, aG.Y().Y() * FM );
  PutCoeff( coeff+14, aG.Y().Z() * FM );
  PutCoeff( coeff+16, bG.Z() * FV );
  PutCoeff( coeff+18, aG.Z().X() * FM );
  PutCoeff( coeff+20, aG.Z().Y() * FM );
  PutCoeff( coeff+22, aG.Z().Z() * FM );

  PutCoeff( coeff+24, bM.X() * FV );
  PutCoeff( coeff+26, aM.X().X() * FM );
  PutCoeff( coeff+28, aM.X().Y() * FM );
  PutCoeff( coeff+30, aM.X().Z() * FM );
  PutCoeff( coeff+32, bM.Y() * FV );
  PutCoeff( coeff+34, aM.Y().X() * FM );
  PutCoeff( coeff+36, aM.Y().Y() * FM );
  PutCoeff( coeff+38, aM.Y().Z() * FM );
  PutCoeff( coeff+40, bM.Z() * FV );
  PutCoeff( coeff+42, aM.Z().X() * FM );
  PutCoeff( coeff+44, aM.Z().Y() * FM );
  PutCoeff( coeff+46, aM.Z().Z() * FM );
}

void 
Calibration::SetCoeff( const unsigned char coeff[ 48 ] )
{
  double v;
  GetCoeff( coeff+ 0, v ); bG.X() = v / FV;
  GetCoeff( coeff+ 2, v ); aG.X().X() = v / FM;
  GetCoeff( coeff+ 4, v ); aG.X().Y() = v / FM;
  GetCoeff( coeff+ 6, v ); aG.X().Z() = v / FM;
  GetCoeff( coeff+ 8, v ); bG.Y() = v / FV;
  GetCoeff( coeff+10, v ); aG.Y().X() = v / FM;
  GetCoeff( coeff+12, v ); aG.Y().Y() = v / FM;
  GetCoeff( coeff+14, v ); aG.Y().Z() = v / FM;
  GetCoeff( coeff+16, v ); bG.Z() = v / FV;
  GetCoeff( coeff+18, v ); aG.Z().X() = v / FM;
  GetCoeff( coeff+20, v ); aG.Z().Y() = v / FM;
  GetCoeff( coeff+22, v ); aG.Z().Z() = v / FM;

  GetCoeff( coeff+24, v ); bM.X() = v / FV;
  GetCoeff( coeff+26, v ); aM.X().X() = v / FM;
  GetCoeff( coeff+28, v ); aM.X().Y() = v / FM;
  GetCoeff( coeff+30, v ); aM.X().Z() = v / FM;
  GetCoeff( coeff+32, v ); bM.Y() = v / FV;
  GetCoeff( coeff+34, v ); aM.Y().X() = v / FM;
  GetCoeff( coeff+36, v ); aM.Y().Y() = v / FM;
  GetCoeff( coeff+38, v ); aM.Y().Z() = v / FM;
  GetCoeff( coeff+40, v ); bM.Z() = v / FV;
  GetCoeff( coeff+42, v ); aM.Z().X() = v / FM;
  GetCoeff( coeff+44, v ); aM.Z().Y() = v / FM;
  GetCoeff( coeff+46, v ); aM.Z().Z() = v / FM;
}

void 
Calibration::PutCoeff( unsigned char * data, double value )
{
  short ival = (short)( round( value ) );
  #ifdef WIN32
  if ( abs( round( value ) ) >= (1<<15) ) {
    fprintf(stderr, "ERROR coefficient too big: %.2f Value %d \n", value, ival );
  }
  #else
  if ( fabs( round( value ) ) >= (1<<15) ) {
    fprintf(stderr, "ERROR coefficient too big: %.2f Value %d \n", value, ival );
  }
  #endif
  data[0] = (unsigned char)(ival & 0xff);
  data[1] = (unsigned char)((ival>>8) & 0xff);
}

void
Calibration::GetCoeff( const unsigned char * data, double & value )
{
  short ival = (short)( ( ((unsigned short)(data[1])) << 8 ) | (unsigned short)(data[0]) );
  value = (double)(ival);
}

void 
Calibration::AddValues(
                int gx, int gy, int gz, int mx, int my, int mz, 
                unsigned int idx, int group, int ignore,
                double compass, double clino )
{
  if ( vG.size() < idx+1 ) {
    size_t k = vGroup.size();
    vG.resize( idx+DELTA_V );
    vM.resize( idx+DELTA_V );
    vIgnore.resize( idx+DELTA_V );
    vError.resize( idx+DELTA_V );
    vGroup.resize( idx+DELTA_V );
    for ( ; k<vGroup.size(); ++k ) {
      vGroup[ k ] = NOT_USED;
      vIgnore[ k ] = 0;
    }
    vCompass.resize( idx+DELTA_V );
    vClino.resize( idx+DELTA_V );
  }
  vD.push_back( MeasureData( gx, gy, gz, mx, my, mz, idx, group, ignore ) );

  vG[idx] = Vector( gx/FV, gy/FV, gz/FV );
  vM[idx] = Vector( mx/FV, my/FV, mz/FV );
  vIgnore[idx]  = ignore;
  vError[idx]   = -1.0;
  vCompass[idx] = compass;
  vClino[idx] = clino;
  if ( num < idx+1 ) num = idx+1;

  vGroup[idx] = group;
  if ( group >= 0 ) {
    if ( vGroups.size() <= (unsigned int)group ) {
      vGroups.resize( (unsigned int)group+1 );
    }
    vGroups[ group ].Add( idx );
  }
}

void 
Calibration::PrintValues()
{
  for (unsigned int k=0; k<num; ++k) {
    printf("%2d: G %.4f %.4f %.4f M %.4f %.4f %.4f \n",
      k, vG[k].X(), vG[k].Y(), vG[k].Z(), vM[k].X(), vM[k].Y(), vM[k].Z() );
  }
}

void
Calibration::PrintCalibrationFile( const char * name )
{
  FILE * fp = fopen( name, "w" );
  if ( fp == 0 ) {
    fprintf(stderr, "ERROR: Cannot open file \"%s\" for writing \n", name );
    return;
  }

  // write calibration coefficients as byte to dump on disto memory
  unsigned char coeff[48];
  GetCoeff( coeff );
  for (int k=0; k<48; ++k) {
    fprintf(fp, "0x%02x ", coeff[k] );
    if ( ( k % 8 ) == 7 ) fprintf(fp, "\n");
  }

  // write calibration input data in decimal notation
  fprintf(fp, "Calibration input data.\n");
  for (size_t k=0; k<vD.size(); ++k) {
    fprintf(fp, "G: %d %d %d M: %d %d %d Grp: %d %d %.4f\n",
      vD[k].G[0], vD[k].G[1], vD[k].G[2], 
      vD[k].M[0], vD[k].M[1], vD[k].M[2], 
      /* vD[k].idx, */ vD[k].grp, vIgnore[k], vError[k] );
  }

  // write calibration coefficients in decimal notation
  fprintf(fp, "Calibration coeffs.\n");
  fprintf(fp, "bG:  %.4f %.4f %.4f \n", bG.X(), bG.Y(), bG.Z() );
  fprintf(fp, "aGx: %.4f %.4f %.4f \n", aG.X().X(), aG.X().Y(), aG.X().Z() );
  fprintf(fp, "  y: %.4f %.4f %.4f \n", aG.Y().X(), aG.Y().Y(), aG.Y().Z() );
  fprintf(fp, "  z: %.4f %.4f %.4f \n", aG.Z().X(), aG.Z().Y(), aG.Z().Z() );
  fprintf(fp, "bM:  %.4f %.4f %.4f \n", bM.X(), bM.Y(), bM.Z() );
  fprintf(fp, "aMx: %.4f %.4f %.4f \n", aM.X().X(), aM.X().Y(), aM.X().Z() );
  fprintf(fp, "  y: %.4f %.4f %.4f \n", aM.Y().X(), aM.Y().Y(), aM.Y().Z() );
  fprintf(fp, "  z: %.4f %.4f %.4f \n", aM.Z().X(), aM.Z().Y(), aM.Z().Z() );

  fclose(fp);
}


bool
Calibration::CheckGroups()
{
  for (unsigned int k=0; k<num; ++k) {
    int group = vGroup[k];
    if ( group >= 0 ) {
      bool ok = false;
      for (size_t j=0; j<vGroups[group].Size(); ++j ) {
        if ( k == vGroups[group].Index(j) ) {
          ok = true;
          break;
        }
      }
      if ( ! ok ) return false;
    } else {
      for (size_t i = 0; i<vGroups.size(); ++i ) {
        for (size_t j=0; j<vGroups[i].Size(); ++j ) {
          if ( k == vGroups[i].Index(j) ) {
            return false;
          }
        }
      }
    }
  }
  return true;
}  
  

void 
Calibration::PrintCoeffs()
{
  fprintf(stderr, "bG: %.4f %.4f %.4f \n", bG.X(), bG.Y(), bG.Z() );
  fprintf(stderr, "aG: %.4f %.4f %.4f \n", aG.X().X(), aG.X().Y(), aG.X().Z() );
  fprintf(stderr, "    %.4f %.4f %.4f \n", aG.Y().X(), aG.Y().Y(), aG.Y().Z() );
  fprintf(stderr, "    %.4f %.4f %.4f \n", aG.Z().X(), aG.Z().Y(), aG.Z().Z() );
  fprintf(stderr, "bM: %.4f %.4f %.4f \n", bM.X(), bM.Y(), bM.Z() );
  fprintf(stderr, "aM: %.4f %.4f %.4f \n", aM.X().X(), aM.X().Y(), aM.X().Z() );
  fprintf(stderr, "    %.4f %.4f %.4f \n", aM.Y().X(), aM.Y().Y(), aM.Y().Z() );
  fprintf(stderr, "    %.4f %.4f %.4f \n", aM.Z().X(), aM.Z().Y(), aM.Z().Z() );
}

void
Calibration::PrintGroups()
{
  for (size_t kg=0; kg<vGroups.size(); ++kg ) {
    fprintf(stderr, "Group %2d: ", kg );
    for ( size_t k=0; k<vGroups[kg].Size(); ++k ) {
      fprintf(stderr, " %2d", vGroups[kg].Index(k) );
    }
    fprintf(stderr, "\n");
  }
  fprintf(stderr, "With no group: \n");
  for (size_t k=0; k<num; ++k ) {
    if ( vGroup[k] == NO_GROUP ) {
      fprintf(stderr, " %2d", k );
    }
  }
  fprintf(stderr, "\n");
}

// BEGIN-CUT

/** rotate M by alpha and combine with G, to get the output G
 *  the output M is the output G rotated by -alpha
 * @param gr  G vector
 * @param mr  M vector
 * @param alpha rotation angle
 * @param gx  output G vector
 * @param mx  output M vector
 *
 *         <-------+    (No normal to the page, incoming)
 *         Mr     /|\   (Mr%No vertical, downwards)
 *              /  |  x Gr
 *  rot_a(Mr) x   |     (No%Gx leftwards)
 *                |
 *                v Gx = rot_a(Mr) + Gr
 */
void 
Calibration::OptVectors(
                 const Vector & gr, const Vector & mr, double sa, double ca,
                 Vector & gx, Vector & mx )
{
  Vector no = gr % mr; // normal to the plane (gr,mr)
  no.normalize();
  // double sa = sin( alpha ); 
  // double ca = cos( alpha );
  gx = mr * ca + (mr % no) * sa + gr;
  gx.normalize();
  mx = gx * ca + (no % gx) * sa; // rotate by -alpha
  // Note this step of the algorithm can be done starting with mx or with gx indifferently
  // mx = gr * ca - (gr % no) * sa + mr;
  // mx.normalize();
  // gx = mx * ca - (no % mx) * sa; // rotate by +alpha
};

/** turn a pair of vectors (gxp, mxp) around the X-axis
 * so that they become "aligned" with a pair of reference vectors (gr, mr).
 */ 
double
Calibration::TurnVectors(
                  const Vector & gxp, const Vector & mxp,
                  const Vector & gr,  const Vector & mr,
                  Vector & gx, Vector & mx, bool print )
{
  double s = gr.Z() * gxp.Y() - gr.Y() * gxp.Z() 
           + mr.Z() * mxp.Y() - mr.Y() * mxp.Z();
  double c = gr.Y() * gxp.Y() + gr.Z() * gxp.Z()
           + mr.Y() * mxp.Y() + mr.Z() * mxp.Z();
  double d = sqrt( s*s + c*c );
  s /= d;
  c /= d;
  gx = gxp.turnX( s, c );
  mx = mxp.turnX( s, c );
  #ifdef DEBUG
  if ( print ) {
    printf("TurnVectors s %8.4f c %8.4f\n", s, c );
    printf("  gf %8.4f %8.4f %8.4f\n", gxp.X(), gxp.Y(), gxp.Z() );
    printf("  gt %8.4f %8.4f %8.4f\n", gx.X(), gx.Y(), gx.Z() );
    printf("  mf %8.4f %8.4f %8.4f\n", mxp.X(), mxp.Y(), mxp.Z() );
    printf("  mt %8.4f %8.4f %8.4f\n", mx.X(), mx.Y(), mx.Z() );
  }
  #else
    print = print;
  #endif
  // double alpha = atan2( s, c );
  // return alpha*180.0/M_PI;
  Vector nr = gr % mr; nr.normalize();
  Vector nx = gx % mx; nx.normalize();
  return acos( nr * nx )*180.0/M_PI;
}

unsigned int 
Calibration::Optimize( double & delta, double & error, unsigned int max_it )
{
  unsigned int it;
  double sin_alpha;
  double cos_alpha;
  // fprintf(stderr, "Calibration::Optimize() num %d \n", num );
  if ( num < 16 ) 
    return (unsigned int)(-1);

  assert( CheckGroups() );

  Vector * gr = new Vector[ num ];
  Vector * mr = new Vector[ num ];
  Vector * gx = new Vector[ num ];
  Vector * mx = new Vector[ num ];

  optimize_eps = EPS * delta;
  max_it = 200; // FIXME
  it = OptimizeCore( gr, mr, gx, mx, max_it, &sin_alpha, &cos_alpha );

  int jmax;
  delta = ComputeDelta( /* gx, mx, */ error, jmax, sin_alpha, cos_alpha, false /*true*/ );

  delete[] gr;
  delete[] mr;
  delete[] gx;
  delete[] mx;

  return it;
}



// (gr,mr) are work vectors where the transformed (G,M) are written
// (gx,mx) are vector where the optimal transformed (G,M) are written
//
int 
Calibration::OptimizeCore( Vector * gr, Vector * mr, Vector * gx, Vector * mx,
                           unsigned int max_it, double * sin_alpha0, double *cos_alpha0 )
{
  Matrix aG0;
  Matrix aM0;
  
  Vector sumG; //  = Vector::zero;
  Vector sumM;
  Matrix sumG2;
  Matrix sumM2;

  double sa = 0.0;
  double ca = 0.0;
  int sum0 = 0;
  for (unsigned int k=0; k<num; ++k) {
    if ( vIgnore[k] == 1 ) continue;
    if ( vGroup[k] == NOT_USED ) continue;
    sa += (vG[k] % vM[k]).length(); // cross product (abs value)
    ca += vG[k] * vM[k];            // dot product
    sumG += vG[k];
    sumM += vM[k];
    sumG2 += vG[k] & vG[k];
    sumM2 += vM[k] & vM[k];
    sum0 ++;
  }
  double invNum = 1.0 / sum0;
  double da = sqrt( sa*sa + ca*ca );
  double sin_alpha = sa/da;
  double cos_alpha = ca/da;
  #ifdef DEBUG
    double alpha = atan2( sa, ca );
    printf("Alpha %.4f s %.4f c %.4f\n", alpha*180.0/M_PI, sa, ca );
  #endif

  Vector avG = sumG * invNum;
  Vector avM = sumM * invNum;

  Matrix iG = (sumG2 - (sumG & avG));
  Matrix iM = (sumM2 - (sumM & avM));
  Matrix invG = iG.inverse();
  Matrix invM = iM.inverse();

  #ifdef DEBUG
    printf("AvG %.4f %.4f %.4f \n", avG.X(), avG.Y(), avG.Z() );
    printf("iG: %.4f %.4f %.4f \n", iG.X().X(), iG.X().Y(), iG.X().Z() );
    printf("    %.4f %.4f %.4f \n", iG.Y().X(), iG.Y().Y(), iG.Y().Z() );
    printf("    %.4f %.4f %.4f \n", iG.Z().X(), iG.Z().Y(), iG.Z().Z() );
  #endif

  #if 0 // def DEBUG
  {
    Matrix cG = iG * invG;
    if ( cG.max_diff( Matrix::one ) > EPS ) {
      fprintf(stderr, "WARNING approximate inverse G matrix: diff %f\n", cG.max_diff( Matrix::one ) );
    } 
    Matrix cM = iM * invM;
    if ( cM.max_diff( Matrix::one ) > EPS ) {
      fprintf(stderr, "WARNING approximate inverse M matrix: diff %f\n", cM.max_diff( Matrix::one ) );
    }
  }
  #endif

  // aG = aM = Id-matrix
  // bG = bM = 0-vector
  PrepareOptimize();

  #ifdef USE_GUI
    CalibrationGui * gui = NULL;
    if ( show_gui ) gui = new CalibrationGui( this, num );
  #endif

  unsigned int it = 0; // iteration number
  double mdG, mdM;
  do {
    // transform the input values by the calibration coeffs
    for (unsigned int k=0; k<num; ++k) {
      if ( vIgnore[k] == 1 ) continue;
      if ( vGroup[k] == NOT_USED ) continue;
      gr[k] = bG + aG * vG[k];
      mr[k] = bM + aM * vM[k];
      #ifdef DEBUG
        if ( k < 4 ) {
          printf("GR[%d] %.5f %.5f %.5f\n", k, gr[k].X(), gr[k].Y(), gr[k].Z() );
          printf("MR[%d] %.5f %.5f %.5f\n", k, mr[k].X(), mr[k].Y(), mr[k].Z() );
        }
      #endif
    }
    #ifdef USE_GUI
      if ( gui ) {
        // gui->DisplayGM( gr, mr );
        gui->DisplayCompassClino( gr, mr );
      }
    #endif

    sa = ca = 0.0;
    for ( size_t kg = 0; kg<vGroups.size(); ++kg ) {
      if ( vGroups[kg].Size() == 0 ) continue;
      Vector grp; // average: zero initialized
      Vector mrp;
      // int cnt = 0;
      double ca_max = 0.0;
      unsigned int first = (unsigned int)(-1);
      int grp_nr = 0; // items in the group
      for (unsigned int k1=0; k1<vGroups[kg].Size(); ++k1) {
        unsigned int k = vGroups[kg].Index(k1);
        if ( vIgnore[k] != 1 ) {
          ++ grp_nr;
          if ( first == (unsigned int)(-1) ) {
            first = k;
          }
        }
      }
      if ( grp_nr > 1 ) {
        for (unsigned int k1=0; k1<vGroups[kg].Size(); ++k1) {
          unsigned int k = vGroups[kg].Index(k1);
          if ( vIgnore[k] == 1 ) continue;
          Vector gt;
          Vector mt;
          double ca = 0.0;
	  ca = TurnVectors( gr[k], mr[k], gr[first], mr[first], gt, mt, (k<4) );
          if ( ca > ca_max ) ca_max = ca;
          #ifdef DEBUG
             if ( k < 4 ) {
               printf("GR[%d] %.5f %.5f %.5f\n", k, gr[k].X(), gr[k].Y(), gr[k].Z() );
               printf("MR[%d] %.5f %.5f %.5f\n", k, mr[k].X(), mr[k].Y(), mr[k].Z() );
               printf("GT[%d] %.5f %.5f %.5f\n", k, gt.X(), gt.Y(), gt.Z() );
               printf("MT[%d] %.5f %.5f %.5f\n", k, mt.X(), mt.Y(), mt.Z() );
             }
          #endif
          grp += gt;
          mrp += mt;
        }
        #ifdef DEBUG
          if ( kg == 1 ) {
            printf("GRP %.5f %.5f %.5f\n",  grp.X(), grp.Y(), grp.Z() );
            printf("MRP %.5f %.5f %.5f\n",  mrp.X(), mrp.Y(), mrp.Z() );
          }
        #endif
        Vector gxp; // optimal mean vector pair (gx,mx)
        Vector mxp;
        OptVectors( grp, mrp, sin_alpha, cos_alpha, gxp, mxp );
        #ifdef DEBUG
          if ( kg == 1 ) {
            printf("GXP %.5f %.5f %.5f\n",  gxp.X(), gxp.Y(), gxp.Z() );
            printf("MXP %.5f %.5f %.5f\n",  mxp.X(), mxp.Y(), mxp.Z() );
          }
        #endif
        // new alpha calculation
        double s = (mrp % gxp).length(); // original
        double c = mrp * gxp;
        sa += s;
        ca += c;
        // printf("group %2d items %2d max alpha %8.4f \n", kg, grp_nr, ca_max );
        // sa += (mxp % gxp).length();
        // ca += mxp * gxp;

        for (unsigned int k1=0; k1<vGroups[kg].Size(); ++k1) {
          unsigned int k = vGroups[kg].Index(k1);
          if ( vIgnore[k] == 1 ) continue;
          // get optimal gx, mx from matched (gxp, mxp)
          TurnVectors( gxp, mxp, gr[k], mr[k], gx[k], mx[k], (k<4) );
          #ifdef DEBUG
             if ( k < 4 ) {
               printf("GR[%d] %.5f %.5f %.5f\n", k, gr[k].X(), gr[k].Y(), gr[k].Z() );
               printf("MR[%d] %.5f %.5f %.5f\n", k, mr[k].X(), mr[k].Y(), mr[k].Z() );
               printf("GX[%d] %.5f %.5f %.5f\n", k, gx[k].X(), gx[k].Y(), gx[k].Z() );
               printf("MX[%d] %.5f %.5f %.5f\n", k, mx[k].X(), mx[k].Y(), mx[k].Z() );
             }
          #endif
    
        }
      } else if ( grp_nr == 1 ) { // individual sample
        assert( first != (unsigned int)(-1) );
        unsigned int k = first;
        OptVectors( gr[k], mr[k], sin_alpha, cos_alpha, gx[k], mx[k] );
        double s = (mr[k] % gx[k]).length(); // original
        double c = mr[k] * gx[k];
        sa += s;
        ca += c;
        // printf("group %2d items %2d alpha %8.4f \n", kg, grp_nr, atan2(s,c)*180.0/M_PI);
      }
    }
    // assert( k == 16 );
    for (unsigned int k=0; k<num; ++k ) { // additional individual samples
      if ( vGroup[k] == NO_GROUP ) {
        OptVectors( gr[k], mr[k], sin_alpha, cos_alpha, gx[k], mx[k] );
        double s = (mr[k] % gx[k]).length(); // original
        double c = mr[k] * gx[k];
        sa += s;
        ca += c;
        // printf("group -- item  %2d alpha %8.4f \n", k, atan2(s,c)*180.0/M_PI);
        // sa += (mx[k] % gx[k]).length();
        // ca += mx[k] * gx[k];
      }
    }

    da = sqrt( sa*sa + ca*ca );
    sin_alpha = sa/da;
    cos_alpha = ca/da;
    #ifdef DEBUG
      alpha = atan2( sa, ca );
      printf("Alpha %.2f s %.4f c %.4f \n", alpha*180.0/M_PI, sa, ca );
    #endif

    // get aG, aM from g, m, gx, mx
    Vector avGx;
    Vector avMx;
    Matrix sumGxG;
    Matrix sumMxM;
    int s0 = 0;
    for (unsigned int k=0; k<num; ++k) {
      if ( vIgnore[k] == 1 ) continue;
      if ( vGroup[k] == NOT_USED ) continue;
      avGx += gx[k];
      avMx += mx[k];
      sumGxG += (gx[k] & vG[k]);
      sumMxM += (mx[k] & vM[k]);
      s0 ++;
    }
    #ifdef DEBUG
      printf("AvGx %.4f %.4f %.4f \n", avGx.X(), avGx.Y(), avGx.Z() );
      printf("sGxG %.4f %.4f %.4f \n", sumGxG.X().X(), sumGxG.X().Y(), sumGxG.X().Z() );
      printf("     %.4f %.4f %.4f \n", sumGxG.Y().X(), sumGxG.Y().Y(), sumGxG.Y().Z() );
      printf("     %.4f %.4f %.4f \n", sumGxG.Z().X(), sumGxG.Z().Y(), sumGxG.Z().Z() );
      printf("AvMx %.4f %.4f %.4f \n", avMx.X(), avMx.Y(), avMx.Z() );
      printf("sMxM %.4f %.4f %.4f \n", sumMxM.X().X(), sumMxM.X().Y(), sumMxM.X().Z() );
      printf("     %.4f %.4f %.4f \n", sumMxM.Y().X(), sumMxM.Y().Y(), sumMxM.Y().Z() );
      printf("     %.4f %.4f %.4f \n", sumMxM.Z().X(), sumMxM.Z().Y(), sumMxM.Z().Z() );
    #endif

    assert( s0 == sum0 );
    aG0 = aG;
    aM0 = aM;
    avGx *= invNum;
    avMx *= invNum;
    aG = (sumGxG - (avGx & sumG) ) * invG;
    aM = (sumMxM - (avMx & sumM) ) * invM;
    // enforce symmetric aG(y,z)
    aG.Y().Z() = aG.Z().Y() = ( aG.Y().Z() + aG.Z().Y()) * 0.5;

    // new bG, bM
    bG = avGx - ( aG * avG );
    bM = avMx - ( aM * avM );
    #ifdef DEBUG
      printf("bG %.4f %.4f %.4f \n", bG.X(), bG.Y(), bG.Z() );
      printf("aG %.4f %.4f %.4f \n", aG.X().X(), aG.X().Y(), aG.X().Z() );
      printf("   %.4f %.4f %.4f \n", aG.Y().X(), aG.Y().Y(), aG.Y().Z() );
      printf("   %.4f %.4f %.4f \n", aG.Z().X(), aG.Z().Y(), aG.Z().Z() );
      printf("bM %.4f %.4f %.4f \n", bM.X(), bM.Y(), bM.Z() );
      printf("aM %.4f %.4f %.4f \n", aM.X().X(), aM.X().Y(), aM.X().Z() );
      printf("   %.4f %.4f %.4f \n", aM.Y().X(), aM.Y().Y(), aM.Y().Z() );
      printf("   %.4f %.4f %.4f \n", aM.Z().X(), aM.Z().Y(), aM.Z().Z() );
    #endif

    mdG = aG.max_diff( aG0 );
    mdM = aM.max_diff( aM0 );
    #if 0
      int jmax;
      delta = ComputeDelta( gx, mx, error, jmax, sin_alpha, cos_alpha );
      printf("Max diff G %.4e M %.4e Delta %.4f\n", mdG, mdM, delta );
      // printf("Max diff G %.4e M %.4e \n", mdG, mdM );
      // printf("check %.8f \n", CheckInput(false) );
      // scanf("%d", &jmax );
    #endif
    #ifdef DEBUG
       printf("Done iteration %d/%d. Max diff %.4f %.4f\n", it, max_it, mdG, mdM );

    #endif
    ++ it;
  } while ( it < max_it && ( mdG > optimize_eps || mdM > optimize_eps ) );

  *sin_alpha0 = sin_alpha;
  *cos_alpha0 = cos_alpha;

  #ifdef USE_GUI
    if ( gui ) {
      gui->Wait();
      delete gui;
    }
  #endif

  return it;
}

double 
Calibration::ComputeDelta( // Vector * gx, Vector * mx,
                           double & error, int & jmax, 
                           double sin_alpha, double cos_alpha, bool print )
{
  double delta = 0.0;
  double alpha = atan2( sin_alpha, cos_alpha );
  dip_angle = alpha * 180/M_PI;
  error = 0.0;
  int cnt=0;
  int n = 0;
  for (unsigned int k=0; k<num; ++k) {
    if ( vIgnore[k] == 1 ) {
      if ( print ) {
        printf("  <%3d>  ", k+1);
        if ( ++cnt == 8 ) { cnt=0; printf("\n"); }
      }
      continue;
    }
    if ( vGroup[k] == NOT_USED ) continue;
    Vector g = bG + aG * vG[k];
    Vector m = bM + aM * vM[k];
    // Vector dG = gx[k] - g;
    // Vector dM = mx[k] - m;
    double sa = (g % m).length(); // cross product (abs value)
    double ca = g * m;            // dot product
    double a = atan2( sa, ca );
    double err = fabs( a - alpha )*180.0/M_PI; 
    vError[k] = err;
    // double err = dG * dG + dM * dM;
    // vError[k] = sqrt( err );
    if ( print ) {
      printf("%8.4f ", vError[k] );
      if ( ++cnt == 8 ) { cnt=0; printf("\n"); }
    }
    if ( error < err ) { error = err; jmax = k; }
    // delta += err;
    delta += err * err;
    n ++;
  }
  if ( print ) {
    printf("max %.4f at %d (%d/%d)\n", error, jmax, 1+jmax/4, 1+jmax%4 );
  }
  // NOTE the original did not divide by num
  // error = sqrt( error );
  // return sqrt(delta/n) * 100.0; // percent
  return sqrt(delta/n); // degrees
}


void 
Calibration::PrepareOptimize()
{
  // initialize the calibration coeffs
  // fprintf(stderr, "Num inputs %d \n", num );
  aG = Matrix::one;
  aM = Matrix::one;
  bG = Vector::zero;
  bM = Vector::zero;
}


 
double
Calibration::CheckInput( bool print )
{
  double diff = 0.0;
  for (unsigned int k=0; k<num; ++k) {
    Vector g = bG + aG * vG[k];
    Vector m = bM + aM * vM[k];
    Vector e( 1.0, 0.0, 0.0 );
    Vector m0 = g % (m % g);
    Vector e0 = g % (e % g);
    double clino = acos( g.X() / g.length() ) - M_PI/2;
    Vector em0 = e0 % m0;
    double s = em0.length() * ( ( em0*g > 0 ) ? -1.0 : 1.0 );
    double c = e0 * m0;
    double compass = atan2( s, c );
    if ( compass < 0.0 ) compass += 2*M_PI;
    double dc = fabs( vCompass[k] - compass );
    if ( dc > M_PI ) dc = fabs(dc - 2*M_PI );
    double dk = fabs( vClino[k] - clino );
    diff += dc + dk;
    if ( print ) {
      fprintf(stderr, "%2d compass %6.2f %6.2f  %6.4f  clino %6.2f %6.2f  %6.4f\n",
        k,
        vCompass[k]*RAD2GRAD, compass*RAD2GRAD, dc*RAD2GRAD,
        vClino[k]*RAD2GRAD, clino*RAD2GRAD, dk*RAD2GRAD );
    }
  }
  return diff / num;
}

void 
Calibration::Measure(
                    const Vector & g_in, const Vector & m_in,
                    double & compass, double & clino )
{
  Vector g = bG + aG * g_in;
  Vector m = bM + aM * m_in;
  Vector e( 1.0, 0.0, 0.0 );
  Vector m0 = g % (m % g);
  Vector e0 = g % (e % g);
  clino = acos( g.X() / g.length() ) - M_PI/2;
  Vector em0 = e0 % m0;
  double s = em0.length() * ( ( em0*g > 0 ) ? -1.0 : 1.0 );
  double c = e0 * m0;
  compass = atan2( s, c );
  if ( compass < 0.0 ) compass += 2*M_PI;
  compass *= RAD2GRAD;
  clino *= RAD2GRAD;
}

#ifdef EXPERIMENTAL
#include "../experimental/ExperimentalCalibration.cpp"
#endif

