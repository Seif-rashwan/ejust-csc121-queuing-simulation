#!/usr/bin/env python3
"""
Enforces C++ naming conventions:
- Files: PascalCase (e.g., CustomerType.h, CustomerType.cpp)
- Classes: PascalCase (e.g., class CustomerType)
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
        filename = filepath.stem  # Remove extension
        if not is_pascal_case(filename):
            ERRORS.append(
                f"❌ File '{filepath}': should be PascalCase (e.g., {filename[0].upper()}{filename[1:] if len(filename) > 1 else ''})"
            )

def check_class_names():
    """Check that all class definitions use PascalCase."""
    cpp_files = list(Path('.').rglob('*.cpp')) + list(Path('.').rglob('*.h'))
    cpp_files = [f for f in cpp_files if '.github' not in str(f)]
    
    for filepath in cpp_files:
        try:
            with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
                content = f.read()
        except Exception as e:
            continue
        
        # Find all class definitions
        for match in re.finditer(r'class\s+(\w+)\s*[:{]', content):
            class_name = match.group(1)
            if not is_pascal_case(class_name):
                line_num = content[:match.start()].count('\n') + 1
                ERRORS.append(
                    f"❌ {filepath}:{line_num}: class '{class_name}' should be PascalCase"
                )

def main():
    """Run all naming convention checks."""
    check_file_names()
    check_class_names()
    
    if ERRORS:
        print("Naming convention violations found:\n")
        for error in ERRORS:
            print(f"  {error}")
        sys.exit(1)
    else:
        print("✅ All file and class names follow PascalCase convention!")
        sys.exit(0)

if __name__ == '__main__':
    main()
