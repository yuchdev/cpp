# C / C++ Compatibility (Advanced) - C89/C99/C11 ↔ C++98 → C++20

This chapter focuses on **principal language and toolchain differences** that matter when you:
- build mixed C/C++ projects
- wrap C libraries in C++
- expose C++ APIs to C code
- reason about initialization, ABI, and linkage
- migrate code between the languages

Some files are **C-only** (`.c`) and some are **C++** (`.cpp`).

## Files

1. `01_name_mangling_and_overload.cpp`
   - C++ name mangling, why overloads work
   - how to inspect symbols with `nm` / `objdump`
   - why C cannot overload by signature

2. `02_extern_c_and_callbacks.cpp`
   - `extern "C"`: linkage and ABI
   - C-callable functions exported from C++
   - callbacks/function pointers across the boundary
   - pitfalls: exceptions, C++ objects across ABI

3. `03_static_init_and_main.cpp`
   - C++ startup: global constructors run **before `main`**
   - C startup: no user constructors/destructors
   - init-order fiasco + safe patterns

4. `04_headers_and_types.cpp`
   - `<string.h>` vs `<cstring>` mapping
   - `void*` conversions, `malloc` in C vs C++
   - `bool` vs `_Bool`, `nullptr` vs `NULL`
   - `restrict` in C99 and portable C++ replacements

5. `05_c_things_not_in_cpp.c`
   - Valid C features that are ill-formed in standard C++:
     VLAs, compound literals, array designators, `restrict`, implicit `void*` conversion,
     C struct-tag differences.

6. `06_cpp_things_not_in_c.cpp`
   - C++ features that C doesn't have (RAII/classes/templates/etc.)
   - a correct "C ABI facade" pattern: opaque handles + create/destroy + error codes

## Build

### C++ examples
```bash
g++ -std=c++20 -Wall -Wextra -pedantic 01_name_mangling_and_overload.cpp -o ex1
g++ -std=c++20 -Wall -Wextra -pedantic 02_extern_c_and_callbacks.cpp -o ex2
g++ -std=c++20 -Wall -Wextra -pedantic 03_static_init_and_main.cpp -o ex3
g++ -std=c++20 -Wall -Wextra -pedantic 04_headers_and_types.cpp -o ex4
g++ -std=c++20 -Wall -Wextra -pedantic 06_cpp_things_not_in_c.cpp -o ex6
```

### C-only example
```bash
cc -std=c11 -Wall -Wextra -pedantic 05_c_things_not_in_cpp.c -o ex5
```

### Inspect symbols (name mangling)
```bash
nm ex1 | grep print
nm -C ex1 | grep print
```

## ABI safety checklist for mixed projects

- `extern "C"` changes **linkage / mangling**, not C++ semantics.
- Never let C++ exceptions cross into C: catch inside and return error codes.
- Never expose `std::string`, `std::vector`, or C++ classes in a C ABI.
- Prefer an opaque-handle API: `create/destroy`, `int` status codes, POD structs.
