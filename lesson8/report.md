I worked with @obhalerao on this project.

## Summary

[Repo Link]()

## Implementation Details

We implemented an LLVM pass to perform loop-invariant code motion. Since I've implemented this particular optimization on a different IR in CS 4120, we decided to take advantage of LLVM's `makeLoopInvariant` function. We learned how to use `opt` and `llc` to run our passes and get better control over which passes are run. This was important because we need `mem2reg` to run before our pass, to ensure that the variables are stored in temporaries rather than memory locations. This also lets us perform a fair comparison to a baseline where we run the exact same set of passes (except for the pass we wrote).

## Testing

We first ran our code on `embench`, which we supported by writing a custom Python script to emulate the C compiler. It took a really long time to get working because we didn't realize that we had to pass `--relocation-model=pic` to `llc`. We noticed that our speeds were identical but our binaries were differrently sized, so we figured that our optimization was running but just not causing a measurable speedup. To investigate this further, we wrote a very contrived C program in which our optimization hoists a multiplication out of the loop, and we were able to observe a speedup on this:
```
$ source compile_baseline.sh ../llvm_benchmarks/less_dumb_licm.c
$ source compile_licm.sh ../llvm_benchmarks/less_dumb_licm.c
$ time echo "1 2" | ./optimized 
1000000000

real    0m3.412s
user    0m3.410s
sys     0m0.001s
$ time echo "1 2" | ./unoptimized 
1000000000

real    0m3.693s
user    0m3.687s
sys     0m0.007s
```
In order to force the compiler to use a multiplication instruction, we used `scanf` to read one of the arguments from `stdin`--this probably means that I/O contributes very heavily to the runtime. Nevertheless, the speedup is consistent so we consider this strong evidence that our optimization can create observable speedup.

## Difficulties
The main difficulty we encountered was running the LLVM pass.