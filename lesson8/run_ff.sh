llvm-clang++ -fpass-plugin=`ls ../L8Pass.*` -c ../llvm_benchmarks/ff.cpp -O0 -S -emit-llvm -o example_ff.ll
llvm-clang++ example_ff.ll
./a.out < ../llvm_benchmarks/ffinput.txt
