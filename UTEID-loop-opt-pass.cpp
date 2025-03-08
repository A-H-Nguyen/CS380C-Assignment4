#include "UTEID-loop-opt-pass.h"
#include "loop-properties-analysis-pass.h"

using namespace llvm;

PreservedAnalyses 
LoopInvariantCodeMotion::run(Function &F, 
                             FunctionAnalysisManager &FAM) {
  errs() << "hey ;)\n";
  // get the loop information analysis passes
  auto &LI = FAM.getResult<LoopAnalysis>(F);
  auto &LP = FAM.getResult<LoopPropertiesAnalysis >(F);

  for (auto &properties : LP) {
    properties->print(errs());
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

