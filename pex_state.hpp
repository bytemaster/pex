#pragma once

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>

namespace eosio {

   typedef double real_type;

   /**
    *  Uses Bancor math to create a 50/50 relay between two asset types. The state of the
    *  bancor exchange is entirely contained within this struct. There are no external
    *  side effects associated with using this API.
    */
   struct [[eosio::table, eosio::contract("pex")]] pex_state {
      name              manager;
      extended_asset    supply; ///< exchange tokens supply
      asset             sold_peg; ///< peg tokens circulating 
      extended_asset    spare_collateral; ///< collateral tokens not in the market maker
      double            current_target_price; ///< the target price as defined by the price feed
      double            target_reserve_ratio = 4.0; ///< the point at which new peg tokens start to be issued rather than increasing spare collateral
      double            fee = 0; ///< the percent of each trade that is kept by the exchange 

      block_timestamp_type last_price_in_range;
      block_timestamp_type last_sync;

      extended_asset collateral_balance;
      extended_asset pegged_balance;

      uint64_t primary_key()const { return supply.quantity.symbol.raw(); }
      uint64_t get_peg_symbol()const { return pegged_balance.quantity.symbol.raw(); } /// index on this to ensure unique

      /**
       *  @param pegio - when converting to or from exchange tokens some peg tokens may be required or
       *  generated, this parameter is set to the quantity required
       */
      extended_asset convert( extended_asset from, extended_symbol to, asset& pegio );
      void           init( name           manager,
                           double         fee,
                           extended_asset initial_collateral, 
                           double         initial_price, 
                           double         target_reserve_ratio, 
                           extended_symbol supply_symbol, symbol peg_symbol );

      void           sync_toward_feed( block_timestamp_type now );
      void           sync_toward_target_reserve();

      EOSLIB_SERIALIZE( pex_state, (manager)(supply)(sold_peg)(spare_collateral)(current_target_price)(target_reserve_ratio)(fee)(collateral_balance)(pegged_balance) )

      private:
         extended_asset convert_to_exchange( extended_asset& connector_balance, extended_asset in ); 
         extended_asset convert_from_exchange( extended_asset& connector_balance, extended_asset in );
         extended_asset buyex( const extended_asset& colin, asset& pegout );
         extended_asset sellex( const extended_asset& colin, asset& pegin );
   };

   struct [[eosio::table, eosio::contract("pex")]] pegs {
      symbol peg_symbol;
      uint64_t primary_key(){ return peg_symbol.raw(); }
   };

   typedef eosio::multi_index<"markets"_n, pex_state > market_table;
   typedef eosio::multi_index<"pegtokens"_n, pegs> pegs_table;

} /// namespace eosio
