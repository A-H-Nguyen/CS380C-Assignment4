#ifndef MP49774_AN35288_LOOP_OPT_PASS_H
#define MP49774_AN35288_LOOP_OPT_PASS_H

#include <llvm/IR/Instruction.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Passes/PassPlugin.h>
#include <llvm/Support/raw_ostream.h>

// additional passes
#include <llvm/Analysis/LoopInfo.h>
#include <llvm/IR/Dominators.h>

#include "mp49774-an35288-loop-analysis-pass.h"

class LoopInvariantCodeMotion : 
    public llvm::PassInfoMixin<LoopInvariantCodeMotion> {

private:
  bool isLoopInvariant(llvm::Instruction *I, const llvm::LoopInfo &LI);
  bool safeToHoist(llvm::Instruction *I, const llvm::LoopInfo &LI, const llvm::DominatorTree &DT);
  int maxLoopDepth(LoopPropertiesAnalysis::Result LP);

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
