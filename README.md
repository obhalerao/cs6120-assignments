# cs6120-assignments
Assignments for CS 6120: Advanced Compilers. Uses the [nlohmann/json](https://json.nlohmann.me/) library to parse JSON files.

# Lesson 2
Counts number of branch instructions in JSON representation of bril file.

To run, execute

`cmake . && make l2 && ./l2 < <filename>`

in your terminal.

# Lesson 3
Runs trivial dead code elimination and local value numbering in JSON representation of bril file.

To run execute

`cmake . && make {target} && ./{target} < <filename>`

in your terminal, where `{target}` is `l3_tdce` for trivial dead code elimination and `l3_lvn` for local value numbering.
