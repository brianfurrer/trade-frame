/************************************************************************
 * Copyright(c) 2009, One Unified. All rights reserved.                 *
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

#include <stdexcept>
#include <cassert>

#include <LibCommon/TimeSource.h>

#include "Order.h"

const std::string COrder::m_sSqlCreate(     
  "create table orders ( \
    orderid INTEGER PRIMARY KEY, \
    version SMALLINT DEFAULT 1, \
    positionid BIGINT NOT NULL, \
    instrumentid TEXT NOT NULL, \
    description TEXT NOT NULL, \
    orderstatus SMALLINT NOT NULL, \
    ordertype SMALLINT NOT NULL, \
    orderside SMALLINT NOT NULL, \
    price1 double NOT NULL, \
    price2 double NOT NULL, \
    signalprice double NOT NULL, \
    quantityordered INT NOT NULL, \
    quantityremaining INT NOT NULL, \
    quantityfilled INT NOT NULL, \
    averagefillprice double NOT NULL, \
    commission double NOT NULL, \
    datetimecreated BLOB, \
    datetimesubmitted BLOB, \
    datetimeclosed BLOB, \
    CONSTRAINT fk_orders_positionid \
      FOREIGN KEY(positionid) REFERENCES positions(positionid) \
        ON DELETE RESTRICT ON UPDATE CASCADE \
    CONSTRAINT fk_orders_instrumentid, \
      FOREIGN KEY(instrumentid) REFERENCES instruments(instrumentid) \
        ON DELETE RESTRICT ON UPDATE CASCADE \
       \
    );"
 );
const std::string COrder::m_sSqlSelect( 
  "SELECT positionid, instrumentid, orderstatus, ordertype, orderside, price1, price2, signalprice, description, \
    quantityordered, quantityremaning, quantityfilled, averagefillprice, commission,  \
    datetimecreated, datetimesubmitted, datetimeclosed \
    from orders where orderid = :id \
  ;" );
const std::string COrder::m_sSqlInsert( 
  "INSERT INTO orders (positionid, instrumentid, orderstatus, ordertype, orderside, price1, price2, description, signalprice, \
      quantityordered, quantityremaining, quantityfilled, averagefillprice, commission, \
      datetimecreated, datetimesubmitted, datetimeclosed ) \
    VALUES (:positionid:, :instrumentid, :orderstatus, :ordertype, :orderside, :price1, :price2, :description, :signalprice, \
      :quantityordered, :quantityremaining, :quantityfilled, :averagefillprice, :commission, \
      :datetimecreated, :datetimesubmitted, :datetimeclosed ) \
  ;" );
const std::string COrder::m_sSqlUpdate( 
  "UPDATE orders SET \
    positionid = :positionid, \
    instrumentid = :instrumentid, \
    orderstatus = :orderstatus, \
    ordertype = :ordertype, \
    orderside = :orderside, \
    price1 = :price1, \
    price2 = :price2, \
    description = :description, \
    signalprice = :signalprice, \
    quantityordered = :quantityordered, \
    quantityremaining = :quantityremaining, \
    quantityfilled = :quantityfilled, \
    averagefillprice = :averagefillprice, \
    commission = :commission, \
    datetimesubmitted = :datetimesubmitted, \
    datetimeclosed = :datetimeclosed \
  WHERE orderid = :id \
  ;" );
const std::string COrder::m_sSqlDelete( "DELETE FROM orders WHERE orderid = :id;" );

COrder::COrder(void) {
}

COrder::COrder( 
  CInstrument::pInstrument_cref instrument,
  OrderType::enumOrderType eOrderType,
  OrderSide::enumOrderSide eOrderSide, 
  unsigned long nOrderQuantity,
  idPosition_t idPosition,
  ptime dtOrderSubmitted
  ) 
:
  m_idPosition( idPosition ),
  m_pInstrument( instrument ), m_eOrderType( eOrderType ),
  m_eOrderSide( eOrderSide ), m_nOrderQuantity( nOrderQuantity ),
  m_dtOrderSubmitted( dtOrderSubmitted ),
  m_eOrderStatus( OrderStatus::Created ),
  m_nNextExecutionId ( 0 ),
  m_dblCommission( 0 ), m_dblPriceXQuantity( 0 ), m_dblAverageFillPrice( 0 ),
  m_nFilled( 0 ), m_nRemaining( nOrderQuantity ),
  m_dblPrice1( 0 ), m_dblPrice2( 0 ), 
  m_bOutsideRTH( false ),
  m_dblSignalPrice( 0 ),
  m_idOrder( 0 )
{
  ConstructOrder();
}

COrder::COrder( 
  CInstrument::pInstrument_cref instrument,
  OrderType::enumOrderType eOrderType,
  OrderSide::enumOrderSide eOrderSide, 
  unsigned long nOrderQuantity,
  double dblPrice1,
  idPosition_t idPosition,
  ptime dtOrderSubmitted
  ) 
:
  m_idPosition( idPosition ),
  m_pInstrument( instrument ), m_eOrderType( eOrderType ),
  m_eOrderSide( eOrderSide ), m_nOrderQuantity( nOrderQuantity ),
  m_dtOrderSubmitted( dtOrderSubmitted ), 
  m_eOrderStatus( OrderStatus::Created ),
  m_nNextExecutionId ( 0 ),
  m_dblCommission( 0 ), m_dblPriceXQuantity( 0 ), m_dblAverageFillPrice( 0 ),
  m_nFilled( 0 ), m_nRemaining( nOrderQuantity ),
  m_dblPrice1( dblPrice1 ), m_dblPrice2( 0 ), 
  m_bOutsideRTH( false ),
  m_dblSignalPrice( 0 ),
  m_idOrder( 0 )
{
  ConstructOrder();
}

COrder::COrder( 
  CInstrument::pInstrument_cref instrument,
  OrderType::enumOrderType eOrderType,
  OrderSide::enumOrderSide eOrderSide, 
  unsigned long nOrderQuantity,
  double dblPrice1, double dblPrice2,
  idPosition_t idPosition,
  ptime dtOrderSubmitted
  ) 
:
  m_idPosition( idPosition ),
  m_pInstrument( instrument ), m_eOrderType( eOrderType ),
  m_eOrderSide( eOrderSide ), m_nOrderQuantity( nOrderQuantity ),
  m_dtOrderSubmitted( dtOrderSubmitted ), 
  m_eOrderStatus( OrderStatus::Created ),
  m_nNextExecutionId ( 0 ),
  m_dblCommission( 0 ), m_dblPriceXQuantity( 0 ), m_dblAverageFillPrice( 0 ),
  m_nFilled( 0 ), m_nRemaining( nOrderQuantity ),
  m_dblPrice1( dblPrice1 ), m_dblPrice2( dblPrice2 ), 
  m_bOutsideRTH( false ),
  m_dblSignalPrice( 0 ),
  m_idOrder( 0 )
{
  ConstructOrder();
}

COrder::COrder( idOrder_t idOrder, sqlite3_stmt* pStmt ) 
: m_idOrder( idOrder ),
  m_bOutsideRTH( false ),
  m_nNextExecutionId ( 0 ),
  m_idPosition( sqlite3_column_int64( pStmt, 0 ) ),
  m_idInstrument( reinterpret_cast<const char*>( sqlite3_column_text( pStmt, 1 ) ) ),
  m_eOrderStatus( static_cast<OrderStatus::enumOrderStatus>(sqlite3_column_int( pStmt, 2 ) ) ),
  m_eOrderType( static_cast<OrderType::enumOrderType>(sqlite3_column_int( pStmt, 3 ) ) ),
  m_eOrderSide( static_cast<OrderSide::enumOrderSide>(sqlite3_column_int( pStmt, 4 ) ) ),
  m_dblPrice1( sqlite3_column_double( pStmt, 5 ) ),
  m_dblPrice2( sqlite3_column_double( pStmt, 6 ) ),
  m_dblSignalPrice( sqlite3_column_double( pStmt, 7 ) ),
  m_sDescription( reinterpret_cast<const char*>( sqlite3_column_text( pStmt, 8 ) ) ),
  m_nOrderQuantity( sqlite3_column_int( pStmt, 9 ) ),
  m_nRemaining( sqlite3_column_int( pStmt, 10 ) ),
  m_nFilled( sqlite3_column_int( pStmt, 11 ) ),
  m_dblAverageFillPrice( sqlite3_column_double( pStmt, 12 ) ),
  m_dblCommission( sqlite3_column_double( pStmt, 13 ) ),
  m_dtOrderCreated( *static_cast<const ptime*>( sqlite3_column_blob( pStmt, 14 ) ) ),
  m_dtOrderSubmitted( *static_cast<const ptime*>( sqlite3_column_blob( pStmt, 15 ) ) ),
  m_dtOrderClosed( *static_cast<const ptime*>( sqlite3_column_blob( pStmt, 16 ) ) )
{
}

COrder::~COrder(void) {
}

void COrder::ConstructOrder() {
//  try {
  m_dtOrderCreated = CTimeSource::Instance().Internal();
  assert( NULL != m_pInstrument.get() );
  //m_dtOrderSubmitted = not_a_date_time;  // already set as such
//  m_nOrderId = m_persistedorderid.GetNextOrderId();
//  }
//  catch (...) {
//    bOrderIdOk = false;
//    std::cout << "CIBTWS::PlaceOrder: Couldn't get the next order key." << std::endl;
//  }
}

void COrder::SetSendingToProvider() {
  assert( OrderStatus::Created == m_eOrderStatus );
  m_eOrderStatus = OrderStatus::SendingToProvider;
  m_dtOrderSubmitted = CTimeSource::Instance().Internal();
}

OrderStatus::enumOrderStatus COrder::ReportExecution(const CExecution &exec) { 
  // need to worry about fill after cancel
  assert( exec.GetOrderSide() == m_eOrderSide );
  bool bOverDone = false;
  if ( 0 == m_nRemaining ) {
    // yes this has happened, 2008/07/09 vmw
    std::cout << "Order " << m_idOrder << " overfilled with +" << exec.GetSize() << std::endl;
    m_eOrderStatus = OrderStatus::OverFilled;
    bOverDone = true;
  }
  else {
    m_nRemaining -= exec.GetSize();
  }
  m_nFilled += exec.GetSize();
  if ( m_nFilled > m_nOrderQuantity ) {
    std:: cout << "Order " << m_idOrder << " overfilled with +" << exec.GetSize() << std::endl;
    bOverDone = true;
  }
  if ( !bOverDone ) {
    m_dblPriceXQuantity += exec.GetPrice() * exec.GetSize();
    m_dblAverageFillPrice = m_dblPriceXQuantity / m_nFilled;
    if ( 0 == m_nRemaining ) {
      m_eOrderStatus = OrderStatus::Filled;
      m_dtOrderClosed = CTimeSource::Instance().Internal();
      OnOrderFilled( *this );
    }
    else {
      switch ( m_eOrderStatus ) {
      case OrderStatus::SendingToProvider:
      case OrderStatus::Submitted:
      case OrderStatus::Filling:
      case OrderStatus::PreSubmission:
        m_eOrderStatus = OrderStatus::Filling;
        break;
      case OrderStatus::Cancelled:
      case OrderStatus::CancelSubmitted:
      case OrderStatus::FillingDuringCancel:
      case OrderStatus::CancelledWithPartialFill:
        m_eOrderStatus = OrderStatus::FillingDuringCancel;
        break;
      case OrderStatus::OverFilled:
        break;
      default:
        std::cout << "COrder::ReportExecution " << static_cast<char>( m_eOrderStatus ) << std::endl;
        break;
      }
      OnPartialFill( *this );
    }
  }
  OnExecution( std::pair<const COrder&, const CExecution&>( *this, exec ) );
  return m_eOrderStatus;
}

void COrder::ActOnError(OrderErrors::enumOrderErrors eError) {
  switch( eError ) {
    case OrderErrors::Cancelled:
      m_eOrderStatus = OrderStatus::Cancelled;
      break;
    case OrderErrors::Rejected:
    case OrderErrors::InstrumentNotFound:
      m_eOrderStatus = OrderStatus::Rejected;
      break;
    case OrderErrors::NotCancellable:
      break;
  }
}

void COrder::SetCommission( double dblCommission ) { 
  m_dblCommission = dblCommission; 
  OnCommission( *this );
}

void COrder::SetOrderId( idOrder_t id ) {
  assert( 0 != id );
  assert( m_idOrder == 0 );
  m_idOrder = id;
}

void COrder::CreateDbTable( sqlite3* pDb ) {

  char* pMsg;
  int rtn;

  rtn = sqlite3_exec( pDb, m_sSqlCreate.c_str(), 0, 0, &pMsg );

  if ( SQLITE_OK != rtn ) {
    std::string sErr( "Error creating table orders: " );
    sErr += pMsg;
    sqlite3_free( pMsg );
    throw std::runtime_error( sErr );
  }

  rtn = sqlite3_exec( pDb, 
    "create index idx_orders_positionid on orders( positionid );",
    0, 0, &pMsg );

  if ( SQLITE_OK != rtn ) {
    std::string sErr( "Error creating index idx_orders_positionid: " );
    sErr += pMsg;
    sqlite3_free( pMsg );
    throw std::runtime_error( sErr );
  }
}

int COrder::BindDbKey( sqlite3_stmt* pStmt ) {
  int rtn( 0 );
  rtn = sqlite3_bind_int64( 
    pStmt, sqlite3_bind_parameter_index( pStmt, ":id" ), m_idOrder );
  return rtn;
}

int COrder::BindDbVariables( sqlite3_stmt* pStmt ) {
  // need to split out into two sets, one for full: (insert), one for partial: (update)
  int rtn( 0 );
  rtn += sqlite3_bind_int64(
    pStmt, sqlite3_bind_parameter_index( pStmt, ":positionid" ), m_idPosition );
  rtn += sqlite3_bind_text( 
    pStmt, sqlite3_bind_parameter_index( pStmt, ":instrumentid" ), m_pInstrument->GetInstrumentName().c_str(), -1, SQLITE_STATIC );
  rtn += sqlite3_bind_text( 
    pStmt, sqlite3_bind_parameter_index( pStmt, ":description" ), m_sDescription.c_str(), -1, SQLITE_STATIC );
  rtn += sqlite3_bind_int( 
    pStmt, sqlite3_bind_parameter_index( pStmt, ":orderstatus" ), m_eOrderStatus );
  rtn += sqlite3_bind_int( 
    pStmt, sqlite3_bind_parameter_index( pStmt, ":ordertype" ), m_eOrderType );
  rtn += sqlite3_bind_int( 
    pStmt, sqlite3_bind_parameter_index( pStmt, ":orderside" ), m_eOrderSide );
  rtn += sqlite3_bind_double( 
    pStmt, sqlite3_bind_parameter_index( pStmt, ":price1" ), m_dblPrice1 );
  rtn += sqlite3_bind_double( 
    pStmt, sqlite3_bind_parameter_index( pStmt, ":price2" ), m_dblPrice2 );
  rtn += sqlite3_bind_double( 
    pStmt, sqlite3_bind_parameter_index( pStmt, ":signalprice" ), m_dblSignalPrice );
  rtn += sqlite3_bind_int( 
    pStmt, sqlite3_bind_parameter_index( pStmt, ":quantityordered" ), m_nOrderQuantity );
  rtn += sqlite3_bind_int( 
    pStmt, sqlite3_bind_parameter_index( pStmt, ":quantityremaining" ), m_nFilled );
  rtn += sqlite3_bind_int( 
    pStmt, sqlite3_bind_parameter_index( pStmt, ":quantityfilled" ), m_nRemaining );
  rtn += sqlite3_bind_double( 
    pStmt, sqlite3_bind_parameter_index( pStmt, ":averagefillprice" ), m_dblAverageFillPrice );
  rtn += sqlite3_bind_double( 
    pStmt, sqlite3_bind_parameter_index( pStmt, ":commission" ), m_dblCommission );
  // timestamps needed here, try to get into database native format for query capability
  rtn += sqlite3_bind_blob( 
    pStmt, sqlite3_bind_parameter_index( pStmt, ":datetimecreated" ), &m_dtOrderCreated, sizeof( m_dtOrderCreated ), NULL );
  rtn += sqlite3_bind_blob( 
    pStmt, sqlite3_bind_parameter_index( pStmt, ":datetimesubmitted" ), &m_dtOrderSubmitted, sizeof( m_dtOrderSubmitted ), NULL );
  rtn += sqlite3_bind_blob( 
    pStmt, sqlite3_bind_parameter_index( pStmt, ":datetimeclosed" ), &m_dtOrderClosed, sizeof( m_dtOrderClosed ), NULL );
  return rtn;
}
