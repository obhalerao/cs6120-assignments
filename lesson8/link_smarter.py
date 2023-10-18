#!/usr/bin/env python3

from pathlib import Path
from subprocess import check_output
import shlex
import sys
import os
import logging

logger = logging.getLogger(__name__)
working_dir = Path(__file__).parent

logger.addHandler(logging.FileHandler("/tmp/smart_link_log.txt", "a"))
logger.critical("\n\n\n")
logger.critical(f"working dir is {working_dir}")
logger.critical(f"cwd dir is {os.getcwd()}")
logger.critical(f"name is {__name__}")


if __name__ == '__main__':
    logger.critical("hello world")
    logger.critical(f"args were {sys.argv}")
    logger.critical(f"together were {' '.join(sys.argv)}")
    
    command = f"llvm-clang -fPIE {' '.join(sys.argv[1:])}"
    logger.critical(f"making binary with {command}")
    check_output(
        shlex.split(command),
        text=True
    )