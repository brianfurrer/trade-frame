/************************************************************************
 * Copyright(c) 2011, One Unified. All rights reserved.                 *
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

//#include <TFHDF5TimeSeries/HDF5DataManager.h>
//#include <TFHDF5TimeSeries/HDF5TimeSeriesContainer.h>

#include <TFTrading/InstrumentManager.h>

#include "Strategy.h"

Strategy::Strategy(void) 
  : m_sim( new ou::tf::CSimulationProvider() ),
  m_TradeDirection( ETradeDirUnkn ),
  m_sma1( &m_quotes,   60 ), //   1 min
  m_sma2( &m_quotes,  120 ), //   2 min
  m_sma3( &m_quotes,  180 ), //   3 min
  m_sma4( &m_quotes,  300 ), //   5 min
  m_sma5( &m_quotes,  600 ), //  10 min
  m_sma6( &m_quotes, 1800 ), //  30 min
  m_sma7( &m_quotes, 3600 ), //  60 min
  m_sma8( &m_quotes, 7200 ), // 120 min
//  m_stateTrade( ETradeOut ), m_dtEnd( date( 2011, 9, 23 ), time_duration( 17, 58, 0 ) ),
  m_stateTrade( ETradeStart ), m_dtEnd( date( 2011, 11, 9 ), time_duration( 17, 45, 0 ) ),  // put in time start
  m_nUpTransitions( 0 ), m_nDnTransitions( 0 ), 
  m_barFactory( 180 ),
  m_dvChart( "Strategy1", "GC" ),
  m_ceShorts( ou::ChartEntryShape::ESell, ou::Colour::Orange ),
  m_ceLongs( ou::ChartEntryShape::EBuy, ou::Colour::Blue ),
  m_tsswSlopeOfSlopeOfSMA1( &m_pricesSlopeOfSlopeOfSMA1, 90 ), 
  m_tsswSlopeOfSlopeOfSMA2( &m_pricesSlopeOfSlopeOfSMA2, 360 ),
  m_tsswSlopeOfBollinger2Offset( &m_pricesBollinger2Offset, 240 ),
  m_tsswSpreads( &m_spreads, 120 ),
  m_er1( &m_trades, 10 ), m_er2( &m_trades, 30 ), m_er3( &m_trades, 90 )
{

  ou::tf::CProviderManager::Instance().Register( "sim01", static_cast<pProvider_t>( m_sim ) );

  m_dvChart.Add( 0, m_ceShorts );
  m_dvChart.Add( 0, m_ceLongs );
  m_dvChart.Add( 0, m_ceBars );
  m_dvChart.Add( 0, m_ceSMA1 );
  m_dvChart.Add( 0, m_ceSMA2 );
  m_dvChart.Add( 0, m_ceSMA3 );
//  m_dvChart.Add( 0, m_ceUpperBollinger1 );
//  m_dvChart.Add( 0, m_ceLowerBollinger1 );
  m_dvChart.Add( 0, m_ceUpperBollinger2 );
  m_dvChart.Add( 0, m_ceLowerBollinger2 );
  m_dvChart.Add( 0, m_ceUpperBollinger3 );
  m_dvChart.Add( 0, m_ceLowerBollinger3 );
  m_dvChart.Add( 1, m_ceVolume );
  m_dvChart.Add( 2, m_ceSlopeOfSMA1 );
  m_dvChart.Add( 2, m_ceSlopeOfSlopeOfSMA1 );
  m_dvChart.Add( 3, m_ceSlopeOfSMA2 );
  m_dvChart.Add( 3, m_ceSlopeOfSlopeOfSMA2 );
  m_dvChart.Add( 3, m_ceSlopeOfBollinger2Offset );
  m_dvChart.Add( 4, m_ceBollinger1Offset );
  m_dvChart.Add( 4, m_ceBollinger2Offset );
  m_dvChart.Add( 4, m_ceBollinger3Offset );
  m_dvChart.Add( 5, m_ceOutstandingLong );
  m_dvChart.Add( 5, m_ceOutstandingShort );
//  m_dvChart.Add( 3, m_ceRR );
  m_dvChart.Add( 6, m_cePLLong );
  m_dvChart.Add( 6, m_cePLShort );
  m_dvChart.Add( 6, m_cePLNet );
  //m_dvChart.Add( 7, m_ceLongTicks );
  //m_dvChart.Add( 7, m_ceShortTicks );
//  m_dvChart.Add( 5, m_ceSpread );

  m_dvChart.Add( 7, m_ceER3 );
  m_dvChart.Add( 7, m_ceER2 );
  m_dvChart.Add( 7, m_ceER1 );

  m_dvChart.Add( 8, m_ceBollinger3Ratio );
  m_dvChart.Add( 8, m_ceBollinger2Ratio );
  m_dvChart.Add( 8, m_ceBollinger1Ratio );

  m_ceSlopeOfSMA1.SetName( "SlopeOfSMA1" );
  m_ceSlopeOfSlopeOfSMA1.SetName( "SlopeOfSlopeOfSMA1" );

  m_ceSlopeOfSMA1.SetName( "SlopeOfSMA1" );
  m_ceSlopeOfSlopeOfSMA2.SetName( "SlopeOfSlopeOfSMA2" );
  m_ceSlopeOfBollinger2Offset.SetName( "SlopeOfBollinger2Offset" );

  m_cePLNet.SetName( "PL Net" );

  m_ceBollinger1Offset.SetName( "Bollinger1Offset" );
  m_ceBollinger2Offset.SetName( "Bollinger2Offset" );
  m_ceBollinger3Offset.SetName( "Bollinger3Offset" );

  m_ceSMA1.SetColour( ou::Colour::DarkOliveGreen );
  m_ceSMA2.SetColour( ou::Colour::Turquoise );
  m_ceSMA3.SetColour( ou::Colour::GreenYellow );

  m_cePLLong.SetColour( ou::Colour::Blue );
  m_cePLShort.SetColour( ou::Colour::Orange );
  m_cePLNet.SetColour( ou::Colour::Green );

  m_ceSlopeOfSMA1.SetColour( ou::Colour::DarkOliveGreen );
  m_ceSlopeOfSMA2.SetColour( ou::Colour::Turquoise );

  m_ceSlopeOfSlopeOfSMA1.SetColour( ou::Colour::MediumAquamarine );
  m_ceSlopeOfSlopeOfSMA2.SetColour( ou::Colour::ForestGreen );

  m_ceOutstandingLong.SetColour( ou::Colour::Blue );
  m_ceOutstandingShort.SetColour( ou::Colour::Red );

  m_ceER1.SetColour( ou::Colour::DarkOliveGreen );
  m_ceER2.SetColour( ou::Colour::Turquoise );
  m_ceER3.SetColour( ou::Colour::GreenYellow );

  m_ceBollinger1Ratio.SetColour( ou::Colour::DarkOliveGreen );
  m_ceBollinger2Ratio.SetColour( ou::Colour::Turquoise );
  m_ceBollinger3Ratio.SetColour( ou::Colour::GreenYellow );

  m_ceUpperBollinger1.SetColour( ou::Colour::DarkOliveGreen );
  m_ceLowerBollinger1.SetColour( ou::Colour::DarkOliveGreen );
  m_ceUpperBollinger2.SetColour( ou::Colour::Turquoise );
  m_ceLowerBollinger2.SetColour( ou::Colour::Turquoise );
  m_ceUpperBollinger3.SetColour( ou::Colour::GreenYellow );
  m_ceLowerBollinger3.SetColour( ou::Colour::GreenYellow );

  m_ceBollinger1Offset.SetColour( ou::Colour::DarkOliveGreen );
  m_ceBollinger2Offset.SetColour( ou::Colour::Turquoise );
  m_ceBollinger3Offset.SetColour( ou::Colour::GreenYellow );

  m_ceSlopeOfBollinger2Offset.SetColour( ou::Colour::DarkMagenta );

  //m_ceLongTicks.SetColour( ou::Colour::Blue );
  //m_ceShortTicks.SetColour( ou::Colour::Red );

  m_barFactory.SetOnBarComplete( MakeDelegate( this, &Strategy::HandleBarCompletion ) );

  m_sim->OnConnected.Add( MakeDelegate( this, &Strategy::HandleSimulatorConnected ) );
  m_sim->OnDisconnected.Add( MakeDelegate( this, &Strategy::HandleSimulatorDisConnected ) );

  m_pExecutionProvider = m_sim;
  m_pExecutionProvider->OnConnected.Add( MakeDelegate( this, &Strategy::HandleOnExecConnected ) );
  m_pExecutionProvider->OnDisconnected.Add( MakeDelegate( this, &Strategy::HandleOnExecDisconnected ) );

  m_pDataProvider = m_sim;
  m_pDataProvider->OnConnected.Add( MakeDelegate( this, &Strategy::HandleOnData1Connected ) );
  m_pDataProvider->OnDisconnected.Add( MakeDelegate( this, &Strategy::HandleOnData1Disconnected ) );

  ou::tf::CInstrumentManager& mgr( ou::tf::CInstrumentManager::Instance() );
  m_pTestInstrument = mgr.Exists( "+GCZ11" ) ? mgr.Get( "+GCZ11" ) : mgr.ConstructFuture( "+GCZ11", "SMART", 2011, 12 );
  m_pTestInstrument->SetMultiplier( 100 );
  m_pTestInstrument->SetMinTick( 0.1 );

//  m_sim->SetGroupDirectory( "/semiauto/2011-Sep-23 19:17:48.252497" );
  //m_sim->SetGroupDirectory( "/app/semiauto/2011-Nov-06 18:54:22.184889" );
  //m_sim->SetGroupDirectory( "/app/semiauto/2011-Nov-07 18:53:31.016760" );
  m_sim->SetGroupDirectory( "/app/semiauto/2011-Nov-08 18:58:29.396624" );
//  m_sim->SetExecuteAgainst( ou::tf::CSimulateOrderExecution::EAQuotes );
  
  m_sim->AddQuoteHandler( m_pTestInstrument, MakeDelegate( this, &Strategy::HandleQuote ) );
  m_sim->AddTradeHandler( m_pTestInstrument, MakeDelegate( this, &Strategy::HandleTrade ) );
  //m_sim->AddQuoteHandler( m_pTestInstrument, MakeDelegate( this, &Strategy::HandleFirstQuote ) );
  //m_sim->AddTradeHandler( m_pTestInstrument, MakeDelegate( this, &Strategy::HandleFirstTrade ) );
  m_sim->SetOnSimulationComplete( MakeDelegate( this, &Strategy::HandleSimulationComplete ) );

  m_pPositionLong.reset( new ou::tf::CPosition( m_pTestInstrument, m_sim, m_sim ) );
  m_pPositionLong->OnExecution.Add( MakeDelegate( this, &Strategy::HandleExecution ) );
  m_pPositionLong->OnCommission.Add( MakeDelegate( this, &Strategy::HandleCommission ) );

  m_pOrdersOutstandingLongs = new OrdersOutstandingLongs( m_pPositionLong );

  m_pPositionShort.reset( new ou::tf::CPosition( m_pTestInstrument, m_sim, m_sim ) );
  m_pPositionShort->OnExecution.Add( MakeDelegate( this, &Strategy::HandleExecution ) );
  m_pPositionShort->OnCommission.Add( MakeDelegate( this, &Strategy::HandleCommission ) );

  m_pOrdersOutstandingShorts = new OrdersOutstandingShorts( m_pPositionShort );
}

Strategy::~Strategy(void) {

  m_sim->SetOnSimulationComplete( 0 );
  m_sim->RemoveQuoteHandler( m_pTestInstrument, MakeDelegate( this, &Strategy::HandleQuote ) );
  m_sim->RemoveTradeHandler( m_pTestInstrument, MakeDelegate( this, &Strategy::HandleTrade ) );

  m_pDataProvider->OnConnected.Remove( MakeDelegate( this, &Strategy::HandleOnData1Connected ) );
  m_pDataProvider->OnDisconnected.Remove( MakeDelegate( this, &Strategy::HandleOnData1Disconnected ) );

  m_pExecutionProvider->OnConnected.Remove( MakeDelegate( this, &Strategy::HandleOnExecConnected ) );
  m_pExecutionProvider->OnDisconnected.Remove( MakeDelegate( this, &Strategy::HandleOnExecDisconnected ) );

  m_sim->OnConnected.Remove( MakeDelegate( this, &Strategy::HandleSimulatorConnected ) );
  m_sim->OnDisconnected.Remove( MakeDelegate( this, &Strategy::HandleSimulatorDisConnected ) );

  m_barFactory.SetOnBarComplete( 0 );

  ou::tf::CProviderManager::Instance().Release( "sim01" );

}

void Strategy::HandleSimulatorConnected( int ) {
  m_sim->Run();
}

void Strategy::HandleSimulatorDisConnected( int ) {
}

void Strategy::HandleOnExecConnected( int ) {
}

void Strategy::HandleOnExecDisconnected( int ) {
}

void Strategy::HandleOnData1Connected( int ) {
}

void Strategy::HandleOnData1Disconnected( int ) {
}

void Strategy::Start( const std::string& sSymbolPath ) {
  m_sim->Connect();
}

void Strategy::HandleQuote( const ou::tf::CQuote& quote ) {

  if ( !quote.Valid() ) {
    return;
  }
  // should also check that a price within 2 - 3 sigma of last

  // problems occur when long trend happens and can't get out of opposing position.

  ptime dt( quote.DateTime() );

  m_quoteLast = quote;
  m_quotes.Append( quote );

  // high speed simple moving average
  ou::tf::TSSWStatsMidQuote& sma1( m_sma2 );
  sma1.Update();

  m_pricesSlopeOfSlopeOfSMA1.Append( ou::tf::CPrice( dt, sma1.Slope() ) );
  m_tsswSlopeOfSlopeOfSMA1.Update();

  // medium speed moving average
  ou::tf::TSSWStatsMidQuote& sma2( m_sma6 );
  sma2.Update();

  m_pricesSlopeOfSlopeOfSMA2.Append( ou::tf::CPrice( dt, sma2.Slope() ) );
  m_tsswSlopeOfSlopeOfSMA2.Update();

  m_pricesBollinger2Offset.Append( ou::tf::CPrice( dt, sma2.BBOffset() ) );
  m_tsswSlopeOfBollinger2Offset.Update();

  // slow speed moving average
  ou::tf::TSSWStatsMidQuote& sma3( m_sma7 );
  sma3.Update();

//  double spread = quote.Ask() - quote.Bid();
//  m_spreads.Append( ou::tf::CPrice( dt, spread ) );
//  m_tsswSpreads.Update();

  if ( 500 < m_quotes.Size() ) {

    m_pOrdersOutstandingLongs->HandleQuote( quote );
    m_pOrdersOutstandingShorts->HandleQuote( quote );

//    m_ceSpread.Add( dt, m_tsswSpreads.MeanY() );

    unsigned int cntLongs = m_pOrdersOutstandingLongs->GetCountOfOutstandingMatches();
    m_ceOutstandingLong.Add( dt, cntLongs );
    unsigned int cntShorts = m_pOrdersOutstandingShorts->GetCountOfOutstandingMatches();
    m_ceOutstandingShort.Add( dt, cntShorts );
    unsigned int dif = ( cntLongs > cntShorts ) ? cntLongs - cntShorts : cntShorts - cntLongs;

    m_ceSMA1.Add( dt, sma1.MeanY() );
    m_ceSlopeOfSMA1.Add( dt, sma1.Slope() );
    double direction1 = m_tsswSlopeOfSlopeOfSMA1.Slope();
    if ( ( 0.00008 < direction1 ) || ( -0.00008 > direction1 ) ) {
      direction1 = 0.00008;
    }
    else {
      m_ceSlopeOfSlopeOfSMA1.Add( dt, direction1 * 400.0 );
    }

//    m_ceUpperBollinger1.Add( dt, sma1.BBUpper() );
//    m_ceLowerBollinger1.Add( dt, sma1.BBLower() );
    m_ceBollinger1Offset.Add( dt, sma1.BBOffset() );

    double slope2 = sma2.Slope();

    m_ceSMA2.Add( dt, sma2.MeanY() );
    m_ceSlopeOfSMA2.Add( dt, slope2 * 10.0 );
    m_ceUpperBollinger2.Add( dt, sma2.BBUpper() );
    m_ceLowerBollinger2.Add( dt, sma2.BBLower() );
    m_ceBollinger2Offset.Add( dt, sma2.BBOffset() );

    double direction2 = m_tsswSlopeOfSlopeOfSMA2.Slope();
    if ( ( 0.00001 < direction2 ) || ( -0.00001 > direction2 ) ) {
      direction2 = 0.0;
    }
    else {
      m_ceSlopeOfSlopeOfSMA2.Add( dt, direction2 * 2000.0 );
    }

    double dblBollingerSlope = m_tsswSlopeOfBollinger2Offset.Slope();
    m_ceSlopeOfBollinger2Offset.Add( dt, dblBollingerSlope * 2.0 );

    m_ceSMA3.Add( dt, sma3.MeanY() );
    m_ceUpperBollinger3.Add( dt, sma3.BBUpper() );
    m_ceLowerBollinger3.Add( dt, sma3.BBLower() );
    m_ceBollinger3Offset.Add( dt, sma3.BBOffset() );

    //m_ceRR.Add( quote.DateTime(), m_sma5min.RR() );
    double dblPLLong = m_pPositionLong->GetRealizedPL() + m_pPositionLong->GetUnRealizedPL() - m_pPositionLong->GetCommissionPaid();
    double dblPLShort = m_pPositionShort->GetRealizedPL() + m_pPositionShort->GetUnRealizedPL() - m_pPositionShort->GetCommissionPaid();
    m_cePLLong.Add( dt, dblPLLong );
    m_cePLShort.Add( dt, dblPLShort );
    m_cePLNet.Add( dt, dblPLLong + dblPLShort );

    double midpoint = quote.Midpoint();
    double val;
    val = ( midpoint - sma1.MeanY() ) / ( sma1.BBOffset() );
    if ( 1.1 < val ) val = 1.1;
    else 
      if ( -1.1 > val ) val = -1.1;
    m_ceBollinger1Ratio.Add( dt, val );

    val = ( midpoint - sma2.MeanY() ) / ( sma2.BBOffset() );
    if ( 1.1 < val ) val = 1.1;
    else 
      if ( -1.1 > val ) val = -1.1;
    m_ceBollinger2Ratio.Add( dt, val );

    val = ( midpoint - sma3.MeanY() ) / ( sma3.BBOffset() );
    if ( 1.1 < val ) val = 1.1;
    else 
      if ( -1.1 > val ) val = -1.1;
    m_ceBollinger3Ratio.Add( dt, val );

    typedef OrdersOutstanding::structRoundTrip structRoundTrip;

    switch ( m_stateTrade ) {
    case ETradeStart:
      m_stateTrade = ETradeOutOfMarket;
      break;
    case ETradeOutOfMarket:
      //if ( m_pPositionLong->OrdersPending() ) {
      if ( dt > m_dtEnd ) {
        m_stateTrade = ETradeCancel;
      }
      else {
        if ( 0.0 == direction1 ) {
          // don't do anything
        }
        else { // ** need to put in stop price into structure, use a bollinger band
          if ( 0.0 < direction1 ) { // go long
            m_pOrder = m_pPositionLong->
              PlaceOrder( ou::tf::OrderType::Limit, ou::tf::OrderSide::Buy, 1, m_pTestInstrument->NormalizeOrderPrice( quote.Midpoint() - 0.1 ) );
            m_pOrdersOutstandingLongs->AddOrderFilling( new structRoundTrip( m_pOrder ) );
            m_pOrder.reset();
            ++m_nUpTransitions;
            m_stateTrade = ETradeWaitShortEntry;
          }
          else { // go short
            m_pOrder = m_pPositionShort->
              PlaceOrder( ou::tf::OrderType::Limit, ou::tf::OrderSide::Sell, 1, m_pTestInstrument->NormalizeOrderPrice( quote.Midpoint() + 0.1 ) );
            m_pOrder->OnOrderFilled.Add( MakeDelegate( this, &Strategy::HandleOrderFilled ) );
            m_pOrdersOutstandingShorts->AddOrderFilling( new structRoundTrip( m_pOrder ) );
            m_pOrder.reset();
            ++m_nDnTransitions;
            m_stateTrade = ETradeWaitLongEntry;
          }
        }
      }
      break;
    case ETradeWaitLongEntry:
      if ( dt > m_dtEnd ) {
        m_stateTrade = ETradeCancel;
      }
      else {
        if ( 0.0 < direction1 ) {
          m_pOrdersOutstandingLongs->CancelAll();
          m_pOrder = m_pPositionLong->
            PlaceOrder( ou::tf::OrderType::Limit, ou::tf::OrderSide::Buy, 1, m_pTestInstrument->NormalizeOrderPrice( quote.Midpoint() - 0.1 ) );
          m_pOrder->OnOrderFilled.Add( MakeDelegate( this, &Strategy::HandleOrderFilled ) );
          m_pOrdersOutstandingLongs->AddOrderFilling( new structRoundTrip( m_pOrder ) );
          m_pOrder.reset();
          ++m_nUpTransitions;
          m_stateTrade = ETradeWaitShortEntry;
        }
      }
      break;
    case ETradeWaitShortEntry:
      if ( dt > m_dtEnd ) {
        m_stateTrade = ETradeCancel;
      }
      else {
        if ( 0.0 > direction1 ) {
          m_pOrdersOutstandingShorts->CancelAll();
          m_pOrder = m_pPositionShort->
            PlaceOrder( ou::tf::OrderType::Limit, ou::tf::OrderSide::Sell, 1, m_pTestInstrument->NormalizeOrderPrice( quote.Midpoint() + 0.1 ) );
          m_pOrder->OnOrderFilled.Add( MakeDelegate( this, &Strategy::HandleOrderFilled ) );
          m_pOrdersOutstandingShorts->AddOrderFilling( new structRoundTrip( m_pOrder ) );
          m_pOrder.reset();
          ++m_nDnTransitions;
          m_stateTrade = ETradeWaitLongEntry;
        }
      }
      break;
    case ETradeCancel:
          m_pPositionLong->CancelOrders();
          m_pPositionShort->CancelOrders();
          m_pOrdersOutstandingLongs->CancelAll();
          m_pOrdersOutstandingShorts->CancelAll();
          m_stateTrade = ETradeClose;
      break;
    case ETradeClose:
          m_pPositionLong->ClosePosition();
          m_pPositionShort->ClosePosition();
          m_stateTrade = ETradeDone;
      break;
    case ETradeDone:
      break;
    }
  }
}

void Strategy::HandleOrderFilled( const ou::tf::COrder& order ) {
  switch ( order.GetOrderSide() ) {
  case ou::tf::OrderSide::Sell:
    m_ceShorts.AddLabel( order.GetDateTimeOrderFilled(), order.GetAverageFillPrice(), "" );
    break;
  case ou::tf::OrderSide::Buy:
    m_ceLongs.AddLabel( order.GetDateTimeOrderFilled(), order.GetAverageFillPrice(), "" );
    break;
  }
  const_cast<ou::tf::COrder&>( order ).OnOrderFilled.Remove( MakeDelegate( this, &Strategy::HandleOrderFilled ) );
}

void Strategy::HandleFirstQuote( const ou::tf::CQuote& quote ) {

  if ( !quote.Valid() ) {
    return;
  }

  m_quoteLast = quote;  // ensure we have a quote before we start to evaluate trades

  m_sim->RemoveQuoteHandler( m_pTestInstrument, MakeDelegate( this, &Strategy::HandleFirstQuote ) );
  m_sim->AddQuoteHandler( m_pTestInstrument, MakeDelegate( this, &Strategy::HandleQuote ) );
  m_sim->RemoveTradeHandler( m_pTestInstrument, MakeDelegate( this, &Strategy::HandleFirstTrade ) );
  m_sim->AddTradeHandler( m_pTestInstrument, MakeDelegate( this, &Strategy::HandleTrade ) );
}

void Strategy::HandleFirstTrade( const ou::tf::CTrade& trade ) {
}

void Strategy::HandleTrade( const ou::tf::CTrade& trade ) {

  m_trades.Append( trade );
  m_barFactory.Add( trade );

  m_er1.Update();
  m_er2.Update();
  m_er3.Update();

  m_ceER1.Add( trade.DateTime(), m_er1.Ratio() );
  m_ceER2.Add( trade.DateTime(), m_er2.Ratio() );
  m_ceER3.Add( trade.DateTime(), m_er3.Ratio() );

  ou::tf::CTrade::price_t price = trade.Trade();

  double mid = m_quoteLast.Midpoint();
  if ( price == mid ) {
    switch ( m_TradeDirection ) {
    case ETradeDirUnkn:
      m_TradeDirection = ETradeDirUp;
      //break; fall through instead
    case ETradeDirUp:
      //m_ceLongTicks.Add( trade.DateTime(), 1 );
      //m_ceShortTicks.Add( trade.DateTime(), 0 );
      break;
    case ETradeDirDn:
      //m_ceShortTicks.Add( trade.DateTime(), -1 );
      //m_ceLongTicks.Add( trade.DateTime(), 0 );
      break;
    }
  }
  else {
    if ( price > mid ) {
      m_TradeDirection = ETradeDirUp;
      //m_ceLongTicks.Add( trade.DateTime(), 1 );
      //m_ceShortTicks.Add( trade.DateTime(), 0 );
    }
    else {
      m_TradeDirection = ETradeDirDn;
      //m_ceShortTicks.Add( trade.DateTime(), -1 );
      //m_ceLongTicks.Add( trade.DateTime(), 0 );
    }
  }

}

void Strategy::HandleSimulationComplete( void ) {
  m_ss.str( "" );
  m_ss << m_nUpTransitions << " up changes, ";
  m_ss << m_nDnTransitions << " dn changes, ";
  m_pPositionLong->EmitStatus( m_ss );
  m_ss << ", ";
  m_pPositionShort->EmitStatus( m_ss );
  m_ss << ". ";
  m_sim->EmitStats( m_ss );
  std::cout << m_ss << std::endl;
}

void Strategy::HandleExecution( ou::tf::CPosition::execution_delegate_t del ) {
  m_ss << "Exec: " << del.second.GetTimeStamp() << ": ";
  m_pPositionLong->EmitStatus( m_ss );
  m_ss << ", ";
  m_pPositionShort->EmitStatus( m_ss );
  std::cout << m_ss << std::endl;
}

void Strategy::HandleCommission( const ou::tf::CPosition* pPosition ) {
  m_ss.str( "" );
  m_pPositionLong->EmitStatus( m_ss );
  m_ss << ", ";
  m_pPositionShort->EmitStatus( m_ss );
  std::cout << m_ss << std::endl;
}

void Strategy::HandleBarCompletion( const ou::tf::CBar& bar ) {
  m_ceBars.AddBar( bar );
  m_ceVolume.Add( bar.DateTime(), bar.Volume() );
}
