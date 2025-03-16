# Requirements

We support linux (ubuntu) and MacOS. Windows users should use WSL2 or the Department lab machines.

We assume you're using llvm-19. Mainly we assume that your LLVM uses the new pass manager. If that means nothing to you then ignore it.

LLVM-19 binaries are avalible here: https://github.com/llvm/llvm-project/releases/tag/llvmorg-19.1.7 as well as on your favorite package manager `brew install llvm@19` or `apt install llvm-19` (after adding llvm to apt sources list: `/etc/apt/sources.list`)

If you encounter build errors on a clean pull its very likely that the include/lib folders are wrong so open an issue with your env, or make a piazza post with you env. 

# Building

This should be familiar for anyone who has used CMake before. if not then follow the instructions below.

- Clone the repo.
- CMake build files into `./build` using `cmake -S . -B ./build`
- Run make on the newly built files. `make -C ./build`

# Run Commands
These commands assume that you are running from within the `build/` directory.

## clang
```
clang++ -S -emit-llvm -g0 -O0 -Xclang -disable-O0-optnone ../test-cases/<input>.cpp -o ../test-cases/<output>.ll
```
Note, we do `-O0` to turn off clang's optimizations. Otherwise, clang may automatically do loop invariant code motion for us.

## opt
For the analysis pass -- if you want to see the output of the analysis, call the printer pass.
```
opt -load-pass-plugin ./libloop-analysis-pass.so -passes=loop-properties-printer \
    ../test-cases/<input>.ll
```

For the transformation pass:
```
opt -load-pass-plugin ./libloop-analysis-pass.so \
    -load-pass-plugin ./libloop-opt-pass.so -passes=UTEID-loop-opt-pass \
    ../test-cases/<input>.ll
```
