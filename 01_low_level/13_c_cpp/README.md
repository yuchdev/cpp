# C / C++ Compatibility (Advanced) — C89/C99/C11 ↔ C++98 → C++20

This chapter focuses on **principal language and toolchain differences** that matter when you:
- build mixed C/C++ projects
- wrap C libraries in C++
- expose C++ APIs to C code
- reason about initialization, ABI, and linkage
- migrate code between the languages

Some files are **C-only** (`.c`) and some are **C++** (`.cpp`).

## ABI safety checklist for mixed projects

- `extern "C"` changes **linkage / mangling**, not C++ semantics.
- Never let C++ exceptions cross into C: catch inside and return error codes.
- Never expose `std::string`, `std::vector`, or C++ classes in a C ABI.
- Prefer an opaque-handle API: `create/destroy`, `int` status codes, POD structs.
