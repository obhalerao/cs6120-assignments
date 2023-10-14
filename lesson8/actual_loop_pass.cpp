#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
using namespace llvm;

namespace {

    struct SkeletonPass : public PassInfoMixin<SkeletonPass> {
        PreservedAnalyses run(Loop &L, LoopAnalysisManager &AM, 
        LoopStandardAnalysisResults &AR, LPMUpdater &U) {
            errs() << "test\n";
            return PreservedAnalyses::all();
        }
    private:
        bool loopInvariant;
    };

}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
    return {
            .APIVersion = LLVM_PLUGIN_API_VERSION,
            .PluginName = "Skeleton pass",
            .PluginVersion = "v0.1",
            .RegisterPassBuilderCallbacks = [](PassBuilder &PB) {
                PB.registerPipelineParsingCallback(
                        [](StringRef name, LoopPassManager &LPM, ...) {
                            LPM.addPass(SkeletonPass());
                            return true;
                        });
            }
    };
}
