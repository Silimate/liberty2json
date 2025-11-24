import os
import subprocess
import filecmp
from pathlib import Path

TEST_DIR = Path(__file__).resolve().parent
LIBERTY2JSON_EXE = os.getenv("REF_L2J_PATH", TEST_DIR.parent / "build" / "liberty2json")


def check_liberty_json(json_filename):
    """Load a Liberty JSON file"""
    with open(json_filename, encoding="utf-8") as json_file:
        json_contents = json_file.read()
        if '"cell_rise":' in json_contents or '"cell_fall":' in json_contents:
            print("  NLDM timing found in file:", json_filename)
        if (
            '"output_current_rise":' in json_contents
            or '"output_current_fall":' in json_contents
        ):
            print("  CCS timing found in file:", json_filename)


def create_reference_files():
    for file_name in os.listdir(TEST_DIR):
        if file_name.endswith(".lib"):
            lib_file = os.path.join(TEST_DIR, file_name)
            ref_file = lib_file.replace(".lib", ".ref.json")

            # Run liberty2json on the .lib file to create the reference JSON file
            subprocess.run(
                [
                    LIBERTY2JSON_EXE,
                    os.path.relpath(lib_file),
                    "--src",
                    "--outfile",
                    ref_file,
                ]
            )


if __name__ == "__main__":
    create_reference_files()
