#pragma once

#ifndef LOOP_PROPERTIES_ANALYSIS_H
#define LOOP_PROPERTIES_ANALYSIS_H

#include <llvm/Analysis/LoopInfo.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Pass.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/Debug.h>
#include <llvm/Support/raw_ostream.h>
#include <vector>

class LoopPropertiesAnalysis : 
  public llvm::AnalysisInfoMixin<LoopPropertiesAnalysis> {

protected:
  struct LoopProperties {
    unsigned int id;
    std::string func; // name of the function containing this loop
    int depth; // 0 if it is not nested; otherwise, 1 more depth parent loop
    bool subLoops; // determins whether this loop has any loop nested within it
    int BBs;
    int instrs;
    int atomics;
    int branches;

    LoopProperties(const llvm::LoopInfo &LI, const llvm::Loop *L, 
                   unsigned int LID, llvm::StringRef FName);

    void print(llvm::raw_ostream &OS);
  };

public:
  using Result = std::vector<LoopProperties*>;
  Result run(llvm::Function &F, llvm::FunctionAnalysisManager &FAM);

  unsigned int LID = 0;  // a global counter for each loop encountered, starting at 0

private:
  static llvm::AnalysisKey Key;
  friend struct llvm::AnalysisInfoMixin<LoopPropertiesAnalysis>;
};

class LoopPropertiesPrinter : public llvm::PassInfoMixin<LoopPropertiesPrinter> {
public:
  llvm::PreservedAnalyses 
  run(llvm::Function& F, llvm::FunctionAnalysisManager& FAM) {
    auto& a = FAM.getResult<LoopPropertiesAnalysis >(F);
    for (auto &L : a) {
      L->print(llvm::errs());
    }
    return llvm::PreservedAnalyses::all();
  }
};
 


#endif
