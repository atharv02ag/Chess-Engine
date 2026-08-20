#!/usr/bin/env python3
"""Build and run a C++ entry point in the chess engine repository."""

from __future__ import annotations

import argparse
import os
from pathlib import Path
import shlex
import shutil
import subprocess
import sys
from typing import Optional


REPO_ROOT = Path(__file__).resolve().parents[1]

COMMON_SOURCES = (
    "src/board.cpp",
    "src/move.cpp",
    "src/move_generator.cpp",
    "src/tt.cpp",
)

OPTIMIZED_FLAGS = (
    "-std=c++20",
    "-O3",
    "-march=native",
    "-flto",
    "-DNDEBUG",
    "-Wall",
    "-Wextra",
    "-Wpedantic",
)


def find_compiler(requested: Optional[str]) -> str:
    candidates = []
    if requested:
        candidates.append(requested)
    elif os.environ.get("CXX"):
        candidates.append(os.environ["CXX"])
    candidates.extend(("g++", "clang++"))

    for candidate in candidates:
        resolved = shutil.which(candidate)
        if resolved:
            return resolved

    searched = ", ".join(candidates)
    raise RuntimeError(f"No C++ compiler found. Tried: {searched}")


def display_command(command: list[str]) -> str:
    if hasattr(shlex, "join"):
        return shlex.join(command)
    return " ".join(shlex.quote(part) for part in command)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Compile and run an optimized C++ entry point."
    )
    parser.add_argument(
        "source",
        type=Path,
        help="Entry-point .cpp file, relative to the repository root.",
    )
    parser.add_argument(
        "--compiler",
        help="C++ compiler executable. Defaults to CXX, then g++, then clang++.",
    )
    parser.add_argument(
        "--build-dir",
        type=Path,
        default=REPO_ROOT / "build",
        help="Output directory (default: <repo>/build).",
    )
    parser.add_argument(
        "--compile-only",
        action="store_true",
        help="Build the target without running it.",
    )
    raw_args = sys.argv[1:]
    if "--" in raw_args:
        separator = raw_args.index("--")
        script_args = raw_args[:separator]
        program_args = raw_args[separator + 1 :]
    else:
        script_args = raw_args
        program_args = []

    args = parser.parse_args(script_args)
    args.program_args = program_args
    return args


def main() -> int:
    args = parse_args()

    try:
        compiler = find_compiler(args.compiler)
    except RuntimeError as error:
        print(f"error: {error}", file=sys.stderr)
        return 2

    source = args.source.expanduser()
    if not source.is_absolute():
        source = REPO_ROOT / source
    source = source.resolve()
    if source.suffix.lower() != ".cpp":
        print(f"error: source must be a .cpp file: {source}", file=sys.stderr)
        return 2
    if not source.is_file():
        print(f"error: source file does not exist: {source}", file=sys.stderr)
        return 2
    try:
        source_label = source.relative_to(REPO_ROOT)
    except ValueError:
        print(f"error: source must be inside the repository: {source}", file=sys.stderr)
        return 2

    build_dir = args.build_dir.expanduser().resolve()
    build_dir.mkdir(parents=True, exist_ok=True)

    executable_suffix = ".exe" if os.name == "nt" else ""
    executable = build_dir / f"{source.stem}{executable_suffix}"
    common_sources = tuple((REPO_ROOT / path).resolve() for path in COMMON_SOURCES)
    sources = tuple(path for path in common_sources if path != source) + (source,)

    compile_command = [
        compiler,
        *(str(path) for path in sources),
        *OPTIMIZED_FLAGS,
        "-o",
        str(executable),
    ]

    print(f"Building {source_label}:")
    print(f"  {display_command(compile_command)}", flush=True)
    try:
        subprocess.run(compile_command, cwd=REPO_ROOT, check=True)
    except subprocess.CalledProcessError as error:
        return error.returncode

    if args.compile_only:
        print(f"Built {executable}")
        return 0

    run_command = [str(executable), *args.program_args]
    print(f"Running {source_label}:", flush=True)
    print(f"  {display_command(run_command)}", flush=True)
    try:
        completed = subprocess.run(run_command, cwd=REPO_ROOT, check=False)
    except OSError as error:
        print(f"error: could not run {executable}: {error}", file=sys.stderr)
        return 1
    return completed.returncode


if __name__ == "__main__":
    raise SystemExit(main())
