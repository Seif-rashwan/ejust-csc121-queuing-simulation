#!/usr/bin/env python3
"""
Enforces C++ file naming conventions:
- Files: PascalCase (e.g., CustomerType.h, CustomerType.cpp)
"""

import re
import sys
from pathlib import Path

ERRORS = []

def is_pascal_case(name):
    """Check if name is in PascalCase."""
    # PascalCase: starts with uppercase, only alphanumeric
    return bool(re.match(r'^[A-Z][a-zA-Z0-9]*$', name))

def check_file_names():
    """Check that all .cpp and .h files use PascalCase."""
    cpp_files = list(Path('src').glob('*.cpp')) + list(Path('src').glob('*.h')) + \
                list(Path('include').glob('*.cpp')) + list(Path('include').glob('*.h'))
    
    for filepath in cpp_files:
        filename = filepath.stem
        
        # allow lowercase 'main' as exception
        if filename.lower() == 'main':
            continue
        if not is_pascal_case(filename):
            ERRORS.append(
                f"❌ File '{filepath}': should be PascalCase (e.g., {filename[0].upper()}{filename[1:] if len(filename) > 1 else ''})"
            )

def main():
    """Run all naming convention checks."""
    check_file_names()
    
    if ERRORS:
        print("File naming convention violations found:\n")
        for error in ERRORS:
            print(f"  {error}")
        sys.exit(1)
    else:
        print("✅ All file names follow PascalCase convention!")
        sys.exit(0)

if __name__ == '__main__':
    main()
