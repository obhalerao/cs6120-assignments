#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {

    struct LessDumbLoopPass : public PassInfoMixin<LessDumbLoopPass> {
        static bool isRequired() { return true; }

        PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM) {
            int32_t fun_counter = 0;

            LLVMContext &Ctx = M.getContext();
            std::vector<Type *> paramTypes = {Type::getInt32Ty(Ctx), Type::getInt32Ty(Ctx)};
            Type *retType = Type::getVoidTy(Ctx);
            FunctionType *logFuncType = FunctionType::get(retType, paramTypes, false);
            FunctionCallee logFunc =
                M.getOrInsertFunction("logop", logFuncType);


            for (auto &F: M.functions()) {

                FunctionAnalysisManager &FAM = AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
                LoopInfo &LI = FAM.getResult<LoopAnalysis>(F);

                int32_t loop_counter = 0;
                for (auto &L: LI) {
                    errs() << L->getName() << "\n";

                    // auto header = L->getHeader();
                    // IRBuilder<> builder(&(header->front()));
                    // builder.SetInsertPoint(&(header->front()));

                    // auto arg1 = llvm::ConstantInt::get(Ctx, llvm::APInt(32, fun_counter, true));
                    // auto arg2 = llvm::ConstantInt::get(Ctx, llvm::APInt(32, loop_counter, true));
                    // Value* args[] = {arg1, arg2};
                    // builder.CreateCall(logFunc, args);
                    loop_counter++;
                }
                fun_counter++;
            }
            return PreservedAnalyses::none();
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
                PB.registerOptimizerLastEPCallback(
                        [](ModulePassManager &MPM, OptimizationLevel Level) {
                            MPM.addPass(LessDumbLoopPass());
                        });
            }
    };
}
