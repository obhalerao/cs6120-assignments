mkdir -p licm
llvm-clang -O1 -mllvm -disable-llvm-optzns $1 -emit-llvm -S -o licm/baseline_0.ll
/home/sb866/llvm-cpp/llvm-project-17.0.1.src/build/bin/opt -passes="mem2reg,sroa,early-cse<memssa>,loop-simplify" licm/baseline_0.ll -S -o licm/baseline.ll
/home/sb866/llvm-cpp/llvm-project-17.0.1.src/build/bin/opt -load-pass-plugin /home/sb866/fall-23/cs6120/omkar-repo/build/L8Pass.so \
-passes="custom_licm" licm/baseline.ll -o licm/optimized.ll -S
/home/sb866/llvm-cpp/llvm-project-17.0.1.src/build/bin/llc --x86-asm-syntax=intel -O=0 licm/optimized.ll -o licm/optimized.s
llvm-clang licm/optimized.s -o optimized -no-pie