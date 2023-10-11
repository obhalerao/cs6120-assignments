llvm-clang -c ../llvm_benchmarks/something_else.c -O0 -S -emit-llvm -o example2.ll
llvm-clang example2.ll
./a.out
