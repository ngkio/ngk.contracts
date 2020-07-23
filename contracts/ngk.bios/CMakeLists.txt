add_contract(ngk.bios ngk.bios ${CMAKE_CURRENT_SOURCE_DIR}/src/ngk.bios.cpp)

target_include_directories(ngk.bios
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_target_properties(ngk.bios
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

target_compile_options( ngk.bios PUBLIC )
