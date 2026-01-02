#!/usr/bin/env python3
"""
clang_format_all.py - Format all C++ source files in the Zyrnix project.

Usage:
    python tools/clang_format_all.py [--check] [--verbose]

Options:
    --check     Check formatting without modifying files (exit 1 if changes needed)
    --verbose   Print each file being processed
    --dry-run   Show what would be formatted without making changes
"""

import argparse
import os
import subprocess
import sys
from pathlib import Path
from typing import List


CPP_EXTENSIONS = {'.cpp', '.hpp', '.h', '.cc', '.cxx', '.hxx', '.c'}


SKIP_DIRS = {'build', 'cmake-build-debug', 'cmake-build-release', 'out', 
             '.git', 'third_party', 'external', 'vendor'}


def find_project_root() -> Path:
    """Find the project root by looking for CMakeLists.txt."""
    current = Path(__file__).resolve().parent
    while current != current.parent:
        if (current / 'CMakeLists.txt').exists():
            return current
        current = current.parent
    return Path.cwd()


def find_cpp_files(root: Path) -> List[Path]:
    """Find all C++ source files in the project."""
    cpp_files = []
    
    for path in root.rglob('*'):
    
        if any(skip_dir in path.parts for skip_dir in SKIP_DIRS):
            continue
        
        if path.is_file() and path.suffix in CPP_EXTENSIONS:
            cpp_files.append(path)
    
    return sorted(cpp_files)


def check_clang_format() -> str:
    """Check if clang-format is available and return its path."""
 
    for name in ['clang-format', 'clang-format-17', 'clang-format-16', 
                 'clang-format-15', 'clang-format-14']:
        try:
            result = subprocess.run([name, '--version'], 
                                    capture_output=True, text=True)
            if result.returncode == 0:
                return name
        except FileNotFoundError:
            continue
    
    print("Error: clang-format not found. Please install clang-format.")
    print("  Ubuntu/Debian: sudo apt install clang-format")
    print("  macOS:         brew install clang-format")
    print("  Windows:       choco install llvm")
    sys.exit(1)


def format_file(clang_format: str, filepath: Path, check_only: bool = False,
                verbose: bool = False) -> bool:
    """
    Format a single file.
    
    Returns True if file was formatted (or needs formatting in check mode).
    """
    if verbose:
        print(f"Processing: {filepath}")
    
    if check_only:
      
        result = subprocess.run(
            [clang_format, '--style=file', '--dry-run', '--Werror', str(filepath)],
            capture_output=True, text=True
        )
        if result.returncode != 0:
            print(f"Needs formatting: {filepath}")
            return True
        return False
    else:
     
        result = subprocess.run(
            [clang_format, '--style=file', '-i', str(filepath)],
            capture_output=True, text=True
        )
        if result.returncode != 0:
            print(f"Error formatting {filepath}: {result.stderr}")
            return False
        return True


def main():
    parser = argparse.ArgumentParser(
        description='Format all C++ files in the Zyrnix project using clang-format'
    )
    parser.add_argument('--check', action='store_true',
                        help='Check formatting without modifying files')
    parser.add_argument('--verbose', '-v', action='store_true',
                        help='Print each file being processed')
    parser.add_argument('--dry-run', action='store_true',
                        help='Show files that would be formatted')
    parser.add_argument('paths', nargs='*', type=Path,
                        help='Specific files or directories to format')
    
    args = parser.parse_args()
    

    project_root = find_project_root()
    print(f"Project root: {project_root}")
    

    clang_format = check_clang_format()
    version = subprocess.run([clang_format, '--version'], 
                             capture_output=True, text=True)
    print(f"Using: {version.stdout.strip()}")
    

    if args.paths:
        cpp_files = []
        for path in args.paths:
            if path.is_file():
                cpp_files.append(path)
            elif path.is_dir():
                cpp_files.extend(find_cpp_files(path))
    else:
        cpp_files = find_cpp_files(project_root)
    
    print(f"Found {len(cpp_files)} C++ files")
    
    if args.dry_run:
        print("\nFiles that would be formatted:")
        for f in cpp_files:
            print(f"  {f.relative_to(project_root)}")
        return 0
    

    needs_formatting = 0
    for filepath in cpp_files:
        if format_file(clang_format, filepath, args.check, args.verbose):
            needs_formatting += 1
    

    if args.check:
        if needs_formatting > 0:
            print(f"\n{needs_formatting} file(s) need formatting.")
            print("Run 'python tools/clang_format_all.py' to fix.")
            return 1
        else:
            print("\nAll files are properly formatted!")
            return 0
    else:
        print(f"\nFormatted {len(cpp_files)} files.")
        return 0


if __name__ == '__main__':
    sys.exit(main())
