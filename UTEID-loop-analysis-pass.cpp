#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Instructions.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/Debug.h>
#include <string>
#include <vector>

using namespace llvm;

struct LoopProperties {
  unsigned int id;
  std::string func; // name of the function containing this loop
  int depth; // 0 if it is not nested; otherwise, 1 more depth parent loop
  bool subLoops; // determins whether this loop has any loop nested within it
  int BBs;
  int instrs;
  int atomics;
  int branches;

  LoopProperties(const LoopInfo &LI, const Loop *L, 
                 unsigned int ID, StringRef FName) { 
    int numInstr = 0;
    int numAtomics = 0;
    int numBranches = 0;

    // Calling loop.getNumBlocks isn't enough because this function doesn't
    // differentiate between blocks inside nested loops and blocks that are
    // only in the top level loop
    int numBasicBlocks = L->getNumBlocks();
    bool isInnerLoopBlock = false;
    for (auto &BB : L->blocks()) {
      // We don't want to count basic blocks from nested loop 
      if (LI.getLoopDepth(BB) > L->getLoopDepth()) {
        isInnerLoopBlock = true;
        numBasicBlocks--;
      }
      else {
        isInnerLoopBlock = false;
      }

      // turns out BB->getNumInstr does not exist (pain) so now I'm gonna iterate
      // over all the instructions in the basic block like a little fool
      for (auto &I : *BB) {
        numInstr++;

        // But, we can't get the number of atomics without iterating through all
        // instructions in the loop anyways
        if (I.isAtomic()) {
          numAtomics++;
        }

        if (!isInnerLoopBlock) {
          if (isa<BranchInst>(I)) {
            numBranches++;
          }
        }
      }
    }

    id = ID; 
    func = FName;
    depth = L->getLoopDepth();     
    subLoops = !L->isInnermost();
    BBs = numBasicBlocks;
    instrs = numInstr;
    atomics = numAtomics;
    branches = numBranches;
  }

  void print(raw_ostream &OS) {
    OS << id << ":\t"
       << "func="       << func 
       << ", depth="    << depth;

    if (subLoops) {
      OS << ", subLoops=true";
    } 
    else {
      OS << ", subLoops=false";
    }

    OS << ", BBs="      << BBs
       << ", instrs="   << instrs
       << ", atomics="  << atomics
       << ", branches=" << branches << "\n";
  }
};

// New PM implementation
struct LoopPass : PassInfoMixin<LoopPass> {
  unsigned int ID = 0;  // a global counter for each loop encountered, starting at 0
  std::vector<LoopProperties*> LV;

  // Main entry point, takes IR unit to run the pass on (&F) and the
  // corresponding pass manager (to be queried if need be)
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM) {
    errs() << "\nhey ;)\n\n";
    errs() << "Here in function:\t" << F.getName() << "\n\n";

    // get the loop information analysis passes
    auto& li = FAM.getResult<LoopAnalysis>(F);

    // Start our pass by iterating through only the top level loops (depth = 0)
    for (auto &loop : li.getLoopsInPreorder()) {
      // LLVM_DEBUG(errs() << "Current Loop:\n"; loop->print(errs()));
      errs() << "Current Loop:\n"; 
      loop->print(errs());
      errs() << "\n"; 

      LV.push_back(new LoopProperties(li, loop, ID, F.getName()));
      ID++;

      // for (auto &subloop : loop->getSubLoops()) {
      //   LV.push_back(new LoopProperties(li, subloop, ID, F.getName()));
      //   ID++;
      // }
    }

    for (auto &L : LV) {
      L->print(errs());
    }

    // for (auto &BB : F) {
    //   errs() << "Basic Block ";
    //   BB.printAsOperand(errs(), false);

    //   // this is supposed to be "basic block loop" I swear
    //   auto BBL = li.getLoopFor(&BB); 
    //   if (BBL) {
    //     errs() << " is in loop: "; 
    //     BBL->getLoopID()->printAsOperand(errs());
    //     errs() << "\tDepth = " << BBL->getLoopDepth();
    //     errs() << "\n";
    //   }
    //   else
    //     errs() << " is loopless\n";
    // }

    return PreservedAnalyses::all();
  }

  // Without isRequired returning true, this pass will be skipped for functions
  // decorated with the optnone LLVM attribute. Note that clang -O0 decorates
  // all functions with optnone.
  static bool isRequired() { return true; }
};

//-----------------------------------------------------------------------------
// New PM Registration
//-----------------------------------------------------------------------------
llvm::PassPluginLibraryInfo getHelloWorldPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "UTEID-Loop-Analysis-Pass", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "UTEID-loop-analysis-pass") {
		    FPM.addPass(LoopSimplifyPass());
                    FPM.addPass(LoopPass());
                    return true;
                  }
                  return false;
                });
          }};
}

// This is the core interface for pass plugins. It guarantees that 'opt' will
// be able to recognize HelloWorld when added to the pass pipeline on the
// command line, i.e. via '-passes=hello-world'
extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getHelloWorldPluginInfo();
}
