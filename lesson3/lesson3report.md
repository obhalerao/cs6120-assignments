I worked with @OmkarBhalerao on this project.

### Summary

[Trivial Dead Code Elimination](https://github.com/obhalerao/cs6120-assignments/blob/main/lesson3/tdce.cpp)

[Local Value Numbering](https://github.com/obhalerao/cs6120-assignments/blob/main/lesson3/lvn.cpp)

### Details

For this assigment, we changed the C++ JSON parsing library we used from RapidJSON to nlohmann/json, as the latter had much cleaner syntax. We can safely say that we did not regret this decision.

For the trivial dead code elimination assignment, we followed a similar algorithm as the one that was discussed in class: simply iterate backwards through the function, maintain a set of dead variables that are assigned to , and remove them from the set . We iterated until convergence here.

Local value numbering was more tricky. The data structure we chose for the value table was a vector of pairs, the first of which contained a Value representation, and the second of which contained a queue of potential homes for each value, the first of which would be the canonical home. In our haste to write a Value representation, we overlooked how to process constant values, and came up with the admittedly janky solution of reusing the Value representation we made, but putting the value of the constant itself instead of the number of the relevant value. This also means that we do not support processing of float constants; this is something that we can and should easily change. We do, however, support commutativity of relevant operations.

In addition to the value table, we also maintain maps from each variable to its value number and also from each Value itself to its value number, and update these on each iteration of the overall LVN pass. Then, after we update the table for each instruction, we rewrite the args of each instruction with the home of each value, after popping names off the queue corresponding to the value that are not actually its home.

Lastly, after the LVN pass is performed for each basic block and all instructions are rewritten, we perform global DCE (the version that simply removes all unused assignments) until convergence on the entire program, and then the same local DCE implemented previously on all basic blocks.

### Testing

We tested our programs using `brench` on the entirety of the bril benchmark suite, in addition to the bril programs in the `tdce` and `lvn` directories from the bril repo. The results we found were below.

### Difficulties

The primary difficulty we had with this assignment were, as per usual, getting used to C++. However, after working with it for an assignment at this scale, we feel as though we have a better handle of how it works now. In addition, there were many edge cases that we missed on our first pass through (e.g. how to handle numbering of `call` instructions), so we're glad we had the benchmark suite to confirm our suspicions on how to handle these edge cases.