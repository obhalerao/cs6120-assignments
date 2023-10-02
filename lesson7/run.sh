`brew --prefix llvm`/bin/clang -fpass-plugin=`ls ../build/L7Pass.*` -c ../c_benchmarks/something.c -o example.o
cc -c rtlib.c -o rtlib.o
cc example.o rtlib.o
./a.out