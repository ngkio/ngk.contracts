add_contract(ngk.wrap ngk.wrap ${CMAKE_CURRENT_SOURCE_DIR}/src/ngk.wrap.cpp)

target_include_directories(ngk.wrap
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_target_properties(ngk.wrap
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

target_compile_options( ngk.wrap PUBLIC )
