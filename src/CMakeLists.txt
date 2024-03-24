cmake_minimum_required(VERSION 3.15)

project(jmm DESCRIPTION "J-- Compiler" VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

add_executable(jmm 
    main.cpp 
    lexer/lexer.cpp 
    common/util.cpp
    common/ast.cpp
    common/ast_printer.cpp
    driver/driver.cpp
    parser/parser.cpp
    semanal/analyzer.cpp
    semanal/global_visitor.cpp
    semanal/func_visitor.cpp
    semanal/type_visitor.cpp
    )


add_compile_definitions($<$<CONFIG:DEBUG>:LEXER_DEBUG>)
add_compile_definitions($<$<CONFIG:DEBUG>:PARSER_DEBUG>)
add_compile_definitions($<$<CONFIG:DEBUG>:SEMANAL_DEBUG>)
target_include_directories(jmm PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

