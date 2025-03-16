#!/bin/bash

# Check if a source file is passed as an argument
if [ $# -ne 1 ]; then
    echo "Usage (assuming running from cwd=build): $0 <source-file>"
    exit 1
fi

# Update IR passes (if any changes)
make

# Get the source file and output file name
SOURCE_FILE=$1

# Ensure that the output directory exists
mkdir -p "../out/$(basename ${SOURCE_FILE%.*})/"

OG_OUTPUT_FILE="../out/$(basename ${SOURCE_FILE%.*})/og_$(basename ${SOURCE_FILE%.*}).ll"
OPT_OUTPUT_FILE="../out/$(basename ${SOURCE_FILE%.*})/opt_$(basename ${SOURCE_FILE%.*}).ll"

# clang the test case into the og dir
clang++-19 -S -emit-llvm -g -g0 -O0 -Xclang -disable-O0-optnone "$SOURCE_FILE" -o "$OG_OUTPUT_FILE" -I/usr/include/c++/v1/

opt-19 -passes="loop-simplify,mem2reg" "$OG_OUTPUT_FILE" -o "$OG_OUTPUT_FILE" -S

# Compile using our opt pass into the opt dir
opt-19 -S -load-pass-plugin ./libloop-analysis-pass.so -load-pass-plugin ./libloop-opt-pass.so -passes=mp49774-an35288-loop-opt-pass "$OG_OUTPUT_FILE" > "$OPT_OUTPUT_FILE"

# Yoink the IR into a binary .s file
llc "$OPT_OUTPUT_FILE"

# Yeet the .s binary into
g++ "${OPT_OUTPUT_FILE%.*}.s" -o "../out/$(basename ${SOURCE_FILE%.*})/testable_$(basename ${SOURCE_FILE%.*}).out"

# Check if the command succeeded
if [ $? -eq 0 ]; then
    echo "Unoptimized LLVM IR has been written to $OUTPUT_FILE"
else
    echo "Failed to generate LLVM IR."
    exit 1
fi
