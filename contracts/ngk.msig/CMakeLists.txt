add_contract(ngk.msig ngk.msig ${CMAKE_CURRENT_SOURCE_DIR}/src/ngk.msig.cpp)

target_include_directories(ngk.msig
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_target_properties(ngk.msig
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

target_compile_options( ngk.msig PUBLIC )
