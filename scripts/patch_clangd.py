#!/usr/bin/env python3
import sys
import os
import subprocess
import re
import shutil

def main():
    if len(sys.argv) != 2:
        print(f"Usage: python3 {sys.argv[0]} <absolute-path-to-arm-none-eabi-gcc>")
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

    # Automatically detect the gcc version folder inside lib/gcc/arm-none-eabi.
    gcc_dir = os.path.join(base_dir, "lib", "gcc", "arm-none-eabi")
    if not os.path.isdir(gcc_dir):
        print(f"Error: {gcc_dir} is not a valid directory")
        sys.exit(1)

    # List all subdirectories in gcc_dir.
    subdirs = [d for d in os.listdir(gcc_dir) if os.path.isdir(os.path.join(gcc_dir, d))]
    if not subdirs:
        print(f"Error: No directories found in {gcc_dir}")
        sys.exit(1)

    # Choose the directory with the highest semantic version.
    try:
        gcc_version = max(subdirs, key=lambda v: tuple(map(int, v.split("."))))
    except Exception as e:
        print(f"Error: Could not determine highest semantic version in {gcc_dir}: {e}")
        sys.exit(1)

    # Construct the six include strings.
    outputs = [
        f"-isystem{base_dir}/lib/gcc/arm-none-eabi/{gcc_version}/../../../../arm-none-eabi/include/c++/{gcc_version}",
        f"-isystem{base_dir}/lib/gcc/arm-none-eabi/{gcc_version}/../../../../arm-none-eabi/include/c++/{gcc_version}/arm-none-eabi/thumb/v7-a+simd/hard",
        f"-isystem{base_dir}/lib/gcc/arm-none-eabi/{gcc_version}/../../../../arm-none-eabi/include/c++/{gcc_version}/backward",
        f"-isystem{base_dir}/lib/gcc/arm-none-eabi/{gcc_version}/include",
        f"-isystem{base_dir}/lib/gcc/arm-none-eabi/{gcc_version}/include-fixed",
        f"-isystem{base_dir}/lib/gcc/arm-none-eabi/{gcc_version}/../../../../arm-none-eabi/include"
    ]
    # Normalize paths to use forward slashes.
    outputs = [s.replace(os.sep, '/') for s in outputs]

    # Determine the script's directory and its parent.
    script_dir = os.path.dirname(os.path.abspath(__file__))
    parent_dir = os.path.dirname(script_dir)

    # Source .clangd file in the same directory as the script.
    source_clangd_path = os.path.join(script_dir, ".clangd")
    if not os.path.isfile(source_clangd_path):
        print(f"Error: {source_clangd_path} does not exist.")
        sys.exit(1)
    
    # Destination .clangd file in the parent directory.
    dest_clangd_path = os.path.join(parent_dir, ".clangd")
    try:
        shutil.copy2(source_clangd_path, dest_clangd_path)
        print(f"Copied {source_clangd_path} to {dest_clangd_path}")
    except Exception as e:
        print(f"Error copying .clangd: {e}")
        sys.exit(1)

    # Read the content of the copied .clangd file.
    with open(dest_clangd_path, 'r') as f:
        content = f.read()

    # Locate the "Add" section using regex.
    # This pattern captures the block starting with "Add:" and an opening bracket,
    # then captures its contents until the closing bracket.
    pattern = r"(Add:\s*\[\s*\n)(.*?)(\n\s*\])"
    match = re.search(pattern, content, flags=re.DOTALL)
    if not match:
        print("Error: Failed to find the 'Add' section in the .clangd file.")
        sys.exit(1)

    prefix = match.group(1)  # e.g., "Add: [\n" with indentation
    block_content = match.group(2)
    suffix = match.group(3)  # e.g., "\n  ]" (closing bracket with indentation)

    # Split the block into lines.
    lines = block_content.splitlines()

    # Determine the indentation from the first non-empty line, or default to 4 spaces.
    indent = "    "
    for line in lines:
        stripped = line.lstrip()
        if stripped:
            indent = line[:len(line)-len(stripped)]
            break

    # Build new lines for the computed strings.
    new_inserts = [f'{indent}"{item}",' for item in outputs]

    # Insert the new lines at the beginning of the existing block.
    new_block_lines = new_inserts + lines
    new_block = prefix + "\n".join(new_block_lines) + suffix

    # Replace the original Add block with the new block.
    new_content = "# If you want to edit this file, edit scripts/.clangd and reconfigure meson\n" + content[:match.start()] + new_block + content[match.end():]

    # Write the updated content back to the destination .clangd file.
    with open(dest_clangd_path, 'w') as f:
        f.write(new_content)
    
    print(f"Successfully updated {dest_clangd_path}.")

if __name__ == "__main__":
    main()
