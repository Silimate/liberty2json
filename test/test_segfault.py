import json
import subprocess

import pytest


def test_segfault():
    p = subprocess.run(
        [str(pytest.l2j), str(pytest.test_root / "test_segfault.lib")],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        encoding="utf8",
    )
    assert p.returncode == 0, "segfault test did not exit cleanly"
    assert (
        "Malformed define_group on line 2: no arguments provided" in p.stderr
    ), "define_group with no arguments did not emit a warning"
    assert (
        "Malformed define_group on line 3: less than two arguments provided" in p.stderr
    ), "define_group with one argument did not emit a warning"
    assert json.loads(p.stdout) == {
        "library": {"really_complex_attribute": [], "names": ["segfault_tests"]}
    }, "unexpected stdout"
