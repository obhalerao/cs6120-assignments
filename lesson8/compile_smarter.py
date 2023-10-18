#!/usr/bin/env python3

from pathlib import Path
from subprocess import check_output
import shlex
import os
import sys
import logging

logger = logging.getLogger(__name__)
working_dir = Path(__file__).parent

logger.addHandler(logging.FileHandler("/tmp/smart_log.txt", "a"))
logger.critical("\n\n\n")
logger.critical(f"working dir is {working_dir}")
logger.critical(f"cwd dir is {os.getcwd()}")
logger.critical(f"name is {__name__}")


if __name__ == '__main__':
    logger.critical("hello world")
    logger.critical(f"args were {sys.argv}")
    logger.critical(f"together were {' '.join(sys.argv)}")
    
    with open(working_dir / "arg_dump.txt", 'a') as arg_file:
        arg_file.write(str(sys.argv) + "\n")
    
    skips = set()
    input_files = []
    output = None
    other_words = ['-fPIE']
    includes = []
    licm = False

    for i, word in enumerate(sys.argv):
        logger.critical(f"processing word {repr(word)}, which has? -o in it: {'-o' in word}")
        if word == '--licm':
            licm = True
            continue

        if i == 0 or '-O' in word or i in skips:
            continue

        if "-D" in word and '=' not in word:
            other_words.append(word)
        elif "-D" in word:
            other_words.append(word)
        elif '-I' in word:
            includes.append(word)
        elif '-o' in word:
            output = sys.argv[i + 1]
            skips.add(i + 1)
        elif '-f' in word:
            other_words.append(word)
        elif i not in skips:  # input files
            input_files.append(word)
    
    command = f"llvm-clang -O0 -mllvm -disable-llvm-optzns {' '.join(other_words)} {' '.join(includes)} {' '.join(input_files)} -emit-llvm -S -o /tmp/opt0.ll"
    logger.critical(f"making opt0 with {command}")
    logger.critical(check_output(
        shlex.split(command),
        text=True
    ))
    
    command = f"/home/sb866/llvm-cpp/llvm-project-17.0.1.src/build/bin/opt -passes=\"mem2reg,sroa,early-cse<memssa>,loop-simplify\" /tmp/opt0.ll -S -o /tmp/opt1.ll"
    logger.critical(f"making opt1 with {command}")
    check_output(
        shlex.split(f"/home/sb866/llvm-cpp/llvm-project-17.0.1.src/build/bin/opt -passes=\"mem2reg,sroa,early-cse<memssa>,loop-simplify\" /tmp/opt0.ll -S -o /tmp/opt1.ll"),
        text=True
    )

    if licm:
        command = f"/home/sb866/llvm-cpp/llvm-project-17.0.1.src/build/bin/opt -load-pass-plugin /home/sb866/fall-23/cs6120/omkar-repo/build/L8Pass.so -passes=\"custom_licm\" /tmp/opt1.ll -o /tmp/opt2.ll -S"
    else:
        command = f"cp /tmp/opt1.ll /tmp/opt2.ll"
    logger.critical(f"making opt2 with {command}")
    check_output(
        shlex.split(command),
        text=True
    )
    
    command = f"/home/sb866/llvm-cpp/llvm-project-17.0.1.src/build/bin/llc /tmp/opt2.ll -o /tmp/opt2.s --relocation-model=pic"
    logger.critical(f"making s with {command}")
    check_output(
        shlex.split(command),
        text=True
    )
    
    command = f"llvm-clang -c /tmp/opt2.s {' '.join(includes)} {' '.join(other_words)} -o {output}"
    logger.critical(f"making binary with {command}")
    check_output(
        shlex.split(command),
        text=True
    )