#include <llvm/Passes/PassPlugin.h>
#include <string>

#include "mp49774-an35288-loop-analysis-pass.h"

using namespace llvm;

LoopPropertiesAnalysis::LoopProperties::LoopProperties(
    const LoopInfo &LI, const Loop *L, 
    unsigned int LID, StringRef FName) {
  int numInstr = 0;
  int numAtomics = 0;
  int numBranches = 0;
  int numBasicBlocks = L->getNumBlocks();

  // Calling loop.getNumBlocks isn't enough because this function doesn't
  // differentiate between blocks inside nested loops and blocks that are
  // only in the top level loop
  bool isInnerLoopBlock = false;

  for (auto &BB : L->blocks()) {
    // per the assignment specifications, we don't want to count basic blocks 
    // from nested loops/children of this current loop
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

      // Same as with num Basic Blocks -- don't want to count branch instrs
      // from the nested loops
      if (!isInnerLoopBlock) {
        if (isa<BranchInst>(I)) {
          numBranches++;
        }
      }
    }
  }

  id = LID; 
  func = FName;
  // We do loop depth minus 1 because part 1 tells us that loop depth is supposed
  // to go from 0, and by default, getLoopDepth returns 1 for the top level loop :(
  depth = L->getLoopDepth() - 1;     
  subLoops = !L->isInnermost();
  BBs = numBasicBlocks;
  instrs = numInstr;
  atomics = numAtomics;
  branches = numBranches;

  // We save the actual loop object here that contains all the basic block
  // and instruction info. Not required by part 1, but it makes writing part 2
  // a little easier in my opinion
  loop = L;
}

void LoopPropertiesAnalysis::LoopProperties::print(raw_ostream &OS) {
  OS << id << ": "
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

LoopPropertiesAnalysis::Result 
LoopPropertiesAnalysis::run(Function &F, FunctionAnalysisManager &FAM) {
  auto LV = std::vector<LoopProperties*>();
  auto& LI = FAM.getResult<LoopAnalysis>(F);

  // Start our pass by iterating through only the top level loops (depth = 0)
  for (auto &L : LI.getLoopsInPreorder()) {
    // errs() << "Current Loop:\n"; 
    // L->print(errs());
    // errs() << "\n"; 
    LV.push_back(new LoopProperties(LI, L, LID, F.getName()));
    LID++;
  }

  // for (auto &L : LV) {
  //   L->print(errs());
  // }

  return LV;
}

AnalysisKey LoopPropertiesAnalysis::Key;

//-----------------------------------------------------------------------------
// New PM Registration
//-----------------------------------------------------------------------------
PassPluginLibraryInfo getLoopAnalysisPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "mp49774-an35288-loop-properties-analysis-pass",
          LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "mp49774-an35288-loop-analysis-pass") {
                    FPM.addPass(LoopPropertiesPrinter());
                    return true;
                  }
                  return false;
            });
            PB.registerAnalysisRegistrationCallback(
                [](FunctionAnalysisManager &FAM) {
                  FAM.registerPass([&] { return LoopPropertiesAnalysis(); });
            });
          }};
}

// This is the core interface for pass plugins. It guarantees that 'opt' will
// be able to recognize HelloWorld when added to the pass pipeline on the
// command line, i.e. via '-passes=hello-world'
extern "C" LLVM_ATTRIBUTE_WEAK ::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getLoopAnalysisPluginInfo();
}
