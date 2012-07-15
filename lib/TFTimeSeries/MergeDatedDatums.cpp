/************************************************************************
 * Copyright(c) 2009, One Unified. All rights reserved.                 *
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

#include "StdAfx.h"

//#include "LibCommon/Log.h"

#include "MergeDatedDatums.h"

namespace ou { // One Unified
namespace tf { // TradeFrame

//
// MergeDatedDatums
//

MergeDatedDatums::MergeDatedDatums(void) 
: m_state( eInit ), m_request( eUnknown )
{
}

MergeDatedDatums::~MergeDatedDatums(void) {
  while ( !m_mhCarriers.Empty() ) {
    MergeCarrierBase *p = m_mhCarriers.RemoveEnd();
    delete p;
  }
}

void MergeDatedDatums::Add( TimeSeries<Quote>* pSeries, MergeDatedDatums::OnDatumHandler function) {
  m_mhCarriers.Append( new CMergeCarrier<Quote>( pSeries, function ) );
}

void MergeDatedDatums::Add( TimeSeries<Trade>* pSeries, MergeDatedDatums::OnDatumHandler function) {
  m_mhCarriers.Append( new CMergeCarrier<Trade>( pSeries, function ) );
}

void MergeDatedDatums::Add( TimeSeries<Bar>* pSeries, MergeDatedDatums::OnDatumHandler function) {
  m_mhCarriers.Append( new CMergeCarrier<Bar>( pSeries, function ) );
}

void MergeDatedDatums::Add( TimeSeries<Greek>* pSeries, MergeDatedDatums::OnDatumHandler function) {
  m_mhCarriers.Append( new CMergeCarrier<Greek>( pSeries, function ) );
}

void MergeDatedDatums::Add( TimeSeries<MarketDepth>* pSeries, MergeDatedDatums::OnDatumHandler function) {
  m_mhCarriers.Append( new CMergeCarrier<MarketDepth>( pSeries, function ) );
}

// http://www.codeguru.com/forum/archive/index.php/t-344661.html

/*
struct SortByMergeCarrier {
public:
  SortByMergeCarrier( std::vector<MergeCarrierBase *> *v ): m_v( v ) {};
  bool operator() ( size_t lhs, size_t rhs ) { return (*m_v)[lhs]->GetDateTime() < (*m_v)[rhs]->GetDateTime(); };
protected:
  std::vector<MergeCarrierBase *> *m_v;
};
*/

// be aware that this maybe running in alternate thread
// the thread is not created in this class 
// for example, see CSimulationProvider
void MergeDatedDatums::Run() {
  m_request = eRun;
  size_t cntCarriers = m_mhCarriers.Size();
//  LOG << "#carriers: " << cntCarriers;  // need cross thread writing 
  MergeCarrierBase *pCarrier;
  m_cntProcessedDatums = 0;
  m_state = eRunning;
  while ( ( 0 != cntCarriers ) && ( eRun == m_request ) ) {  // once all series have been depleted, end of run
    pCarrier = m_mhCarriers.GetRoot();
    pCarrier->ProcessDatum();  // automatically loads next datum when done
    ++m_cntProcessedDatums;
    if ( NULL == pCarrier->GetDatedDatum() ) {
      // retire the consumed carrier
      m_mhCarriers.ArchiveRoot();
      --cntCarriers;
    }
    else {
      // reorder the carriers
      m_mhCarriers.SiftDown();
    }
  }
  m_state = eStopped;
//  LOG << "Merge stats: " << m_cntProcessedDatums << ", " << m_cntReorders;
}

void MergeDatedDatums::Stop( void ) {
  m_request = eStop;
}

} // namespace tf
} // namespace ou
