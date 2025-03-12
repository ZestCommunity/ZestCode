#!/usr/bin/env python3
import sys
import os
import subprocess
import re

def main():
    if len(sys.argv) != 2:
        print("Usage: python3 script.py <absolute-path-to-arm-none-eabi-gcc>")
        sys.exit(1)

    gcc_executable = sys.argv[1]

    # Obtain sysroot by calling `arm-none-eabi-gcc --print-sysroot`
    try:
        output = subprocess.check_output([gcc_executable, "--print-sysroot"], universal_newlines=True)
        sysroot = output.strip()
    except Exception as e:
        print(f"Error: Unable to run '{gcc_executable} --print-sysroot': {e}")
        sys.exit(1)

    # The base directory is the parent of the sysroot.
    base_dir = os.path.dirname(sysroot)

    # Detect the GCC version folder automatically.
    gcc_dir = os.path.join(base_dir, "lib", "gcc", "arm-none-eabi")
    if not os.path.isdir(gcc_dir):
        print(f"Error: {gcc_dir} is not a valid directory")
        sys.exit(1)

    subdirs = [d for d in os.listdir(gcc_dir) if os.path.isdir(os.path.join(gcc_dir, d))]
    if len(subdirs) != 1:
        print(f"Error: Expected exactly one directory in {gcc_dir}, found: {subdirs}")
        sys.exit(1)
    
    gcc_version = subdirs[0]

    # Build the six output strings.
    outputs = [
        f"-isystem{base_dir}/lib/gcc/arm-none-eabi/{gcc_version}/../../../../arm-none-eabi/include/c++/{gcc_version}",
        f"-isystem{base_dir}/lib/gcc/arm-none-eabi/{gcc_version}/../../../../arm-none-eabi/include/c++/{gcc_version}/arm-none-eabi/thumb/v7-a+simd/softfp",
        f"-isystem{base_dir}/lib/gcc/arm-none-eabi/{gcc_version}/../../../../arm-none-eabi/include/c++/{gcc_version}/backward",
        f"-isystem{base_dir}/lib/gcc/arm-none-eabi/{gcc_version}/include",
        f"-isystem{base_dir}/lib/gcc/arm-none-eabi/{gcc_version}/include-fixed",
        f"-isystem{base_dir}/lib/gcc/arm-none-eabi/{gcc_version}/../../../../arm-none-eabi/include"
    ]

    # Normalize paths to use forward slashes.
    outputs = [s.replace(os.sep, '/') for s in outputs]

    # Locate the .clangd file in the parent directory of this script.
    script_dir = os.path.dirname(os.path.abspath(__file__))
    parent_dir = os.path.dirname(script_dir)
    clangd_path = os.path.join(parent_dir, ".clangd")
    if not os.path.isfile(clangd_path):
        print(f"Error: {clangd_path} does not exist.")
        sys.exit(1)
    
    # Read the existing .clangd content.
    with open(clangd_path, 'r') as f:
        content = f.read()

    # Replace the contents of the "Add" section with the new output strings.
    # The regex matches from "Add: [" to the next closing bracket "]" (using DOTALL for multi-line matching).
    pattern = r"(Add:\s*\[)[^\]]*(\])"
    def repl(match):
        new_items = "\n    " + ",\n    ".join([f'"{item}"' for item in outputs]) + "\n  "
        return f"{match.group(1)}{new_items}{match.group(2)}"
    
    new_content, count = re.subn(pattern, repl, content, flags=re.DOTALL)
    if count == 0:
        print("Error: Failed to find the 'Add' section in the .clangd file.")
        sys.exit(1)
    
    # Write the updated content back to the .clangd file.
    with open(clangd_path, 'w') as f:
        f.write(new_content)
    
    print(f"Successfully updated {clangd_path}.")

if __name__ == "__main__":
    main()
