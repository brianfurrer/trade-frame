/************************************************************************
 * Copyright(c) 2010, One Unified. All rights reserved.                 *
 * email: info@oneunified.net                                           *
 *                                                                      *
 * This file is provided as is WITHOUT ANY WARRANTY                     *
 *  without even the implied warranty of                                *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                *
 *                                                                      *
 * This software may not be used nor distributed without proper license *
 * agreement.                                                           *
 *                                                                      *
 * See the file LICENSE.txt for redistribution information.             *
 ************************************************************************/

#include "stdafx.h"

#include "math.h"

#include "RunningStats.h"

namespace ou { // One Unified
namespace tf { // TradeFrame

RunningStats::RunningStats(void) : 
  /*b2( 0 ),*/ b1( 0 ), b0( 0 ), 
  SumXX( 0 ), SumX( 0 ), SumXY( 0 ), SumY( 0 ), SumYY( 0 ),
  rr( 0 ), r( 0 ), meanY( 0 ), sd( 0 ),
  nX( 0 ), m_BBMultiplier( 2.0 )
{
}

RunningStats::RunningStats( double BBMultiplier ) : 
  /*b2( 0 ),*/ b1( 0 ), b0( 0 ), 
  SumXX( 0 ), SumX( 0 ), SumXY( 0 ), SumY( 0 ), SumYY( 0 ),
  rr( 0 ), r( 0 ), meanY( 0 ), sd( 0 ),
  nX( 0 ), m_BBMultiplier( BBMultiplier )
{
}

RunningStats::~RunningStats(void) {
}

void RunningStats::Reset( void ) {
  /*b2 = */ b1 = b0 
    = meanY 
    = rr = r 
    = sd /*= bbUpper = bbLower */
    = nX = nY 
    = SumXX = SumX = SumXY = SumY = SumYY = 0;
}

void RunningStats::Add(double x, double y) {
  SumXX += x * x;
  SumX += x;
  SumXY += x * y;
  SumY += y;
  SumYY += y * y;
  nX++;
}

void RunningStats::Remove(double x, double y) {
  SumXX -= x * x;
  SumX -= x;
  SumXY -= x * y;
  SumY -= y;
  SumYY -= y * y;
  nX--;
}

void RunningStats::CalcStats() {

  if ( 0 == nX ) {
    SumXX = SumX = 0;
    SumXY = 0;
    SumY = SumYY = 0;
    r = rr = 0;
    sd = meanY = b1 = b0 = 0;
  }
  else {

    double Sxx, Sxy, Syy;
    double SST, SSR, SSE;

//    double oldb1 = b1;

    Sxx = SumXX - ( SumX * SumX ) / nX;
    Sxy = SumXY - ( SumX * SumY ) / nX;
    Syy = SumYY - ( SumY * SumY ) / nX;

    SST = Syy;
    SSR = ( Sxy * Sxy ) / Sxx;
    SSE = SST - SSR;

    rr = SSR / SST;
    r = Sxy / sqrt(Sxx * Syy);

    sd = sqrt(Syy / nX);

    meanY = SumY / nX;

//    double BBOffset = m_BBMultiplier * sd;
//    bbUpper = meanY + BBOffset;
//    bbLower = meanY - BBOffset;

    b1 = ( nX > 1 ) ? Sxy / Sxx : 0;
    b0 = (1 / nX) * ( SumY - b1 * SumX );
//    b2 = b1 - oldb1;  // *** do this differently
  }
}

} // namespace tf
} // namespace ou
