#!/usr/bin/env python3
"""
generate_docs.py - Generate API documentation for the Zyrnix project.

This script parses header files and generates markdown documentation
for the public API.

Usage:
    python tools/generate_docs.py [--output docs/api]
"""

import argparse
import os
import re
import sys
from dataclasses import dataclass, field
from pathlib import Path
from typing import List, Optional, Dict


@dataclass
class Parameter:
    """Represents a function parameter."""
    name: str
    type: str
    description: str = ""


@dataclass
class Function:
    """Represents a function or method."""
    name: str
    return_type: str
    parameters: List[Parameter] = field(default_factory=list)
    description: str = ""
    is_static: bool = False
    is_virtual: bool = False
    is_const: bool = False


@dataclass
class Class:
    """Represents a class or struct."""
    name: str
    description: str = ""
    methods: List[Function] = field(default_factory=list)
    base_classes: List[str] = field(default_factory=list)
    is_struct: bool = False


@dataclass 
class Header:
    """Represents a header file."""
    path: Path
    classes: List[Class] = field(default_factory=list)
    functions: List[Function] = field(default_factory=list)
    description: str = ""


def find_project_root() -> Path:
    """Find the project root by looking for CMakeLists.txt."""
    current = Path(__file__).resolve().parent
    while current != current.parent:
        if (current / 'CMakeLists.txt').exists():
            return current
        current = current.parent
    return Path.cwd()


def extract_doxygen_comment(content: str, pos: int) -> str:
    """Extract the doxygen comment before a given position."""

    before = content[:pos].rstrip()
    

    if before.endswith('*/'):
        start = before.rfind('/**')
        if start != -1:
            comment = before[start:].strip()
     
            lines = comment.split('\n')
            cleaned = []
            for line in lines:
                line = re.sub(r'^/\*\*\s*', '', line)
                line = re.sub(r'\*/$', '', line)
                line = re.sub(r'^\s*\*\s?', '', line)
                line = line.strip()
                if line:
                    cleaned.append(line)
            return ' '.join(cleaned)
    

    lines = before.split('\n')
    comment_lines = []
    for line in reversed(lines):
        line = line.strip()
        if line.startswith('///'):
            comment_lines.insert(0, line[3:].strip())
        elif line.startswith('//'):
            continue
        else:
            break
    
    return ' '.join(comment_lines)


def parse_header(filepath: Path) -> Header:
    """Parse a header file and extract documentation info."""
    header = Header(path=filepath)
    
    try:
        content = filepath.read_text(encoding='utf-8')
    except Exception as e:
        print(f"Warning: Could not read {filepath}: {e}")
        return header
    

    file_comment = re.search(r'/\*\*[\s\S]*?\*/', content[:500])
    if file_comment:
        header.description = extract_doxygen_comment(content, file_comment.end())
    

    class_pattern = re.compile(
        r'(class|struct)\s+(\w+)(?:\s*:\s*(?:public|protected|private)?\s*([\w:]+))?\s*\{',
        re.MULTILINE
    )
    
    for match in class_pattern.finditer(content):
        is_struct = match.group(1) == 'struct'
        class_name = match.group(2)
        base_class = match.group(3)
        
        cls = Class(
            name=class_name,
            is_struct=is_struct,
            description=extract_doxygen_comment(content, match.start())
        )
        
        if base_class:
            cls.base_classes.append(base_class)
        
        header.classes.append(cls)
    

    func_pattern = re.compile(
        r'^\s*(static\s+)?([\w:<>]+)\s+(\w+)\s*\(([^)]*)\)',
        re.MULTILINE
    )
    
    return header


def generate_class_doc(cls: Class) -> str:
    """Generate markdown documentation for a class."""
    lines = []
    
    kind = "struct" if cls.is_struct else "class"
    lines.append(f"### `{kind} {cls.name}`")
    lines.append("")
    
    if cls.description:
        lines.append(cls.description)
        lines.append("")
    
    if cls.base_classes:
        lines.append(f"**Inherits from:** `{'`, `'.join(cls.base_classes)}`")
        lines.append("")
    
    if cls.methods:
        lines.append("#### Methods")
        lines.append("")
        for method in cls.methods:
            params = ', '.join(f"{p.type} {p.name}" for p in method.parameters)
            signature = f"`{method.return_type} {method.name}({params})`"
            if method.is_const:
                signature += " const"
            lines.append(f"- {signature}")
            if method.description:
                lines.append(f"  - {method.description}")
        lines.append("")
    
    return '\n'.join(lines)


def generate_header_doc(header: Header, project_root: Path) -> str:
    """Generate markdown documentation for a header file."""
    lines = []
    
    rel_path = header.path.relative_to(project_root)
    lines.append(f"## `{rel_path}`")
    lines.append("")
    
    if header.description:
        lines.append(header.description)
        lines.append("")
    
    for cls in header.classes:
        lines.append(generate_class_doc(cls))
    
    return '\n'.join(lines)


def generate_docs(project_root: Path, output_dir: Path) -> None:
    """Generate documentation for all header files."""
    include_dir = project_root / 'include' / 'Zyrnix'
    
    if not include_dir.exists():
        print(f"Error: Include directory not found: {include_dir}")
        sys.exit(1)
    

    headers = []
    for header_path in sorted(include_dir.glob('*.hpp')):
        print(f"Parsing: {header_path.name}")
        headers.append(parse_header(header_path))
    
 
    output_dir.mkdir(parents=True, exist_ok=True)
    
    index_lines = [
        "# Zyrnix API Reference",
        "",
        "Auto-generated API documentation for Zyrnix.",
        "",
        "## Headers",
        "",
    ]
    
    for header in headers:
        name = header.path.stem
        index_lines.append(f"- [{name}]({name}.md)")
        
     
        doc_content = [
            f"# {name}",
            "",
            f"Header: `include/Zyrnix/{header.path.name}`",
            "",
        ]
        
        if header.classes:
            doc_content.append("## Classes")
            doc_content.append("")
            for cls in header.classes:
                doc_content.append(generate_class_doc(cls))
        
        output_file = output_dir / f"{name}.md"
        output_file.write_text('\n'.join(doc_content), encoding='utf-8')
        print(f"Generated: {output_file}")
    
z
    index_file = output_dir / "index.md"
    index_file.write_text('\n'.join(index_lines), encoding='utf-8')
    print(f"Generated: {index_file}")


def main():
    parser = argparse.ArgumentParser(
        description='Generate API documentation for Zyrnix'
    )
    parser.add_argument('--output', '-o', type=Path, default=Path('docs/api'),
                        help='Output directory for generated docs')
    parser.add_argument('--verbose', '-v', action='store_true',
                        help='Verbose output')
    
    args = parser.parse_args()
    
    project_root = find_project_root()
    print(f"Project root: {project_root}")
    
    output_dir = project_root / args.output
    print(f"Output directory: {output_dir}")
    
    generate_docs(project_root, output_dir)
    
    print("\nDocumentation generated successfully!")
    return 0


if __name__ == '__main__':
    sys.exit(main())
