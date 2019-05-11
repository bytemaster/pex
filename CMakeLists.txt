cmake_minimum_required(VERSION 3.5)
project( pexpro VERSION 1.0.0 )

find_package( eosio.cdt )

add_contract( pex pex pex.cpp pex_state.cpp )
target_include_directories( pex.wasm PUBLIC . )

