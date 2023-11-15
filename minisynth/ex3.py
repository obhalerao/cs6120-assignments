import ex2
import sys
import lark

import itertools
import pprint

parser = lark.Lark(ex2.GRAMMAR)

def put_1_hole(source):
    parts = source.split('??')
    out = []
    for (i, part) in enumerate(parts[:-1]):
        out.append(part)
        out.append((f"(1)"))
    out.append(parts[-1])
    return ''.join(out)

def desugar_hole(source, free_vars):
    choose_var_template = '+'.join(
        itertools.chain(
            (f'(hbool_{i}_{{0}} ? {var} : 0)' for i, var in enumerate(free_vars)),
            ['hconst_{0}']
        )
    )

    parts = source.split('??')
    out = []
    for (i, part) in enumerate(parts[:-1]):
        out.append(part)
        out.append(f'({choose_var_template})'.format(i))
    out.append(parts[-1])
    return ''.join(out)


def simplify(tree, subst={}):
    op = tree.data

    if op in ('add', 'sub', 'mul', 'div', 'shl', 'shr', 'neg', 'if'):
        for i in range(len(tree.children)):
            tree.children[i] = simplify(tree.children[i], subst)

    if op == 'if':
        cond = tree.children[0]
        if cond.data == 'var':
            name = cond.children[0]
            if name in subst:
                val = subst[name]
                if val.as_signed_long():
                    return tree.children[1]
                else:
                    return tree.children[2]
    elif op == 'add':
        left = tree.children[0]
        right = tree.children[1]
        if left.data == 'var' and left.children[0] in subst and subst[left.children[0]].as_signed_long() == 0:
            return right
        elif right.data == 'var' and right.children[0] in subst and subst[right.children[0]].as_signed_long() == 0:
            return left
        elif right.data == 'num' and int(right.children[0]) == 0:
            return left
        elif left.data == 'num' and int(left.children[0]) == 0:
            return right
    elif op == 'shl':
        left = tree.children[0]
        right = tree.children[1]
        if right.data == 'var' and right.children[0] in subst and subst[right.children[0]].as_signed_long() == 0:
            return left
        elif right.data == 'num' and int(right.children[0]) == 0:
            return left
    return tree


def ex3(source):
    src1, src2 = source.strip().split('\n')
    free_vars = ex2.z3_expr(parser.parse(src1))[1].keys()

    src2 = desugar_hole(src2, free_vars)  # Allow ?? in the sketch part.

    tree1 = parser.parse(src1)
    tree2 = parser.parse(src2)


    model = ex2.synthesize(tree1, tree2)
    print(ex2.pretty(tree1))
    # print(ex2.pretty(tree2))

    values = ex2.model_values(model)
    # print(sorted(values.items()))
    simplify(tree2, values)  # Remove foregone conclusions.
    print(ex2.pretty(tree2, values))


if __name__ == '__main__':
    ex3(sys.stdin.read())
