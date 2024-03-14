cmake_minimum_required(VERSION 3.15)

project(jmm DESCRIPTION "J-- Compiler" VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(jmm 
    main.cpp 
    lexer/lexer.cpp 
    common/token.cpp
    common/ast.cpp
    driver/driver.cpp
    parser/parser.cpp)


add_compile_definitions($<$<CONFIG:DEBUG>:LEXER_DEBUG>)
add_compile_definitions($<$<CONFIG:DEBUG>:PARSER_DEBUG>)
target_include_directories(jmm PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

