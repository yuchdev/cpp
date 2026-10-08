# NN. Chapter Title in Modern C++

<!--
HOW TO USE THIS TEMPLATE
- Copy to `NN_chapter/NN_section/README.md` and replace every <placeholder>.
- Delete all HTML comments (like this one) from the finished chapter.
- Body sections (1..N) are free-form: add as many as the subject needs.
- The LAST FOUR "##" sections are mandatory, keep their exact names and order.
-->

One or two paragraphs: why this topic is trickier than it looks, and what the reader will be able to explain afterwards.

For experienced C++ developers, the more important questions are:

* what the language actually guarantees about <topic>;
* <surprising question 2>;
* <surprising question 3>;
* and <closing question>.

This chapter follows the examples in this directory:

1. [`01_example/example.cpp`](01_example/example.cpp) — one-line description of what it demonstrates.
2. [`02_example/example.cpp`](02_example/example.cpp) — one-line description of what it demonstrates.

The repository currently builds as C++20. Examples that need a newer standard are feature-tested so the baseline stays buildable.

---

## 1. First topic: a claim, not a label

Optional one-paragraph overview of the section.

### One fact per heading, phrased as a statement

Short explanation (2-5 sentences).

```cpp
// Minimal snippet. Expected behavior is asserted, not only printed.
static_assert(sizeof(char) == 1);
```

Why it matters in practice, or the rule to take away.

### Another fact

* bullet-point facts in the repo's concise style;
* each ends with a semicolon, the last with a full stop.

---

## 2. Second topic

### A fact

...

### A fact with sub-parts

#### Variant A

...

#### Variant B

...

---

## N. Cross-cutting pitfalls worth remembering

### Pitfall stated as a fact

...

<!-- ===================================================================
     MANDATORY CLOSING SECTIONS: exactly these four, in this order.
     =================================================================== -->

---

## Rules worth keeping in working memory

One-sentence framing: the mental model that explains most of the surprises in this chapter.

> A compact, quotable definition of the model.

### Core rules

Numbered or bulleted; each rule is **one line in bold** plus at most one sentence of explanation.

1. **Rule stated as an imperative or a fact.** Why it holds.
2. **Rule.** Why it holds.

### Pitfalls at a glance

| Pitfall            | What happens                    | Do instead                     |
|--------------------|---------------------------------|--------------------------------|
| `<trap>`           | Observable consequence          | Safer alternative              |
| `<trap>`           | Observable consequence          | Safer alternative              |

### Mental model summary

<!-- Dense subjects: compare the chapter's key entities side by side. -->

| Feature        | `<entity A>` | `<entity B>` | `<entity C>` |
|----------------|--------------|--------------|--------------|
| <property>     | Yes          | No           | Yes (note)   |
| Common pitfall | ...          | ...          | ...          |

### Review checklist

When reviewing <topic> code, ask:

- [ ] Question phrased so that "no" or "don't know" is the alarming answer?
- [ ] Question?
- [ ] Question?

---

## Diagnostics, useful compiler settings and extensions

One or two sentences: which problems the compiler can and cannot catch for this topic.

### Warnings

| Compiler  | Flags                          | Catches                         |
|-----------|--------------------------------|---------------------------------|
| GCC/Clang | `-Wall -Wextra -W<specific>`   | What these flags detect         |
| MSVC      | `/W4 /w14<NNN>`                | What these flags detect         |

### Sanitizers and runtime checks

| Tool                      | Flag                     | Detects                    |
|---------------------------|--------------------------|----------------------------|
| UBSan                     | `-fsanitize=undefined`   | What it detects            |
| `<other>`                 | `<flag>`                 | What it detects            |

### Semantics-changing options

| Option                  | Effect on this topic                | Recommendation          |
|-------------------------|-------------------------------------|-------------------------|
| `<-fflag>` / `<ext>`    | What changes (not just speed)       | When to use / avoid     |

### Compiler extensions

<!-- Non-standard features relevant to the topic. Always state portability and the standard replacement, if any. -->

| Extension                         | Compilers        | What it gives                    | Standard alternative / note        |
|-----------------------------------|------------------|----------------------------------|------------------------------------|
| `__builtin_<name>`                | GCC/Clang        | What it does                     | `<std facility>` since C++NN       |
| `__attribute__((<attr>))`         | GCC/Clang        | What it does                     | `[[<attr>]]` since C++NN           |
| `__declspec(<attr>)` / `__assume` | MSVC             | What it does                     | `[[<attr>]]` / none                |
| `#pragma <name>`                  | all (varies)     | What it does                     | Portability caveat                 |
| `__int128`, `_Float16`, ...       | GCC/Clang        | Extra types relevant to topic    | `<stdfloat>`, none                 |
| `<predefined macro>`              | all              | Feature/platform detection       | `__cpp_<feature>` test macro       |

* Prefer the standard facility when it exists; keep extensions behind a small wrapper header or macro.
* Guard them with feature-test macros (`__cpp_*`, `__has_cpp_attribute`, `__has_builtin`) rather than compiler-version checks.
* `-pedantic` / `-pedantic-errors` (GCC/Clang) and `/permissive-` (MSVC) flag non-standard code; `-std=c++NN` instead of `-std=gnu++NN` disables GNU extensions.

### Libraries and tooling beyond the standard

| Tool / library                      | Purpose                              | Typical use for this topic         |
|-------------------------------------|--------------------------------------|------------------------------------|
| clang-tidy / cppcheck               | Static analysis                      | Specific checks, e.g. `<check>`    |
| `<GSL / Boost / abseil / fmt>`      | Library-level extensions             | Safer or richer API for the topic  |
| Compiler Explorer                   | Inspect generated code per compiler  | Verify what each compiler does     |

Closing sentence: the layered strategy (types and APIs, warnings, sanitizers, extensions kept behind wrappers, static analysis, tests).

---

## Standards timeline

<!-- One H3 per standard, always in this order. If a standard did not change
     the topic, keep the heading and write "No material changes." -->

### C++98/03: <theme>

* feature or rule;
* feature or rule.

### C++11: <theme>

* feature or rule;
* feature or rule.

### C++14: <theme>

* feature or rule.

### C++17: <theme>

* feature or rule.

### C++20: <theme>

* feature or rule;
* feature or rule.

### C++23: <theme>

* feature or rule.

### C++26: <theme>

* feature or rule (mark as "expected" if not yet shipped by major compilers).

One closing sentence: comment the **version dependency** in code, not just the behavior of the current compiler.

---

## Further reading

Short lead-in sentence.

### Standard and language reference

* Topic page: <https://en.cppreference.com/w/cpp/...>
* Topic page: <https://en.cppreference.com/w/cpp/...>

### Proposals and Core Guidelines

* P0000Rx — title: <https://wg21.link/p0000>
* C++ Core Guidelines, rule `X.NN`: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#...>

### Articles and talks

* Author, "Title" — venue/year: <https://...>
