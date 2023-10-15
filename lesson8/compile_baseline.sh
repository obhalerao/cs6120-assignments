llvm-clang -O1 -mllvm -disable-llvm-optzns $1 -emit-llvm -S -o baseline_old.ll
opt-17 -passes="mem2reg,sroa,early-cse<memssa>,loop-simplify" baseline_old.ll -S -o baseline.ll
llc-17 baseline.ll -o baseline.s
llvm-clang baseline.s -o baseline -no-pie