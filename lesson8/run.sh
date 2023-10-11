llvm-clang -fpass-plugin=`ls ../L8Pass.*` -c ../llvm_benchmarks/something_else.c -O0 -S -emit-llvm -o example.ll
llvm-clang example.ll
./a.out
