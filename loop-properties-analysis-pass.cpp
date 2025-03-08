#include <llvm/Passes/PassPlugin.h>
#include <string>

#include "loop-properties-analysis-pass.h"

LoopPropertiesAnalysis::LoopProperties::LoopProperties(
    const llvm::LoopInfo &LI, const llvm::Loop *L, 
    unsigned int LID, llvm::StringRef FName) {
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
        if (llvm::isa<llvm::BranchInst>(I)) {
          numBranches++;
        }
      }
    }
  }

  id = LID; 
  func = FName;
  depth = L->getLoopDepth();     
  subLoops = !L->isInnermost();
  BBs = numBasicBlocks;
  instrs = numInstr;
  atomics = numAtomics;
  branches = numBranches;
}

void LoopPropertiesAnalysis::LoopProperties::print(llvm::raw_ostream &OS) {
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

LoopPropertiesAnalysis::Result 
LoopPropertiesAnalysis::run(llvm::Function &F, llvm::FunctionAnalysisManager &FAM) {
  llvm::errs() << "Here in function:\t" << F.getName() << "\n\n";
  
  auto LV = std::vector<LoopProperties*>();
  auto& li = FAM.getResult<llvm::LoopAnalysis>(F);

  // Start our pass by iterating through only the top level loops (depth = 0)
  for (auto &loop : li.getLoopsInPreorder()) {
    // llvm::errs() << "Current Loop:\n"; 
    // loop->print(llvm::errs());
    // llvm::errs() << "\n"; 
    LV.push_back(new LoopProperties(li, loop, LID, F.getName()));
    LID++;
  }

  // for (auto &L : LV) {
  //   L->print(llvm::errs());
  // }

  return LV;
}

//-----------------------------------------------------------------------------
// New PM Registration
//-----------------------------------------------------------------------------
llvm::PassPluginLibraryInfo getLoopAnalysisPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "UTEID-Loop-Analysis-Pass", LLVM_VERSION_STRING,
          [](llvm::PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](llvm::StringRef Name, llvm::FunctionPassManager &FPM,
                   llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) {
                  if (Name == "loop-properties-printer") {
                    FPM.addPass(LoopPropertiesPrinter());
                    return true;
                  }
                  return false;
                });
            PB.registerAnalysisRegistrationCallback(
                [](llvm::FunctionAnalysisManager &FAM) {
                  FAM.registerPass([&] { return LoopPropertiesAnalysis(); });
                });
          }};
}

// This is the core interface for pass plugins. It guarantees that 'opt' will
// be able to recognize HelloWorld when added to the pass pipeline on the
// command line, i.e. via '-passes=hello-world'
extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getLoopAnalysisPluginInfo();
}
