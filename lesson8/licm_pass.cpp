#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/CodeGen/ReachingDefAnalysis.h"
using namespace llvm;

// because LLVM is in SSA form, all we have to check is if a variable's definition
// is outside of the loop.
// so we don't need reaching defs at all, but we still need to iterate to convergence.

namespace {

    struct SkeletonPass : public PassInfoMixin<SkeletonPass> {
        PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM) {
            int32_t fun_counter = 0;
            errs() << "hi\n";
            for (auto &F : M.functions()) {

                // Get the function to call from our runtime library.
                LLVMContext &Ctx = F.getContext();
                std::vector<Type*> paramTypes = {Type::getInt32Ty(Ctx), Type::getInt32Ty(Ctx)};
                Type *retType = Type::getVoidTy(Ctx);
                FunctionType *logFuncType = FunctionType::get(retType, paramTypes, false);
                FunctionCallee logFunc =
                        F.getParent()->getOrInsertFunction("logop", logFuncType);

                if(F.isDeclaration()) continue;
                FunctionAnalysisManager &FAM = AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
                LoopAnalysisManager &LAM = FAM.getResult<LoopAnalysisManagerFunctionProxy>(F).getManager();
                
                LoopInfo &LI = FAM.getResult<LoopAnalysis>(F);
                DominatorTree &DT = FAM.getResult<DominatorTreeAnalysis>(F);


                int idx = 0;
                for (auto &L : LI) {
                    idx++;
                    std::set<Instruction*> loop_invars;
                    auto header = L->getHeader();
                    for(auto &BB : L->getBlocks()){
                      for(auto &I: *BB){
                        errs() << I << "\n";
                        if(L->hasLoopInvariantOperands(&I)){
                          bool changed = false;
                          L->makeLoopInvariant(&I, changed);
                        }
                      }
                    }
                }

            }
            return PreservedAnalyses::all();
        }
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
                            MPM.addPass(createModuleToFunctionPassAdaptor(LoopSimplifyPass()));
                            MPM.addPass(SkeletonPass());
                            
                        });
            }
    };
}
