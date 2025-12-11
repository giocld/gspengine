cmake_minimum_required(VERSION 3.14...3.31)
project(
    gspengine
    LANGUAGES CXX C
)
add_executable( helloworld demo/helloworld.cpp )

set_target_properties( helloworld PROPERTIES CXX_STANDARD 20 )
add_custom_target( run_helloworld helloworld USES_TERMINAL )

option( USE_SANITIZER "Use Address Sanitizer" ON )
if( USE_SANITIZER )
    if(MSVC)
        add_compile_options(/fsanitize=address)
        add_link_options(/fsanitize=address)
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        ## Also `-fsanitize=memory` for Clang
        add_compile_options(-fsanitize=address)
        add_link_options(-fsanitize=address)
    endif()
endif()
