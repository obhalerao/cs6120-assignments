#include "llvm/Pass.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {

struct DumbFunctionPass : public PassInfoMixin<DumbFunctionPass>{
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM) {
    errs() << "HIIIIIIII!!!!!!!!!!!!\n";
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
                    FunctionPassManager FPM;
                    FPM.addPass(DumbFunctionPass());
                    MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM)));
                    MPM.addPass(SkeletonPass());
                    MPM.addPass(SkeletonPass2());
                });
        }
    };
}
