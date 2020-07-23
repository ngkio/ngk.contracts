add_contract(ngk.proposal ngk.proposal ${CMAKE_CURRENT_SOURCE_DIR}/src/ngk.proposal.cpp)

target_include_directories(ngk.proposal
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_target_properties(ngk.proposal
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

target_compile_options( ngk.proposal PUBLIC )
