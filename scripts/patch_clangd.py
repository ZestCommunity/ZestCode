#!/usr/bin/env python3
import sys
import os
import subprocess

def main():
    if len(sys.argv) != 2:
        print("Usage: python3 script.py <absolute-path-to-arm-none-eabi-gcc>")
        sys.exit(1)

    gcc_executable = sys.argv[1]

    # Call the gcc executable with --print-sysroot to obtain the sysroot path.
    try:
        output = subprocess.check_output([gcc_executable, "--print-sysroot"], universal_newlines=True)
        sysroot = output.strip()
    except Exception as e:
        print(f"Error: Unable to run '{gcc_executable} --print-sysroot': {e}")
        sys.exit(1)

    # The sysroot is expected to be something like:
    # "C:/Users/.../usr/bin/../arm-none-eabi"
    # The base directory is the parent of the sysroot.
    base_dir = os.path.dirname(sysroot)

    # Build the path to the gcc directory where the version folder resides.
    gcc_dir = os.path.join(base_dir, "lib", "gcc", "arm-none-eabi")
    if not os.path.isdir(gcc_dir):
        print(f"Error: {gcc_dir} is not a valid directory")
        sys.exit(1)

    # Expect exactly one directory inside gcc_dir; that is the gcc version.
    subdirs = [d for d in os.listdir(gcc_dir) if os.path.isdir(os.path.join(gcc_dir, d))]
    if len(subdirs) != 1:
        print(f"Error: Expected exactly one directory in {gcc_dir}, found: {subdirs}")
        sys.exit(1)
    
    gcc_version = subdirs[0]

    # Construct the six output strings.
    outputs = [
        f"-isystem{base_dir}/lib/gcc/arm-none-eabi/{gcc_version}/../../../../arm-none-eabi/include/c++/{gcc_version}",
        f"-isystem{base_dir}/lib/gcc/arm-none-eabi/{gcc_version}/../../../../arm-none-eabi/include/c++/{gcc_version}/arm-none-eabi/thumb/v7-a+simd/softfp",
        f"-isystem{base_dir}/lib/gcc/arm-none-eabi/{gcc_version}/../../../../arm-none-eabi/include/c++/{gcc_version}/backward",
        f"-isystem{base_dir}/lib/gcc/arm-none-eabi/{gcc_version}/include",
        f"-isystem{base_dir}/lib/gcc/arm-none-eabi/{gcc_version}/include-fixed",
        f"-isystem{base_dir}/lib/gcc/arm-none-eabi/{gcc_version}/../../../../arm-none-eabi/include"
    ]

    # Normalize paths to use forward slashes.
    outputs = [path.replace(os.sep, '/') for path in outputs]

    # Print the output strings.
    for line in outputs:
        print(line)

if __name__ == "__main__":
    main()
