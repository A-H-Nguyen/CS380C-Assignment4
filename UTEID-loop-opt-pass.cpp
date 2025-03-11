#include "UTEID-loop-opt-pass.h"
#include "UTEID-loop-analysis-pass.h"
#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/Analysis/LoopInfo.h>
#include <llvm/IR/ValueMap.h>
#include <llvm/Support/raw_ostream.h>

using namespace llvm;

bool LoopInvariantCodeMotion::isLoopInvariant(llvm::Instruction *I) {
  auto T = I->getType();
  errs() << "\tCurr inst type: ";
  T->print(errs());
  errs() << "\n\n";

  return true;
}

bool LoopInvariantCodeMotion::safeToHoist(llvm::Instruction *I) {
  return true;
}

// Really simple function to see how deeply nested the loops in our function are
int LoopInvariantCodeMotion::maxLoopDepth(LoopPropertiesAnalysis::Result LP) {
  int res = 0;
  for (auto &P : LP) {
    if (P->depth > res) {
      res = P->depth;
    }
  }
  return res;
}

// If there's something that seems wierd code-style wise, it's LLVM's fault for
// having a really weird code style. I also don't know how to spell weard
PreservedAnalyses 
LoopInvariantCodeMotion::run(Function &F, 
                             FunctionAnalysisManager &FAM) {
  errs() << "hey ;)\n";

  // get the basic Loop Information analysis passes
  // (LI for Loop Info)
  auto &LI = FAM.getResult<LoopAnalysis>(F);

  // Code for getting the "loop properties" from part 1
  // (LP for Loop Properties)
  auto &LP = FAM.getResult<LoopPropertiesAnalysis >(F);

  // When I first wrote this, there were so many nested loops, just really bad.
  // My method to alleviate that is to create a map, where we map the depth value 
  // to an llvm::SmallVector (or std::vector) of LoopProperties objects 
  //
  // It should be something like:
  // { 1: [loop A, loop B, loop C]
  //   2: [loop D, loop E]
  //   ...
  //   N: [loop Y, loop Z] }
  //
  // Ugly, ugly templating! The Keys to this map, as said before, are the depth
  // values of the various loops (so integers). Each key will point to a vector
  // of loops. This is so that we can iterate through the loops on a per-depth
  // basis. Why do I care so much about depth? You will see later.
  // 
  // I would say this is the most C++ code I've ever seen, but I think I've 
  // seen Kayvan write worse.
  // ValueMap<int, SmallVector<LoopPropertiesAnalysis::LoopProperties*>> LPM;
  DenseMap<int, SmallVector<LoopPropertiesAnalysis::LoopProperties*>> LPM;
  for (auto L : LP) {
    auto &entry = LPM.FindAndConstruct(L->depth);
    entry.second.push_back(L);
  }

  // When we iterate through loops, we are going to analyze based on depth.
  // We start with the deepest nested loop, and hoist into the next level up,
  // then we got to that level, and so on until we reach the top level loops
  // which have a depth of 0
  //
  // In other words consider:
  // int x;
  // while (true) {
  //    while (true) {
  //      x = 0;
  //    }
  // }
  //
  // `x = 0;` will first get hoisted by a single level of depth:
  // int x;
  // while (true) {
  //    x = 0;
  //    while (true) {
  //    }
  // }
  // and then it will be hoisted out of the top level loop:
  // int x;
  // x = 0;
  // while (true) {
  //    while (true) {
  //    }
  // }
  for (int currDepth = maxLoopDepth(LP); currDepth > -1; currDepth--) {
    errs() << "Analyzing loops of depth = " << currDepth << "\n";
    
    // Here, we are iterating all the LoopProperties objects that have a depth
    // equal to currDepth, using the map that we created earlier
    for (auto &L : LPM.find(currDepth)->second) {
      errs() << "Current Loop:\n";
      L->print(errs());
      errs() << "\n";

      // Iterate through all the basic blocks in the loop, L
      for (auto &BB : L->loop->blocks()) {
        
        // Iterate through all instruction in basic block BB
        for (auto &I : *BB) {
          if (!isLoopInvariant(&I) && !safeToHoist(&I)) {
            continue;
          }
          
          // If an instruction is both loop invariant and safe to move, put
          // it either the parent of this loop, or if this is a top level
          // loop (i.e. depth == 0), then put this instruction outside the loop
          // entirely. "Hoist" is just the compiler jargin for lifting code
          // outside the loop. You can think of it as, taking the code, and 
          // carrying it upwards, outside the loop (hence, "hoist").
          errs() << "Hoist instruction: ";
          I.print(errs());
          errs() << "\n";
        }
      }
      errs() << "\n";
    }
    errs() << "\n";
  }

  errs() << "\nOutput IR of our pass:\n\n";
  F.print(errs());
  errs() << "\n\n";

  return PreservedAnalyses::all();
}

// New PM Registration
//-----------------------------------------------------------------------------
PassPluginLibraryInfo getLoopOptPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "UTEID-Loop-Opt-Pass", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "UTEID-loop-opt-pass") {
		                FPM.addPass(LoopSimplifyPass());
                    FPM.addPass(LoopInvariantCodeMotion());
                    return true;
                  }
                  return false;
                });
          }};
}

// This is the core interface for pass plugins. It guarantees that 'opt' will
// be able to recognize HelloWorld when added to the pass pipeline on the
// command line, i.e. via '-passes=hello-world'
extern "C" LLVM_ATTRIBUTE_WEAK ::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getLoopOptPassPluginInfo();
}

