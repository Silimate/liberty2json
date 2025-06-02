import re
import json
import shlex
import pytest
import logging
import functools
import subprocess
from types import NoneType
from itertools import zip_longest
from typing import Dict, List, Optional, Union

whitespace_rx = re.compile(r"(\n|\s|\t|\\\n)+")

JsonTypes = Union[Dict, List, float, int, str, bool, NoneType]


def deep_compare(lhs: JsonTypes, rhs: JsonTypes):
    """
    This is a custom, slightly lenient deep-dict comparator for JSON dicts.
    
    Compared to a more standard comparator:
    
    * floats and ints are all coerced to floats for comparison purposes.
        * tolerance of 1% exists to account for different handling of floating
          point units.
    * strings with commas in them (dotlib files' excuse for arrays) are compared
      with whitespace normalized.
    * "false", False, and 0 are equivalent, and "true", True, and 1 are
      equivalent.
    * strings with +, -, * or /, i.e. expressions, are currently not compared
    """
    if isinstance(lhs, List):
        result = {}
        if not isinstance(rhs, List):
            return f"expected list got {type(rhs)}"
        for i, (el0, el1) in enumerate(zip_longest(lhs, rhs, fillvalue=None)):
            if el0 is not None and el1 is not None:
                result_i = deep_compare(el0, el1)
                if result_i is not None:
                    result[i] = result_i
            elif el0 is not None and el1 is None:
                result[key] = f"expected entry missing"
            elif el0 is None and el1 is not None:
                result[key] = f"extra entry"
        return result if len(result) else None
    elif isinstance(lhs, Dict):
        result = {}
        if not isinstance(rhs, Dict):
            return f"expected dict got {type(rhs)}"
        all_keys = set(lhs.keys()).union(rhs.keys())
        for key in all_keys:
            if key in lhs and key in rhs:
                result_key = deep_compare(lhs[key], rhs[key])
                if result_key is not None:
                    result[key] = result_key
            elif key in lhs and key not in rhs:
                result[key] = f"expected key missing"
            else:
                result[key] = f"extra key"
        return result if len(result) else None

    valid_mixed_type_sets = {
        frozenset({"false", 0}),
        frozenset({"false", False}),
        frozenset({"true", 1}),
        frozenset({"true", True}),
        frozenset({1, True}),
        frozenset({0, False}),
    }
    if frozenset({lhs, rhs}) in valid_mixed_type_sets:
        return None

    if isinstance(lhs, str):
        if functools.reduce(lambda acc, c: acc or c in lhs, ["+", "*", "-", "/"]):
            return None  # cant compare expressions rn
        if not isinstance(rhs, str):
            return f"expected str got {type(rhs)}"
        if "," in lhs or "," in rhs:
            lhs = whitespace_rx.sub("", lhs)
            rhs = whitespace_rx.sub("", rhs)
        return None if lhs == rhs else {"expected": lhs, "got": rhs}
    elif isinstance(lhs, int) or isinstance(lhs, float):
        if not isinstance(rhs, int) and not isinstance(rhs, float):
            return f"expected number got {type(rhs)}"
        if lhs == rhs:
            return None
        elif lhs != 0:
            lhs = float(lhs)
            rhs = float(rhs)
            difference = abs(lhs - rhs) / lhs
            return None if difference <= 0.01 else {"expected": lhs, "got": rhs}
        else:
            return None if lhs == rhs else {"expected": lhs, "got": rhs}
    elif isinstance(lhs, bool):
        if not isinstance(rhs, bool):
            return f"expected bool got {type(rhs)}"
        return None if lhs == rhs else {"expected": lhs, "got": rhs}
    else:
        return None


def flatten(dict_in: dict, dict_out: Optional[dict] = None, keypath=""):
    if dict_out is None:
        dict_out = {}
    for key, value in dict_in.items():
        new_keypath = keypath + "/" + str(key)
        if isinstance(value, dict):
            flatten(value, dict_out, new_keypath)
        else:
            dict_out[new_keypath] = value
    return dict_out


@pytest.mark.parametrize("au", pytest.tests)
def test_lib(au: str):
    dotlib = au.replace(".ref.json", ".lib")
    got = au.replace(".ref.json", ".test.json")
    result = subprocess.run(
        [str(pytest.l2j), "--outfile", got, dotlib],
        stderr=subprocess.STDOUT,
        encoding="utf8",
    )
    assert (
        result.returncode == 0
    ), f"subprocess {shlex.join(result.args)} failed unexpectedly"
    with open(au, encoding="utf8") as f:
        au_parsed = json.load(f)
    with open(got, encoding="utf8") as f:
        got_parsed = json.load(f)
    difference = deep_compare(au_parsed, got_parsed)
    diff_json = au.replace(".ref.json", ".diff.json")
    with open(diff_json, "w") as f:
        json.dump(difference, fp=f)
    if difference is not None:
        flattened = flatten(difference)
        all_extra = functools.reduce(lambda acc, el: acc and "extra" in el, flattened.values(), True)
        if all_extra:
            logging.warning(f"Differences found in {got}, but all extra")
            return
    assert difference is None, f"Differences found -- {diff_json}"
