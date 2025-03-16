#include "mp49774-an35288-loop-opt-pass.h"
#include "mp49774-an35288-loop-analysis-pass.h"
#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/Analysis/LoopInfo.h>
#include <llvm/IR/Constant.h>
#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/ValueMap.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/raw_ostream.h>

#include <llvm/Analysis/ValueTracking.h>
#include <llvm/IR/Dominators.h>

using namespace llvm;

/*
 * An instruction is loop invariant if both of the following are true:
 *
 * 1. It's one of the following LLVM instructions/classes:
 *
 *      binary operator, shift, select, cast, getelementptr
 *
 *      All other LLVM instructions are considered not loop-invariant.
 *
 *      Some particulars to avoid moving include:
 *
 *      terminators, phi, load, sttore, call, invoke, malloc, free, alloca, vanext, vaarg
 *
 * 2. Every operand of the instruction is either:
 *
 *      i) constant, or
 *
 *      ii) computed outside the loop.
 *
 */
bool LoopInvariantCodeMotion::isLoopInvariant(llvm::Instruction *I, 
                                              const llvm::LoopInfo &LI) {
  // binary operator, shift, select, cast, getelementptr
  // todo - why is shift not included??
  if (!(isa<BinaryOperator>(I) || isa<SelectInst>(I) || isa<CastInst>(I) || 
      isa<GetElementPtrInst>(I) || I->isShift()))
    return false;
    // errs() << "Howdy :)\n";

  auto curr_loop = LI.getLoopFor(I->getParent());
  SmallVector<bool> results;

  for (auto &OP : I->operands()) {

    //errs() << "\tOperand " << OP.getOperandNo() << ":\t";
    //OP->print(errs());
    //errs() << "\n";

    auto op_def = dyn_cast<Instruction>(OP.get());

      if (op_def && curr_loop->contains(op_def)) {
          return false;
      }

    /*
      auto arg_def = dyn_cast<Argument>(OP.get());
      if (op_def) {
      errs() << "\t\tOperand defined by instr\n";
      auto op_parent_blk = op_def->getParent();
      auto op_parent_loop = LI.getLoopFor(op_parent_blk);

      if (curr_loop != op_parent_loop) {
        errs() << "\t\tDefinition of operand " << OP.getOperandNo()
               << " is outside of the current loop\n";
        results.push_back(true);
      }
      else {
        errs() << "\t\tDefinition of operand " << OP.getOperandNo()
               << " is within the current loop\n";
        results.push_back(false);
      }
    }

    else if(arg_def){
        errs() << "\t\tDefinition of operand " << OP.getOperandNo()
               << " is from a function param\n";
        results.push_back(true);
    }

    // op is not defined within the loop, so proceed
    else {
      if (isa<Constant>(OP)) {
        errs() << "\t\tOperand " << OP.getOperandNo()
               << " is a constant\n";
        results.push_back(true);
      }
      else {
        errs() << "\t\tOperand " << OP.getOperandNo()
               << " is not a constant\n";
        results.push_back(false);
      }
    }*/
  }
    return true;

    /*bool loop_invariance = all_of(results, [](bool R) { return R; });
  if (loop_invariance) {
    errs() << "\tCurr instr is loop invariant\n";
  }
  else {
    errs() << "\tCurr instr is NOT loop invariant\n";
  }
  errs() << "\n\n";

  return loop_invariance;*/
}

/*
 * An instruction is safe to hoise if either of the following is true:
 *
 * 1. It has no side effects (exceptions/traps). You can use isSafeToSpeculativelyExecute() in <llvm/Analysis/ValueTracking.h>
 *
 * 2. The basic block containing the instruction dominates all exit blocks for the loop. The exit blocks are targets
 *      of exits from the loop, i.e. they are outside the loop.
 */
bool LoopInvariantCodeMotion::safeToHoist(llvm::Instruction *I,
                                          const llvm::LoopInfo &LI,
                                          const llvm::DominatorTree &DT) {

    if(isSafeToSpeculativelyExecute(I)){
       return true;
    }

    // get the BB of the instruction
    BasicBlock *BB = I->getParent();

    // get the loop of the BB of the instruction
    const Loop *L = LI.getLoopFor(BB);

    // todo - what to do in the case for loopless instructions?
    if(!L){
        return false; // don't touch for now
    }

    // obtain exit blocks for the loop
    /*std::vector<BasicBlock*> ExitBlocks;
    for (auto *Succ : successors(BB)) {
        if (L->isLoopExiting(Succ)) {
            ExitBlocks.push_back(Succ);
        }
    }*/

    // Check if the containing basic block dominates all exit blocks
    bool DominatesAllExits = true;
    SmallVector<BasicBlock *, 16> exitBlocks;
    L->getExitBlocks(exitBlocks);
    for (auto *ExitBB : exitBlocks) {
        if (!DT.dominates(BB, ExitBB)) {
            DominatesAllExits = false;
            break;
        }
    }

    return DominatesAllExits;

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
  //errs() << "hey ;)\n";

  // get the basic Loop Information analysis passes
  // (LI for Loop Info)
  auto &LI = FAM.getResult<LoopAnalysis>(F);

  // Code for getting the "loop properties" from part 1
  // (LP for Loop Properties)
  auto &LP = FAM.getResult<LoopPropertiesAnalysis>(F);

  // yoink the dom tree analysis results
  auto &DT = FAM.getResult<DominatorTreeAnalysis>(F);

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
    //errs() << "Analyzing loops of depth = " << currDepth << "\n";

    SmallVector<Instruction*> hoist_victims;
    
    // Here, we are iterating all the LoopProperties objects that have a depth
    // equal to currDepth, using the map that we created earlier
    auto found = LPM.find(currDepth);

    if(found == LPM.end()){
        //errs() << "Couldn't find loop of depth: " << currDepth << '\n';
        continue;
    }

    for (auto &L : found->second) {
      //errs() << "Current Loop:\n";
      //L->print(errs());
      //errs() << "\n";

      // Iterate through all the basic blocks in the loop, L
      for (auto &BB : L->loop->blocks()) {
        
        // Iterate through all instruction in basic block BB
        for (auto &I : *BB) {
          //errs() << "\tCurr inst: ";
          //I.print(errs());
          //errs() << "\n";

          if (!isLoopInvariant(&I, LI) || !safeToHoist(&I, LI, DT)) {
            continue;
          }
          
          // If an instruction is both loop invariant and safe to move, put
          // it either the parent of this loop, or if this is a top level
          // loop (i.e. depth == 0), then put this instruction outside the loop
          // entirely. "Hoist" is just the compiler jargin for lifting code
          // outside the loop. You can think of it as, taking the code, and 
          // carrying it upwards, outside the loop (hence, "hoist").
          //errs() << "\tInstr added to hoisting queue!";
          //errs() << "\n\n";
          hoist_victims.push_back(&I);

          // recompute Dominator Tree in the event we hoist.
          //DT.recalculate(F);


        }
      }
      //errs() << "\n";
    }

    // The hoist_victims vector is an example of the "working list" concept
    // Kayvan mentioned. If we try to edit basic blocks while parsing them,
    // we're gonna have a bad time (don't ask how I know). So, we get a list
    // of instructions that we know will be loop invariant, and then parse
    // said list after we've looked at all the basic blocks we care about.
    // The rest of the code in this loop is so that we can move the line of
    // loop invariant code out of the loop.
    //
    // consider the following LLVM IR:
    // 
    // define i32 @main() #0 {        ; This is block zero
    //   %1 = alloca i32, align 4
    //   %2 = alloca i32, align 4
    //   store i32 0, ptr %1, align 4
    //   br label %3
    //
    // 3:                                                ; preds = %0, %3
    //   store i32 1, ptr %2, align 4
    //   br label %3, !llvm.loop !6
    // }
    //
    // In general, we would want to move store i32 1, ptr %2, align 4 from
    // block %3 to block %0.
    // I know that for this assignment we ignore stores, but this is just a
    // simple example that I think is fairly clear.
    // So, in order to get to block %0, we get the Loop object containing
    // block %3, then get the entry to said loop, and then from that, get the
    // successor to the entry of the loop.
    // That may have made zero sense.
    // If that's the case, track me down to explain it irl.

    bool hoisted = false;
    for (auto &I : hoist_victims) {
      auto L = LI.getLoopFor(I->getParent());
      auto entry_block = L->getHeader()->getPrevNode();
      I->removeFromParent();
      I->insertBefore(&entry_block->back());
      hoisted = true;
    }

    // redo this depth if there was hoisting happening
    if (hoisted) {
        currDepth++;
    }


    //errs() << "\n";
  }

  //errs() << "\nOutput IR of our pass:\n\n";
  //F.print(errs());
  //errs() << "\n\n";

  return PreservedAnalyses::all();
}

// New PM Registration
//-----------------------------------------------------------------------------
PassPluginLibraryInfo getLoopOptPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "mp49774-an35288-Loop-Opt-Pass", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "mp49774-an35288-loop-opt-pass") {
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

