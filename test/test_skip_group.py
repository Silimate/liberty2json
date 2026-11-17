import json
import subprocess

import pytest


def test_skip_group():
    p = subprocess.run(
        [
            str(pytest.l2j),
            "--skip-group",
            "timing",
            "--skip-group",
            "internal_power",
            str(pytest.test_root / "test_skip_group.lib"),
        ],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        encoding="utf8",
    )
    assert p.returncode == 0, "skip-group test did not exit cleanly"
    assert json.loads(p.stdout) == {
        "library": {
            "groups": [
                {
                    "lu_table_template": {
                        "index_1": ["0.1, 0.2"],
                        "variable_1": "input_net_transition",
                        "names": ["delay_template"],
                    }
                },
                {
                    "cell": {
                        "groups": [
                            {
                                "pin": {
                                    "groups": [
                                        {
                                            "hyperbolic_noise_low": {
                                                "height_coefficient": 0.5
                                            }
                                        }
                                    ],
                                    "capacitance": 0.25,
                                    "direction": "input",
                                    "names": ["A"],
                                }
                            },
                            {
                                "pin": {
                                    "direction": "output",
                                    "function": "!A",
                                    "max_capacitance": 0.5,
                                    "names": ["Y"],
                                }
                            },
                            {"leakage_power": {"value": 0.125}},
                        ],
                        "area": 1,
                        "names": ["INV"],
                    }
                },
            ],
            "names": ["skip_group_tests"],
        }
    }, "unexpected stdout"
