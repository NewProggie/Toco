cmake_minimum_required(VERSION 3.7)
project(Toco VERSION 0.1 LANGUAGES CXX)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)
include(FindCurses)

# Manually define package for LLVM, since version 3.9 is currently broken as it
# does still refer to jit instead of mcjit in LLVM-Config.cmake
set(LLVM_PATH /usr/local/Cellar/llvm/3.9/)
set(LLVM_INCLUDE_DIRS ${LLVM_PATH}/include)
link_directories(${LLVM_PATH}/lib)

# Generate syntax files
BISON_TARGET(toco_parser
    ${CMAKE_CURRENT_SOURCE_DIR}/src/toco_parser.y
    ${CMAKE_CURRENT_BINARY_DIR}/toco_parser.cc
    COMPILE_FLAGS "-d --defines=${CMAKE_CURRENT_BINARY_DIR}/toco_parser.h")

FLEX_TARGET(toco_lexer
    ${CMAKE_CURRENT_SOURCE_DIR}/src/toco_lexer.l
    ${CMAKE_CURRENT_BINARY_DIR}/toco_lexer.cc)

ADD_FLEX_BISON_DEPENDENCY(toco_lexer toco_parser)

add_executable(toco
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/nodes.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/llvm_codegen.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/llvm_codegen.h
    ${FLEX_toco_lexer_OUTPUTS}
    ${BISON_toco_parser_OUTPUTS})

target_include_directories(toco
    PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/src
      ${CMAKE_CURRENT_BINARY_DIR}
      ${LLVM_INCLUDE_DIRS})

set_target_properties(toco PROPERTIES CXX_STANDARD 14)

target_link_libraries(toco
    LLVMX86Disassembler LLVMX86AsmParser LLVMX86CodeGen LLVMGlobalISel
    LLVMSelectionDAG LLVMAsmPrinter LLVMDebugInfoCodeView LLVMDebugInfoMSF
    LLVMCodeGen LLVMScalarOpts LLVMInstCombine LLVMTransformUtils LLVMBitWriter
    LLVMX86Desc LLVMMCDisassembler LLVMX86Info LLVMX86AsmPrinter LLVMX86Utils
    LLVMMCJIT LLVMExecutionEngine LLVMTarget LLVMAnalysis LLVMProfileData
    LLVMRuntimeDyld LLVMObject LLVMMCParser LLVMBitReader LLVMMC LLVMCore
    LLVMSupport LLVMDemangle z m ${CURSES_LIBRARIES})