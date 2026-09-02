# Copyright (C) 2026 Silimate Inc.
#
# Written by Mohamed Gaber <me@donn.website>
#
# Adapted from Yosys
#
# Copyright (C) 2026 Catherine <whitequark@whitequark.org>
#
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
import datetime
import io
import os
import pathlib
import tarfile
import tempfile
import sysconfig
import subprocess
import hashlib
import urllib.request
from email.policy import EmailPolicy
from email.message import EmailMessage
from typing import Tuple, Iterable, Optional
from wheel.wheelfile import WheelFile

PROJECT_NAME = "liberty2json"
PROJECT_VERSION = os.getenv("LIBERTY2JSON_WHEEL_VERSION")
if PROJECT_VERSION is None:
    try:
        PROJECT_VERSION = (
            pathlib.Path("SDIST_VERSION").read_text(encoding="ascii").strip()
        )
    except FileNotFoundError:
        PROJECT_VERSION = datetime.datetime.now().strftime("%Y.%m.%d")
DIST_NAME = f"{PROJECT_NAME}-{PROJECT_VERSION}"

PLATFORM_TAG_RAW = sysconfig.get_platform()
PLATFORM_TAG = (
    PLATFORM_TAG_RAW.lower().replace("-", "_").replace(".", "_").replace(" ", "_")
)
COMPAT_TAG = f"py3-none-{PLATFORM_TAG}"

# python uses ENTRY_POINTS in metadata to synthesize entries in ./venv/bin
ENTRY_POINTS = f"""
[console_scripts]
liberty2json = {PROJECT_NAME}.__main__:liberty2json
"""

# downloadable deps
CUDD_URL = "https://github.com/silimate/cudd/archive/cudd-3.0.0.tar.gz"
CUDD_SHA256 = "5fe145041c594689e6e7cf4cd623d5f2b7c36261708be8c9a72aed72cf67acce"


def build_sdist(sdist_dir, config_settings=None):
    sdist_filename = f"{DIST_NAME}.tar.gz"

    with tarfile.open(
        pathlib.Path(sdist_dir) / sdist_filename,
        "w:gz",
        format=tarfile.PAX_FORMAT,
    ) as sdist:
        version_bytes = PROJECT_VERSION.encode("ascii")
        version_tarinfo = tarfile.TarInfo(f"{DIST_NAME}/SDIST_VERSION")
        version_tarinfo.size = len(version_bytes)
        sdist.addfile(version_tarinfo, io.BytesIO(version_bytes))

        def exclude_build(entry):
            name = os.path.basename(entry.name)
            if name in (
                ".git",
                ".github",
                ".cache",
                "build",
                "dist",
                "venv",
                ".venv",
                "test",
                "__pycache__",
            ):
                return
            if (
                name.endswith(".whl")
                or name.endswith(".tgz")
                or name.endswith(".tar.gz")
            ):
                return
            return entry

        sdist.add(os.getcwd(), arcname=DIST_NAME, filter=exclude_build)

    return sdist_filename


def make_message(headers: Iterable[Tuple[str, str]], payload: Optional[str] = None):
    """
    converts a set of python tuples and an optional payload in a manner
    consistent with
    https://packaging.python.org/en/latest/specifications/core-metadata/#core-metadata
    """
    msg = EmailMessage(policy=EmailPolicy(max_line_length=0))
    for name, value in headers:
        if isinstance(value, list):
            for value_part in value:
                msg[name] = value_part
        else:
            msg[name] = value
    if payload:
        msg.set_payload(payload)
    return bytes(msg)


def get_metadata_files():
    """
    (see https://packaging.python.org/en/latest/specifications/recording-installed-packages/)
    """
    with open("README.md", "rb") as readme:
        long_description = readme.read()

    return {
        "WHEEL": make_message(
            [
                ("Wheel-Version", "1.0"),
                ("Generator", "custom silimate liberty2json build backend"),
                ("Root-Is-Purelib", "false"),
                ("Tag", [COMPAT_TAG]),
            ]
        ),
        "METADATA": make_message(
            [
                ("Metadata-Version", "2.4"),
                ("Name", PROJECT_NAME),
                ("Version", PROJECT_VERSION),
                (
                    "Summary",
                    "Convert Liberty files to JSON using OpenSTA's liberty parser",
                ),
                ("Description-Content-Type", "text/markdown"),
                ("Classifier", "Programming Language :: Python :: 3"),
                ("Requires-Python", ">=3.8"),
                ("License", "MIT"),
            ],
            long_description,
        ),
        "entry_points.txt": ENTRY_POINTS.encode("utf8"),
    }


def prepare_metadata_for_build_wheel(metadata_directory, config_settings=None):
    """
    top-level function (called by pip during wheel build)

    generates dist-info
    """
    os.mkdir(f"{metadata_directory}/{DIST_NAME}.dist-info")

    for filename, contents in get_metadata_files().items():
        with open(f"{metadata_directory}/{DIST_NAME}.dist-info/{filename}", "wb") as f:
            f.write(contents)

    return f"{DIST_NAME}.dist-info"


def _ensure_cudd(d):
    cudd_result = subprocess.check_output(
        ["cmake", "-P", "third_party/opensta/cmake/FindCUDD.cmake"],
        encoding="utf8",
    )
    if "not found" not in cudd_result:
        return None

    urllib.request.urlretrieve(CUDD_URL, d / "cudd.tar.gz")
    with open(d / "cudd.tar.gz", "rb") as f:
        buffer = f.read()
        sha256 = hashlib.sha256()
        sha256.update(buffer)
        got = sha256.hexdigest()
        if CUDD_SHA256 != got:
            raise RuntimeError(f"upstream hash for cudd source changed: {got}")

    with tarfile.open(d / "cudd.tar.gz", mode="r:gz") as tf:
        tf.extractall(d / "cudd-src")

    subprocess.check_call(
        ["./configure", f"--prefix={d / 'cudd'}"],
        cwd=d / "cudd-src" / "cudd-cudd-3.0.0",
    )
    subprocess.check_call(
        ["make", f"-j{os.cpu_count()}"],
        cwd=d / "cudd-src" / "cudd-cudd-3.0.0",
    )
    subprocess.check_call(
        ["make", "install"],
        cwd=d / "cudd-src" / "cudd-cudd-3.0.0",
    )
    return d / "cudd"


def build_wheel(wheel_dir, config_settings=None, metadata_directory=None):
    """
    top-level function (called by wheel build)

    builds liberty2json and creates python version-agnostic wheel
    """
    wheel_filename = f"{DIST_NAME}-{COMPAT_TAG}.whl"

    with WheelFile(pathlib.Path(wheel_dir) / wheel_filename, "w") as wheel:
        # write metadata
        for filename, contents in get_metadata_files().items():
            wheel.writestr(f"{DIST_NAME}.dist-info/{filename}", contents)

        # build in temporary directory
        with tempfile.TemporaryDirectory(f".{PROJECT_NAME}-build", "w") as d_str:
            d = pathlib.Path(d_str)

            cmake_prefix_path = os.getenv("CMAKE_PREFIX_PATH", "")

            # build cudd if it cannot be found
            if cudd_prefix := _ensure_cudd(d):
                cmake_prefix_path = f"{cudd_prefix}:{cmake_prefix_path}"

            # copy python files
            wheel.write(
                "wheel_build/liberty2json/__init__.py", f"{PROJECT_NAME}/__init__.py"
            )
            wheel.write(
                "wheel_build/liberty2json/__main__.py", f"{PROJECT_NAME}/__main__.py"
            )

            # configure
            env = os.environ.copy()
            env["CMAKE_PREFIX_PATH"] = cmake_prefix_path
            subprocess.check_call(
                [
                    "cmake",
                    "-B",
                    d,
                    ".",
                ],
                env=env,
            )

            # build
            subprocess.check_call(
                [
                    "cmake",
                    "--build",
                    d,
                    f"-j{os.cpu_count()}",
                    "--target", "liberty2json",
                ]
            )

            # copy binary to same location as __main__.py
            wheel.write(d / "liberty2json", f"{PROJECT_NAME}/liberty2json")

    return wheel_filename
