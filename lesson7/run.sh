llvm-clang -fpass-plugin=`ls ../build/L7Pass.*` -c ../llvm_benchmarks/something.c -O0 -S -emit-llvm -o example.ll
llvm-clang++ rtlib.cpp -O0 -S -emit-llvm -o rtlib.ll
llvm-clang++ example.ll rtlib.ll
./a.out
