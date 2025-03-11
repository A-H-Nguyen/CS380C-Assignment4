#include "UTEID-loop-opt-pass.h"
#include "loop-properties-analysis-pass.h"
#include <llvm/Support/raw_ostream.h>

using namespace llvm;

bool LoopInvariantCodeMotion::isLoopInvariant(llvm::Instruction *I) {
  auto T = I->getType();
  return false;
}

bool LoopInvariantCodeMotion::safeToHoist(llvm::Instruction *I) {
  return false;
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
  // for (auto &properties : LP) {
  //   properties->print(errs());
  // }

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
    
    // Instead of iterating through the default loop analysis LI, iterate
    // through the results of part 1's code. I think that this will make things
    // a little simpler in the long run
    for (auto &L : LP) {
      if (L->depth == currDepth) {
        errs() << "Current Loop:\n";
        L->print(errs());

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
    }
    errs() << "\n";
  }

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

