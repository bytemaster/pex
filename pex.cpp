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

   adjust_balance( manager, -initial_collateral);
   _pegs.emplace( manager, [&]( auto& record ) {
      record.peg_symbol = peg_symbol;
   });
   _markets.emplace( manager, [&]( auto& state ) {
      state.init( manager, fee_percent, initial_collateral, initial_price, target_reserve_ratio, extended_symbol( market_symbol, _self ), peg_symbol );
      adjust_balance( manager, state.supply );
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

   adjust_balance( user, -from );
   adjust_balance( user, out );
   adjust_balance( user, extended_asset( pegio, _self ) );


   print( "final market state:\n" );
   print( "   mm shares: ", itr->supply, "\n" );
   print( "   collateral_balance: ", itr->collateral_balance, "\n" );
   print( "   pegged_balance:     ", itr->pegged_balance, "\n" );
   print( "   sold_pegged:        ", itr->sold_peg, "\n" );
   print( "   spare_collateral:   ", itr->spare_collateral, "\n" );
   print( "   implied price:      ",  double(itr->pegged_balance.quantity.amount) / itr->collateral_balance.quantity.amount , "\n" );

}
[[eosio::action]]
void pex::open( name user, extended_symbol balance_type ) {
   require_auth( user );
   auto idx = _balances.get_index<"ownerasset"_n>();
   auto itr = idx.find( balance::to_owner_asset( user, balance_type ) );
   check( itr == idx.end(), "balance already open" );

   _balances.emplace( user, [&]( auto& record ) {
      record.owner   = user;
      record.primary = _balances.available_primary_key();
      record.balance = extended_asset(0, balance_type);
   });
}

void pex::close( name user, extended_symbol balance_type ) {
   require_auth( user );
   auto idx = _balances.get_index<"ownerasset"_n>();
   auto itr = idx.find( balance::to_owner_asset( user, balance_type ) );
   check( itr != idx.end(), "no balance" );
   check( itr->balance.quantity.amount == 0, "withdraw balance first" );
   idx.erase(itr);
}

void pex::withdraw( name user, extended_asset quantity ) {
   require_auth( user );
   adjust_balance( user, -quantity );
   check( quantity.contract != _self, "cannot withdraw pex tokens" );
      
   using transfer_action = eosio::action_wrapper<"transfer"_n, &pex::transfer>;
   transfer_action transfer_act{ quantity.contract, { _self, "active"_n} };
   transfer_act.send( _self, user, quantity.quantity, std::string("withdraw") );
}

void pex::transfer( name from, name to, asset quantity, const std::string& m ) {
   require_auth( from );
   require_recipient( from );
   require_recipient( to );

   check( from != to, "cannot send to self" );

   check( quantity.is_valid(), "invalid quantity" );
   check( quantity.amount > 0, "must transfer positive" );
   check( m.size() < 256, "memo has more than 256 bytes" );

   extended_asset q(quantity,_self);
   adjust_balance( from, -q );
   adjust_balance( to, q );
}

void pex::adjust_balance( name owner, extended_asset delta ) {
   if( delta.quantity.amount == 0 ) return;

   auto idx = _balances.get_index<"ownerasset"_n>();
   auto itr = idx.find( balance::to_owner_asset( owner, delta.get_extended_symbol() ) );

   if( itr == idx.end() ) {
      check( delta.quantity.amount > 0, "negative balances are not allowed" );
      _balances.emplace( owner, [&]( auto& record ) {
         record.owner   = owner;
         record.primary = _balances.available_primary_key();
         record.balance = delta;
      });
   } else {
      idx.modify( itr, owner, [&]( auto& record ) {
          record.balance += delta;
          check( record.balance.quantity.amount >= 0, "negative balances are not allowed" );
      });
   }
}


