#ifndef UTEID_LOOP_OPT_PASS_H
#define UTEID_LOOP_OPT_PASS_H

#include <llvm/Analysis/LoopInfo.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Passes/PassPlugin.h>
#include <llvm/Support/raw_ostream.h>

class LoopInvariantCodeMotion : 
    public llvm::PassInfoMixin<LoopInvariantCodeMotion> {

public:
  // Main entry point, takes IR unit to run the pass on (&F) and the
  // corresponding pass manager (to be queried if need be)
  llvm::PreservedAnalyses run(llvm::Function &F, llvm::FunctionAnalysisManager &FAM);

  // Without isRequired returning true, this pass will be skipped for functions
  // decorated with the optnone LLVM attribute. Note that clang -O0 decorates
  // all functions with optnone.
  static bool isRequired() { return true; }
};

#endif
