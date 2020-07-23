cmake_minimum_required( VERSION 3.5 )

project(contracts)

set(NGK_WASM_OLD_BEHAVIOR "Off")
find_package(ngk.cdt)

add_subdirectory(ngk.bios)
add_subdirectory(ngk.msig)
add_subdirectory(ngk.system)
add_subdirectory(ngk.token)
add_subdirectory(ngk.wrap)
add_subdirectory(ngk.proposal)
