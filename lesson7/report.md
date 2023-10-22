I worked with @obhalerao on this project.

## Summary

[Repo Link]()

## Implementation Details

We implemented a simple LLVM pass that logs every time the body of a loop is executed. To do so, we used LLVM's loop analysis to find all the natural loops and inserted a call to a logging utlity to increment the count for that loop. The runtime library is implemented in C++ using a nested `std::unorderedmap`, so its capacity grows dynamically and works for any program that is transformed.

## Testing

We used two benchmarks to test our implementation.

The first one was a contrived C program which had six for-loops, each of whose bodies execute for two iterations. We used this to check the correctness of our implementation (the header of the loop actually gets entered three times, since the condition is checked three times).

The second one was a C++ implementation of Ford-Fulkerson for finding the max flow in a flow network, which has much more interesting control flow. Here is a snippet of the output (the program's output to `stdout` was unchanged):

```
function 4 had its 0th loop header execute 59334722 times
function 206 had its 0th loop header execute 1602 times
function 587 had its 0th loop header execute 80003 times
function 577 had its 0th loop header execute 11336490 times
function 376 had its 0th loop header execute 21 times
function 300 had its 0th loop header execute 41 times
function 329 had its 0th loop header execute 51 times
function 387 had its 0th loop header execute 1020 times
function 256 had its 0th loop header execute 2 times
function 193 had its 0th loop header execute 64082444 times
function 427 had its 0th loop header execute 51 times
function 37 had its 0th loop header execute 801 times
function 37 had its 1th loop header execute 21 times
function 37 had its 3th loop header execute 21 times
function 424 had its 0th loop header execute 51 times
function 101 had its 0th loop header execute 6588 times
function 507 had its 0th loop header execute 80003 times
function 449 had its 0th loop header execute 21 times
function 528 had its 0th loop header execute 80002 times
function 354 had its 0th loop header execute 51 times
function 267 had its 0th loop header execute 2 times
function 460 had its 0th loop header execute 1020 times
```

All the library functions that were included with the implementation of Ford-Fulkerson were also transformed by our pass, which explains why there seem to be far more loops (and functions) in our output than anticipated.

## Difficulties
The main difficulty we encountered was getting the LLVM pass to work properly. In our current version of the code, there is a for-loop through the basic blocks of the function whose body only executes once.

Initially, we implemented the runtime library as a 2D array in C and declared the dimensions of the array as a constant. After learning about C/C++ interoperability, we wrote it in C++ and got it to work pretty easily.

For future work, we think it would be nice to restrict the pass to user-defined functions somehow. We also think it would be fun to write the runtime library in other languages like OCaml, to explore the interoperability further.
