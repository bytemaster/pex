#include "pex.hpp"


void  pex::create( name            manager,
              double          fee_percent,
              double          initial_price, 
              double          target_reserve_ratio, 
              extended_asset  initial_collateral, 
              symbol          market_symbol, symbol peg_symbol ){
   require_auth( manager );
   check( market_symbol.precision() == initial_collateral.quantity.symbol.precision(), "maker asset precision must match collateral precision" );
   check( peg_symbol.precision() == initial_collateral.quantity.symbol.precision(), "peg asset precision must match collateral precision" );

   _markets.emplace( manager, [&]( auto& state ) {
      state.init( manager, fee_percent, initial_collateral, initial_price, target_reserve_ratio, extended_symbol( market_symbol, _self ), peg_symbol );
   });
}

[[eosio::action]]
void pex::convert( name user, symbol market, extended_asset from, extended_symbol to ) {
   require_auth( user );

   auto itr = _markets.find( market.raw() );
   check( itr != _markets.end(), "unknown market" );

   print( "init market state:\n" );
   print( "   mm shares: ", itr->supply, "\n" );
   print( "   collateral_balance: ", itr->collateral_balance, "\n" );
   print( "   pegged_balance:     ", itr->pegged_balance, "\n" );
   print( "   sold_pegged:        ", itr->sold_peg, "\n" );
   print( "   spare_collateral:   ", itr->spare_collateral, "\n" );
   print( "   implied price:      ",  double(itr->pegged_balance.quantity.amount) / itr->collateral_balance.quantity.amount , "\n" );

   asset          pegio;
   extended_asset out;
   _markets.modify( itr, _self, [&]( auto& state ) {
      out = state.convert( from, to, pegio );
   });
   print(  "-------------------\n" );
   print(  "in:    ", from, "\n" );
   print(  "out:   ", out, "\n" );
   print(  "pegio: ", pegio, "\n" );
   print(  "pegio.amount: ", pegio.amount, "\n" );


   print( "final market state:\n" );
   print( "   mm shares: ", itr->supply, "\n" );
   print( "   collateral_balance: ", itr->collateral_balance, "\n" );
   print( "   pegged_balance:     ", itr->pegged_balance, "\n" );
   print( "   sold_pegged:        ", itr->sold_peg, "\n" );
   print( "   spare_collateral:   ", itr->spare_collateral, "\n" );
   print( "   implied price:      ",  double(itr->pegged_balance.quantity.amount) / itr->collateral_balance.quantity.amount , "\n" );

}


