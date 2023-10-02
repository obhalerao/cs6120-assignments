#include "llvm/Pass.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {

struct LessDumbLoopPass : public PassInfoMixin<LessDumbLoopPass>{
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM) {
    for (auto &F : M){
        FunctionAnalysisManager &FAM = AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
        LoopInfo &LI = FAM.getResult<LoopAnalysis>(F);
        for(auto &L: LI){
            errs() << L->getName() << "\n";
        }
    }
    return PreservedAnalyses::all();
  };
};

struct SkeletonPass : public PassInfoMixin<SkeletonPass> {
    PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM) {
        for (auto &F : M) {
            errs() << "I saw a function called " << F.getName() << "!\n";
        }
        return PreservedAnalyses::all();
    };
};

struct SkeletonPass2 : public PassInfoMixin<SkeletonPass> {
    PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM) {
        for (auto &F : M) {
            errs() << "I, too, saw a function called " << F.getName() << "!\n";
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
                    MPM.addPass(SkeletonPass());
                    MPM.addPass(SkeletonPass2());
                    MPM.addPass(LessDumbLoopPass());
                });
        }
    };
}
