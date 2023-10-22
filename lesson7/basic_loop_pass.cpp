#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
using namespace llvm;

namespace {

    struct SkeletonPass : public PassInfoMixin<SkeletonPass> {
        PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM) {
            int32_t fun_counter = 0;
            for (auto &F : M.functions()) {

                // Get the function to call from our runtime library.
                LLVMContext &Ctx = F.getContext();
                std::vector<Type*> paramTypes = {Type::getInt32Ty(Ctx), Type::getInt32Ty(Ctx)};
                Type *retType = Type::getVoidTy(Ctx);
                FunctionType *logFuncType = FunctionType::get(retType, paramTypes, false);
                FunctionCallee logFunc =
                        F.getParent()->getOrInsertFunction("logop", logFuncType);

                bool done = false;
                if(F.isDeclaration()) continue;
                // for (auto &B : F) {
                //     if (done) continue;


                    FunctionAnalysisManager &FAM = AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
                    LoopInfo &LI = FAM.getResult<LoopAnalysis>(F);

                    int32_t loop_counter = 0;
                    for (auto &L : LI) {
                        auto header = L->getHeader();
                        IRBuilder<> builder(&(header->front()));
                        builder.SetInsertPoint(&(header->back()));

                        auto arg1 = llvm::ConstantInt::get(Ctx, llvm::APInt(32, fun_counter, true));
                        auto arg2 = llvm::ConstantInt::get(Ctx, llvm::APInt(32, loop_counter, true));
                        Value* args[] = {arg1, arg2};
                        builder.CreateCall(logFunc, args);

                        loop_counter++;
                    }



                    if (!paramTypes.empty()) {
                        done = true;
                    }
                // }
                fun_counter++;
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
                            MPM.addPass(SkeletonPass());
                        });
            }
    };
}
