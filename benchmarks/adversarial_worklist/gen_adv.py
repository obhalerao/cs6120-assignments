from pathlib import Path

# borrowed from CS 4787 bc I'm lazy
global_order_counter = 0
def next_count():
    global global_order_counter
    rv = global_order_counter
    global_order_counter = global_order_counter + 1
    return rv

if __name__ == '__main__':
    for out_path in Path(__file__).parent.glob("*.bril"):
        out_path.unlink()

    for num_blocks in range(0, 16, 2):
        out_path = Path(__file__).parent / f"adversarial_{num_blocks:03d}.bril"

        lines = []
        lines.append("# ARGS: " + " ".join("false" for _ in range(num_blocks)))
        args = ", ".join(f"b{i}: bool" for i in range(num_blocks))
        lines.append(f"@main({args}) " + "{")
        lines.append(f"    jmp .start;")
        lines.append(f"  .end:")
        for i in range(num_blocks):
            lines.append(f"    print x_{i}_2;")
        lines.append(f"    ret;")
        for i in range(num_blocks - 1, -1, -1):
            lines.append(f"  .l_{i}_3:")
            if i == num_blocks - 1:
                lines.append(f"    jmp .end;")
            else:
                lines.append(f"    br b{i + 1} .l_{i + 1}_1 .l_{i + 1}_2;")
            lines.append(f"  .l_{i}_2:")
            lines.append(f"    x_{i}_2 : int = const {next_count()};")
            lines.append(f"    jmp .l_{i}_3;")
            lines.append(f"  .l_{i}_1:")
            lines.append(f"    x_{i}_1 : int = const {next_count()};")
            lines.append(f"    jmp .l_{i}_3;")
        lines.append("  .start:")
        lines.append("    br b0 .l_0_1 .l_0_2;")
        lines.append("}")

        with open(out_path, 'w') as out_file:
            out_file.write("\n".join(lines))