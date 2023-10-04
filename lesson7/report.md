I worked with @SanjitBasker on this project.

## Summary

[Repo Link]()

## Implementation Details

We implemented a simple LLVM pass that logs every time a loop is ran. To do so, we used LLVM's loop analysis to find all the natural loops and inserted a function call to a logging utlity in a runtime library that updated its count. 

## Testing

We used two benchmarks to test our implementation. The first one was a bit of a contrived example <sanjit please add details>. The second one was an implementation of Ford-Fulkerson for finding the max flow in a directed graph, which has many loops in it that are run many times by nature. The results of the former program were deemed uninteresting, so we report the output of running Ford-Fulkerson below.

<insert images here>

Note that all the library functions that were included with the implementation of Ford-Fulkerson also had their loops counted, which explains why there seem to be far more loops (and functions) in our output than we initially anticipated.

## Difficulties