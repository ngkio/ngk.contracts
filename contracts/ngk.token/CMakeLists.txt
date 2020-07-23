add_contract(ngk.token ngk.token ${CMAKE_CURRENT_SOURCE_DIR}/src/ngk.token.cpp)

target_include_directories(ngk.token
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_target_properties(ngk.token
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

target_compile_options( ngk.token PUBLIC )
