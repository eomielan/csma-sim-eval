import os
import subprocess

import pytest


@pytest.fixture(scope="module", autouse=True)
def change_dir():
    # Chage directory to root of project before tests
    os.chdir("../..")
    yield


@pytest.mark.parametrize(
    "input_filename, expected_output_data",
    [
        ("src/test/test_input1.txt", "0.40"),
        ("src/test/test_input2.txt", "0.55"),
        ("src/test/test_input3.txt", "0.43"),
        ("src/test/test_input4.txt", "0.80"),
        ("src/test/test_input5.txt", "1.00"),
    ],
)
def test_csma(input_filename, expected_output_data):
    simulation_process = subprocess.Popen(["./csma", input_filename])

    simulation_process.wait()

    with open("output.txt", "r") as output_file:
        output_data = output_file.read().strip()

    assert output_data == expected_output_data


if __name__ == "__main__":
    pytest.main(["-v"])
