# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this repo is

"Advanced C++ By Example" — a course for junior-to-mid C++ developers. It is a collection of small, standalone example programs, not an application or library. Each example pairs runnable code with theory (Standard references) in the chapter `README.md`. Early chapters are polished; later ones (03–05) are drafts/snippets.

## Build

CMake (≥3.14), C++20 (`CMAKE_CXX_STANDARD 20`). In-source builds are rejected — always use a separate build dir. CLion uses `cmake-build-debug/` (Ninja, exports `compile_commands.json`).

```sh
cmake -S . -B cmake-build-debug -G Ninja
cmake --build cmake-build-debug                      # everything that is enabled
cmake --build cmake-build-debug --target 04_const_expr_eval   # one example
./cmake-build-debug/01_low_level/02_constness/04_const_expr_eval/04_const_expr_eval
```

There is no test suite; examples self-check with `assert`/`static_assert` and print output. "Testing" an example means building and running its executable (Debug build, so asserts are live).

Formatting: `.clang-format` (Allman braces, 4-space indent, no column limit) and `.cmake-format` for CMake files.

## Structure and build wiring

- Hierarchy: `NN_chapter/NN_section/NN_example/`. Numbering defines the intended reading order.
- Each leaf example dir has its own `CMakeLists.txt` producing **one executable whose target name equals the directory name** (e.g. `04_const_expr_eval`). Sources are picked up with `file(GLOB *.cpp *.h)`, so all `.cpp` files in an example dir are linked into one binary — multi-file examples (e.g. `01_low_level/10_linking/*`) rely on this. After adding a file, re-run CMake configure.
- Every intermediate dir has a `CMakeLists.txt` that is just a list of `add_subdirectory(...)`.
- What is actually built is controlled by those lists. Currently the root builds only `utilities`, `01_low_level`, and `02_oop` (and within `02_oop` only `01_classes`, `02_memory`); `03_templates`, `04_stl`, `05_concurrency` are commented out / not wired in. `01_low_level/13_ub` is **intentionally** excluded because it demonstrates undefined behavior.
- `utilities/` is a header-only INTERFACE library (`elapsed.h` timer, `bitwise.h`, `generate.h`, `functional.h`). Examples link it and include via `${CMAKE_SOURCE_DIR}`, e.g. `#include "utilities/elapsed.h"`.
- `set_property(... FOLDER ...)` is for Visual Studio solution grouping; the root `CMakeLists.txt` also has MSVC-specific flags.

## Adding / renumbering examples

`script/build_helper.py <chapter_or_section_dir> [--sequential-rename]` regenerates the leaf `CMakeLists.txt` files (standard template) and the parent `add_subdirectory` list for all subdirs of the given directory; with `--sequential-rename` it `git mv`s subdirs to `01_…, 02_…` order. **Note: it overwrites existing CMakeLists.txt files and runs `git add` + `git commit --all`** — don't run it without the user's approval.

## Code conventions in examples

- Recently reworked examples (e.g. `01_low_level/02_constness`) put code in `namespace cpp { ... }` with small static functions, each demonstrating one fact, called from `main()`.
- Expected behavior is asserted in-code (`static_assert` for compile-time facts, `assert` for runtime) rather than only printed; code that intentionally doesn't compile is left commented out with an explanation.
- Chapter `README.md` files are concise bullet-point "facts" lists with short snippets; keep new theory in that style.
