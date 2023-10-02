#include "llvm/Pass.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {

struct LoopCounterPass : public PassInfoMixin<LoopCounterPass>{
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM) {
    for (auto &F : M){
        auto &context = F.getContext();
        IRBuilder builder(&(*(F.begin()->begin())));
        builder.CreateAlloca(Type::getInt32Ty(context), 0, "anti_clobber_counter");
        // auto *ai = new AllocaInst(Type::getInt32Ty(context));
        FunctionAnalysisManager &FAM = AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
        LoopInfo &LI = FAM.getResult<LoopAnalysis>(F);
        for(auto &L: LI){
            auto header = (L->getHeader());
            IRBuilder builder2(&(*(header->begin())));
            // builder2.CreateAdd()
        }
        for(auto &B: F){
          for(auto &I: B){
            errs() << I << "\n";
          }
        }
    }
    return PreservedAnalyses::all();
  };
};

}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
    return {
        .APIVersion = LLVM_PLUGIN_API_VERSION,
        .PluginName = "Skeleton pass",
        .PluginVersion = "v0.1",
        .RegisterPassBuilderCallbacks = [](PassBuilder &PB) {
            PB.registerPipelineStartEPCallback(
                [](ModulePassManager &MPM, OptimizationLevel Level) {
                    MPM.addPass(LoopCounterPass());
                });
        }
    };
}
