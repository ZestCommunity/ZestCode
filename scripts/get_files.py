#!/usr/bin/env python3

"""
Meson Build Source File Collector

This script recursively searches a directory for source files matching specified extensions
and outputs their absolute paths. Intended for integration with Meson build system, so
you don't have to list every single one of your source files.
"""

import sys
from pathlib import Path

def main():
    """
    Main function handling command-line arguments and file discovery.
    
    Process flow:
    1. Validate command-line arguments
    2. Resolve source directory path
    3. Recursively search for files matching patterns
    4. Deduplicate and sort results
    5. Output absolute file paths
    """
    if len(sys.argv) < 3:
        print(f"Usage: {sys.argv[0]} <source_dir> <ext1> <ext2> ...", file=sys.stderr)
        sys.exit(1)

    # Resolve source directory to absolute path
    source_dir = Path(sys.argv[1]).resolve()
    
    # Extract file extension patterns from arguments (supports glob patterns like *.c)
    patterns = sys.argv[2:]

    # Verify source directory exists
    if not source_dir.is_dir():
        print(f"Error: Source directory '{source_dir}' does not exist.", file=sys.stderr)
        sys.exit(1)

    # Use set to avoid duplicate entries from overlapping patterns
    files = set()

    # Process each file pattern
    for pattern in patterns:
        # Recursive glob search (rglob) through directory tree
        for path in source_dir.rglob(pattern):
            # Skip directories that might match pattern (e.g., 'some.dir' matching *.h)
            if path.is_file():
                # Add resolved absolute path to ensure consistency
                files.add(path.resolve())

    # Sort files for deterministic output order
    sorted_files = sorted(files)

    # Print results in format Meson can consume (one absolute path per line)
    for file_path in sorted_files:
        print(file_path)

if __name__ == '__main__':
    # Entry point when executed directly
    main()