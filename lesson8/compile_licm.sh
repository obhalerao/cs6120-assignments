llvm-clang -O1 -mllvm -disable-llvm-optzns $1 -emit-llvm -S -o baseline_old.ll
opt-17 -passes="mem2reg,sroa,early-cse<memssa>,loop-simplify" baseline_old.ll -S -o baseline.ll
opt-17 -load-pass-plugin ../build/L8Pass.so \
-passes="custom_licm" baseline.ll -o optimized.ll -S
llc-17 optimized.ll -o optimized.s
llvm-clang optimized.s -o optimized -no-pie