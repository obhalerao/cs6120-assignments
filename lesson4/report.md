

Summary
In this assignment @SanjitBasker and I implemented a dataflow analysis framework and instantiated it on two dataflow analyses. We continued to use C++ for the assignment. We implemented the formation of CFGs (which is an extension of the basic block forming algorithm we used in the last assignment) and then wrote a template class for the dataflow analysis. We had some trouble getting the main program to take advantage of our template class, but it can be instantiated through a simple interface in which one provides the meet and transfer functions (and also a utility function for making "top" values with which to initialize the analysis). The analysis can proceed in a forwards or backwards direction, but we realized only after implementing the solver that it may have been a cleaner design to make this a template parameter.

Testing
In order to validate our implementation, we implemented analyses for constant propogation and defined variables. We then visualized our results by generating graphs in DOT. This was a bit painful because we couldn't find any C++ libraries for graphs that give full control over the rendered labels. So we decided to generate the graphs ourselves, which we visualized in Edotor.

For fun, we also implemented a DFS-based traversal of the CFG and used this to solve the dataflow equations: when we discussed this topic in CS 4120, we learned that solving the strongly connected components of the graph in order will result in faster convergence than a naive worklist, since nodes are solved after their dependencies. In both implementations, we measured the number of times that a node was taken off the worklist and processed (this is equivalent to counting the invocations of the meet function). This was the measure of "performance" for our `brench` benchmark, with the correctness being a text report of the dataflow results. Our results are hopeful, but it seems that in some CFGs the naive worklist can outperform the DFS-based solver. We are happy to report that the results are the same regardless of the solver.

Below are some pretty charts detailing the results of our two solvers on our first analysis, defined variables:

And here are some charts detailing their results on our second analysis, constant propagation: