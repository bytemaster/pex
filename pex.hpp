#pragma once
#include <eosio/eosio.hpp>
#include <pex_state.hpp>

using namespace eosio;

class [[eosio::contract]] pex : public contract {
  public:
     pex( name s, name code, datastream<const char*> ds )
     :contract(s,code,ds),_markets(s,s.value)
     {
        print( "construct me s: ", s, ",   code: ", code,"\n" );
     }

     [[eosio::action]]
     void create( name            manager,
                  double          fee_percent,
                  double          initial_price, 
                  double          target_reserve_ratio, 
                  extended_asset  initial_collateral, 
                  symbol          supply_symbol, symbol peg_symbol );


     [[eosio::action]]
     void convert( name user, symbol market, extended_asset from, extended_symbol to );

     [[eosio::action]]
     void test(){ print( "test me\n" ); }

     market_table _markets;
};
