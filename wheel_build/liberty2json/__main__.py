import os
import sys

from . import LIBERTY2JSON_BIN_PATH


def liberty2json():
    os.execl(LIBERTY2JSON_BIN_PATH, "liberty2json", *sys.argv[1:])


if __name__ == "__main__":
    liberty2json()
