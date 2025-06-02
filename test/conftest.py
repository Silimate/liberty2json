import glob
import os
import pytest
from pathlib import Path


def pytest_configure():
    pytest.test_root = Path(__file__).resolve().parent
    pytest.tests = glob.glob(f"{pytest.test_root}/*.ref.json")
    pytest.l2j = os.getenv(
        "PYTEST_L2J_BIN", pytest.test_root.parent / "build" / "src" / "liberty2json"
    )
