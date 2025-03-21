cmake_minimum_required(VERSION 3.10)
project(CS380CLAB LANGUAGES CXX)

set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  # Add link flags specific to Darwin
  message("OSx Compile.")
  include_directories(/opt/homebrew/opt/llvm/include)
  link_directories(/opt/homebrew/opt/llvm/lib)
else()
  # These paths might be borked. But it hashtag works on my machine
  message("Linux Compile. looking for LLVM headers in /usr/lib/llvm-19")
  include_directories(/usr/lib/llvm-19/include)
  link_directories(/usr/lib/llvm-19/lib)
endif()



include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-fvisibility-inlines-hidden" SUPPORTS_FVISIBILITY_INLINES_HIDDEN_FLAG)
if (${SUPPORTS_FVISIBILITY_INLINES_HIDDEN_FLAG} EQUAL "1")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility-inlines-hidden")
endif()

add_library(loop-analysis-pass SHARED
        mp49774-an35288-loop-analysis-pass.cpp
        #test-cases/binary_op_1.cpp
)

add_library(loop-opt-pass SHARED
        mp49774-an35288-loop-opt-pass.cpp
        #test-cases/binary_op_1.cpp
)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  # Add link flags specific to Darwin
  message("OSx Compile. add some silly flags to fix dylib")
  target_link_options(loop-analysis-pass PRIVATE -undefined dynamic_lookup)
  target_link_options(loop-opt-pass PRIVATE -undefined dynamic_lookup)
endif()
