mkdir -p baseline
llvm-clang -O1 -mllvm -disable-llvm-optzns $1 -emit-llvm -S -o baseline/baseline_0.ll
/home/sb866/llvm-cpp/llvm-project-17.0.1.src/build/bin/opt -passes="mem2reg,sroa,early-cse<memssa>,loop-simplify" baseline/baseline_0.ll -S -o baseline/baseline.ll
/home/sb866/llvm-cpp/llvm-project-17.0.1.src/build/bin/llc --x86-asm-syntax=intel -O=0 baseline/baseline.ll -o baseline/baseline.s
llvm-clang baseline/baseline.s -o unoptimized -no-pie