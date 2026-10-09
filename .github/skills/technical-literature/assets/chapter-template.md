# NN. Chapter Title in Modern C++

<!--
HOW TO USE THIS TEMPLATE
- Copy to `NN_chapter/NN_section/README.md` and replace every <placeholder>.
- Delete all HTML comments (like this one) from the finished chapter.
- Body sections (1..N) are free-form: add as many as the subject needs.
- The LAST FIVE "##" sections are mandatory: keep their exact names and order.
- STYLE: narrative technical literature. Every example is preceded by an extended
  explanatory paragraph (what the idea is, which intuition fails, what the code shows)
  and followed by a short interpretation. Lists are for closed enumerations only;
  tables are for side-by-side comparison and are introduced and followed by prose.
-->

Open with one or two paragraphs that explain why this topic is trickier than it looks: the familiar one-line explanation, why it is only the beginning, and what the reader will be able to explain and reason about afterwards.

For experienced C++ developers the more important questions are the less obvious ones, and it helps to state them as a short list, since this is a genuine enumeration of parallel questions:

* what the language actually guarantees about <topic>;
* <surprising question 2>;
* <surprising question 3>;
* and <closing question>.

Then one paragraph on how the chapter is organized, and which examples it follows:

1. [`01_example/example.cpp`](01_example/example.cpp) — one-line description of what it demonstrates.
2. [`02_example/example.cpp`](02_example/example.cpp) — one-line description of what it demonstrates.

The repository currently builds as C++20. Examples that need a newer standard are feature-tested so the baseline stays buildable.

---

## 1. First topic: a claim, not a label

A short orienting paragraph for the whole section: what question it answers and why the following facts belong together.

### One fact per heading, phrased as a statement

<!-- 1-3 paragraphs of motivating prose BEFORE any code. Roughly 4-8 sentences each. -->

State the idea in plain terms, then expose the intuition that fails. Name the rule that actually governs, keeping the rule visibly separate from any mental model or implementation tendency, and finish by saying what the example is about to demonstrate.

```cpp
// Minimal snippet. Expected behavior is asserted, not only printed.
static_assert(sizeof(char) == 1);
```

<!-- Interpretation AFTER the code: what to notice, which edge case matters, what to take away. -->

Say what the reader should notice in the example, which limitation or pitfall matters, and the one thing to carry away. Do not narrate every line.

### Another fact

Another explanatory paragraph in the same arc: claim, failed intuition, governing rule, example, interpretation. Where a fact really is a closed enumeration (the members a rule applies to, the overloads involved), a short list inside the explanation is fine, but a section made only of bullets is not.

---

## 2. Second topic

### A fact

Prose, example, interpretation.

### A fact with sub-parts

An introductory paragraph that explains why the fact splits into variants and what distinguishes them.

#### Variant A

Prose, example, interpretation.

#### Variant B

Prose, example, interpretation.

---

## N. Cross-cutting pitfalls worth remembering

A paragraph that explains why these pitfalls are grouped: what they have in common, and which earlier sections they come from.

### Pitfall stated as a fact

Prose, example, interpretation.

<!-- ===================================================================
     MANDATORY CLOSING SECTIONS: exactly these five, in this order.
     =================================================================== -->

---

## Rules worth keeping in working memory

Open with a paragraph that states the mental model explaining most of the surprises in this chapter, and why it is the right model to carry.

> A compact, quotable definition of the model.

### Core rules

<!-- Rules are a closed set of parallel items, so a numbered list is appropriate. Each rule is one bold line plus at most one sentence of explanation. -->

Lead with one sentence introducing the rules, then list them:

1. **Rule stated as an imperative or a fact.** Why it holds.
2. **Rule.** Why it holds.

### Pitfalls at a glance

One or two sentences saying what the table summarizes and that every row is explained earlier in the chapter.

| Pitfall            | What happens                    | Do instead                     |
|--------------------|---------------------------------|--------------------------------|
| `<trap>`           | Observable consequence          | Safer alternative              |
| `<trap>`           | Observable consequence          | Safer alternative              |

### Mental model summary

<!-- Dense subjects only: compare the chapter's key entities side by side. Introduce in prose, then the table, then one sentence on the most important difference. -->

| Feature        | `<entity A>` | `<entity B>` | `<entity C>` |
|----------------|--------------|--------------|--------------|
| <property>     | Yes          | No           | Yes (note)   |
| Common pitfall | ...          | ...          | ...          |

### Review checklist

A checklist is a genuine run-through of independent questions, so a list is the right form here. Introduce it with a sentence on when to use it.

When reviewing <topic> code, ask:

- [ ] Question phrased so that "no" or "don't know" is the alarming answer?
- [ ] Question?
- [ ] Question?

---

## Diagnostics, useful compiler settings and extensions

One or two paragraphs: which problems the compiler can and cannot catch for this topic, and why a clean default warning set is not enough when the code is legal but wrong.

### Warnings

A sentence or two on which warning groups matter for this topic and any noise trade-off.

| Compiler  | Flags                          | Catches                         |
|-----------|--------------------------------|---------------------------------|
| GCC/Clang | `-Wall -Wextra -W<specific>`   | What these flags detect         |
| MSVC      | `/W4 /w14<NNN>`                | What these flags detect         |

### Sanitizers and runtime checks

A sentence on what runtime checking adds beyond compile-time diagnostics.

| Tool                      | Flag                     | Detects                    |
|---------------------------|--------------------------|----------------------------|
| UBSan                     | `-fsanitize=undefined`   | What it detects            |
| `<other>`                 | `<flag>`                 | What it detects            |

### Semantics-changing options

A paragraph explaining that these options change what the program means, not merely how fast it runs.

| Option                  | Effect on this topic                | Recommendation          |
|-------------------------|-------------------------------------|-------------------------|
| `<-fflag>` / `<ext>`    | What changes (not just speed)       | When to use / avoid     |

### Compiler extensions

<!-- Non-standard features relevant to the topic. Always state portability and the standard replacement, if any. -->

A paragraph on which non-standard features exist for this topic, which have since been standardized, and what portability they cost.

| Extension                         | Compilers        | What it gives                    | Standard alternative / note        |
|-----------------------------------|------------------|----------------------------------|------------------------------------|
| `__builtin_<name>`                | GCC/Clang        | What it does                     | `<std facility>` since C++NN       |
| `__attribute__((<attr>))`         | GCC/Clang        | What it does                     | `[[<attr>]]` since C++NN           |
| `__declspec(<attr>)` / `__assume` | MSVC             | What it does                     | `[[<attr>]]` / none                |
| `#pragma <name>`                  | all (varies)     | What it does                     | Portability caveat                 |
| `__int128`, `_Float16`, ...       | GCC/Clang        | Extra types relevant to topic    | `<stdfloat>`, none                 |
| `<predefined macro>`              | all              | Feature/platform detection       | `__cpp_<feature>` test macro       |

Follow the table with prose that makes three points: prefer the standard facility when it exists and keep extensions behind a small wrapper header or macro; guard them with feature-test macros (`__cpp_*`, `__has_cpp_attribute`, `__has_builtin`) rather than compiler-version checks; and use `-pedantic` / `-pedantic-errors` (GCC/Clang) or `/permissive-` (MSVC) to flag non-standard code, with `-std=c++NN` instead of `-std=gnu++NN` to disable GNU extensions.

### Libraries and tooling beyond the standard

A sentence on where tools and libraries take over from the language.

| Tool / library                      | Purpose                              | Typical use for this topic         |
|-------------------------------------|--------------------------------------|------------------------------------|
| clang-tidy / cppcheck               | Static analysis                      | Specific checks, e.g. `<check>`    |
| `<GSL / Boost / abseil / fmt>`      | Library-level extensions             | Safer or richer API for the topic  |
| Compiler Explorer                   | Inspect generated code per compiler  | Verify what each compiler does     |

Close with the layered strategy in one sentence: types and APIs that express intent, compiler warnings, sanitizers, extensions kept behind wrappers, static analysis, and targeted tests at the boundaries.

---

## Standards timeline

<!-- One H3 per standard, always in this order. Each is a short paragraph (2-4 sentences) that names the theme and the features, not a bare bullet list. If a standard did not change the topic, keep the heading and write "No material changes." -->

A sentence on how the topic evolved overall, so the individual entries read as one story.

### C++98/03: <theme>

A short paragraph: what the language already specified and why it still matters.

### C++11: <theme>

A short paragraph: what was introduced and what it changed for the reader's code.

### C++14: <theme>

A short paragraph.

### C++17: <theme>

A short paragraph.

### C++20: <theme>

A short paragraph.

### C++23: <theme>

A short paragraph.

### C++26: <theme>

A short paragraph. Mark features as "expected" if not yet shipped by major compilers.

Close with one sentence: comment the **version dependency** in code, not just the behavior of the current compiler.

---

## Migration note for Java / Python / C# developers

One or two paragraphs: the assumption that readers from managed languages bring to this topic, and where it breaks in C++.

### Where intuition transfers and where it breaks

A sentence introducing the comparison.

| Concept in this chapter | Java            | Python          | C#              | C++ reality                          |
|-------------------------|-----------------|-----------------|-----------------|--------------------------------------|
| `<concept>`             | Closest analog  | Closest analog  | Closest analog  | The actual difference (UB, lifetime, value semantics, ...) |
| `<concept>`             | Closest analog  | Closest analog  | Closest analog  | The actual difference                |

### Common wrong assumptions

Prose, one paragraph per assumption or a short paragraph covering a few. For each: "In <language> this is X, so in C++ it is X", what C++ does instead, and a pointer to the section above. Cover assumptions about memory, lifetime and ownership, and about defined behavior (no exception or default value where C++ gives undefined behavior).

### Idiomatic C++ replacement

A sentence introducing the mapping.

| Habit from managed languages | Idiomatic C++                           |
|------------------------------|-----------------------------------------|
| `<habit>`                    | `<facility>` and why                    |

---

## Further reading

A short lead-in sentence on what these sources are good for. Links are a closed enumeration, so lists are appropriate here.

### Standard and language reference

* Topic page: <https://en.cppreference.com/w/cpp/...>
* Topic page: <https://en.cppreference.com/w/cpp/...>

### Proposals and Core Guidelines

* P0000Rx — title: <https://wg21.link/p0000>
* C++ Core Guidelines, rule `X.NN`: <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#...>

### Articles and talks

* Author, "Title" — venue/year: <https://...>
