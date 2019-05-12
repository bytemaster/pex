#pragma once
#include <eosio/eosio.hpp>
#include <pex_state.hpp>

using namespace eosio;

class [[eosio::contract]] pex : public contract {
  public:
     pex( name s, name code, datastream<const char*> ds )
     :contract(s,code,ds),_markets(s,s.value),_balances(s,s.value),_pegs(s,s.value)
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
     void open( name user, extended_symbol balance_type );

     [[eosio::action]]
     void close( name user, extended_symbol balance_type );

     [[eosio::action]]
     void withdraw( name user, extended_asset quantity );

     [[eosio::action]]
     void transfer( name from, name to, asset quantity, const std::string& );

     [[eosio::on_notify("eosio.token::transfer")]]
     void on_eosio_token_transfer( name from, name to, asset quantity ) {
        check( to == _self, "make sure notice is a deposit" );
        adjust_balance( from, extended_asset( quantity, "eosio.token"_n ) ); 
        print( "accept deposit of: ", quantity, "\n" );
     }


     struct [[eosio::table]] balance {
        int64_t        primary;
        name           owner;
        extended_asset balance;

        int64_t primary_key()const { return primary; }

        static checksum256 to_owner_asset( name owner, const extended_symbol& bal ) {
           checksum256 result;
           int64_t* parts = (int64_t*)&result;
           parts[0] = owner.value;
           parts[1] = bal.get_symbol().raw();
           parts[2] = bal.get_contract().value;
           parts[3] = 0;
           return result;
        }

        checksum256 get_owner_asset()const {
           return to_owner_asset( owner, balance.get_extended_symbol() );
        }
     };

  private:
     void adjust_balance( name owner, extended_asset delta );

     typedef eosio::multi_index<"balances"_n, balance,
                indexed_by< "ownerasset"_n, const_mem_fun<balance, checksum256, &balance::get_owner_asset> > 
             > balance_table;

     market_table    _markets;
     balance_table   _balances;
     pegs_table      _pegs;
};
