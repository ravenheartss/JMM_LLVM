cmake_minimum_required(VERSION 3.20.0)

project(jmm DESCRIPTION "J-- Compiler" VERSION 0.1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

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
    codegen/codegen_visitor.cpp
    codegen/codegen_ir.cpp
    codegen/codegen_globals.cpp
    )

add_compile_definitions($<$<CONFIG:DEBUG>:LEXER_DEBUG>)
add_compile_definitions($<$<CONFIG:DEBUG>:PARSER_DEBUG>)
add_compile_definitions($<$<CONFIG:DEBUG>:SEMANAL_DEBUG>)
target_include_directories(jmm PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

# Find the libraries that correspond to the LLVM components
# that we wish to use
llvm_map_components_to_libnames(llvm_libs support core irreader)

# Link against LLVM libraries
target_link_libraries(jmm ${llvm_libs})


