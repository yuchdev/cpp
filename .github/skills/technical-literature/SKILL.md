---
name: technical-literature
description: Write, rewrite, edit, or translate long-form technical prose — book chapters, tutorials, engineering essays, explanatory articles — as continuous, narrative explanation that teaches how and why something works, not just what to do. Use for chapters of this C++ course (README.md files in NN_chapter/NN_section), for rewriting bullet-list notes into prose, and for reviewing a chapter against the house style and chapter template. Not for API reference, runbooks, install steps, CLI reference, or other technical documentation.
---

# Technical Literature

Technical literature explains technology; technical documentation operates it. The reader of a documentation page asks *what exactly must I do, configure, call, or verify?* The reader of a chapter asks *how does this work, why does it matter, and what should I understand about it?* This skill is for the second reader. The text should sound like a knowledgeable engineer explaining the subject, not like a specification or a ticket queue.

The priority order is fixed: **technical truth, then conceptual meaning, then terminology, then pedagogy, then flow, then voice.** When precision and elegance conflict, precision wins.

## What makes it different from documentation

Documentation is built from lists, imperatives, and tables because its reader scans for an answer. A chapter is built from paragraphs because its reader follows an argument. Three habits carry most of the difference:

1. **Prose carries the reasoning; lists carry only enumerations.** A claim, its cause, and its consequence belong in sentences that connect them. Use a list when the items are genuinely parallel and independent (a set of flags, a checklist to run through), and a table when comparing the same attributes across several things. Never convert an explanation into bullets to make it look organized.
2. **An extended paragraph comes before the example.** Do not open a section with a heading followed directly by a code block, and do not open with a bullet list of facts. First say what the idea is, why a reader's intuition about it is wrong or incomplete, and what the code is about to demonstrate. Then show the code. Then say what to notice and where the limits are.
3. **The reader is addressed as a peer, not instructed.** Prefer "consider what happens when…" and "the compiler is allowed to assume…" over "Do X. Avoid Y." Advice follows from explanation, and arrives after it.

## Anatomy of a section

Every `###` fact in a chapter follows the same arc, which is what makes a long chapter feel continuous rather than encyclopedic:

1. **Heading** — a statement of the fact, not a label (`const is shallow`, not `Shallow const`).
2. **Motivating prose** — one to three paragraphs (typically 4–8 sentences each). State the idea, expose the intuition that fails, name the rule that actually governs, and say what the example will show. Keep rule and mental model visibly separate.
3. **Example** — minimal, self-contained, with expected behavior asserted (`static_assert` / `assert`) rather than only printed. Intentionally non-compiling lines stay commented out with the reason.
4. **Interpretation** — a short paragraph: what to notice, which edge case or pitfall matters, what to take away. Do not narrate obvious lines.

Vary this rhythm deliberately. A section opener may be a single orienting paragraph. A closing remark may be one emphatic sentence. A bullet list is acceptable inside step 2 or 4 when it enumerates a closed set (the member functions a rule applies to, say), but a section made only of bullets has failed the style.

## Core principles

- **Technical truth first.** Preserve conditions, limits, uncertainty, distinctions, edge cases. Never improve style at the cost of correctness.
- **Explain, do not merely state.** Keep the why, the failed intuition, the consequence.
- **Distinguish rule from intuition.** Never present an analogy, heuristic, implementation tendency, or rule of thumb as a guarantee of the language, protocol, or system.
- **Claim discipline.** Keep specified, implementation-defined, unspecified, undefined, de facto, common-implementation, historical, recommended, and opinion visibly distinct, and keep modal force (*must*, *may*, *typically*, *usually*, *not guaranteed*). Never silently convert one into another. See [references/style-guide.md](references/style-guide.md) section 12.
- **Preserve machine-sensitive text exactly:** identifiers, filenames, paths, URLs, commands, flags, config keys, version strings, keywords, literal error messages. Put them in inline code.
- **Preserve the teaching strategy.** Examples, counterexamples, analogies, warnings, questions, and deliberate repetition are usually there on purpose.
- **Controlled personality.** First and second person, a rhetorical question, mild humor, a memorable analogy are welcome when they sharpen understanding and the source or task supports them — never as decoration. Avoid *simply*, *obviously*, *trivial*, *easy* when the matter is not.
- **Do not infantilize the reader**, and do not inflate plain points into academic prose.

Default intensity is **Level 2 — technical book**: continuous explanatory prose, strong pedagogical structure, occasional rhetorical questions, a visible but disciplined voice. Level 1 (restrained professional) and Level 3 (conversational, distinctive) are defined in the style guide; use them only when asked.

## Chapter structure

A chapter for this course is an H1 title, an introduction, numbered body sections, and then **five mandatory closing sections with exact names and order**:

1. `## Rules worth keeping in working memory`
2. `## Diagnostics, useful compiler settings and extensions`
3. `## Standards timeline`
4. `## Migration note for Java / Python / C# developers`
5. `## Further reading`

The full skeleton, with the expected prose-then-structure format for each closing section, is in [assets/chapter-template.md](assets/chapter-template.md). Copy it to `NN_chapter/NN_section/README.md`, replace every placeholder, and delete the HTML comments. Tables are the right tool for the closing sections' comparative data (compiler flags, sanitizers, language analogs, pitfalls), but each table is introduced and followed by prose; the timeline is written as short paragraphs per standard, not bare bullet lists.

## Code in this repository

Examples follow the conventions in the repository `CLAUDE.md`: code in `namespace cpp { ... }`, small static functions each demonstrating one fact, called from `main()`; compile-time facts with `static_assert`, runtime facts with `assert`; building and running the example (Debug build, so asserts are live) is how it is tested. Preserve code exactly unless the task includes correction or modernization. When a chapter's prose and its example disagree, fix the disagreement, do not hide it.

If `CLAUDE.md` says chapter READMEs are concise bullet lists, treat that as a description of the earlier drafts; the chapter-writing instruction from the user and this skill govern new and rewritten chapters.

## Working method

1. Identify the audience and assumed level (default: junior-to-mid C++ developers who want experienced-level insight).
2. Identify and protect machine-sensitive text.
3. List the exact technical claims, qualifications, guarantees, and exceptions that must survive.
4. Find the teaching structure: concept, motivation, example, contrast, pitfall, conclusion.
5. Write or rewrite as natural technical English, keeping established terminology consistent.
6. Improve paragraph flow without flattening the author's reasoning; keep useful examples, analogies, warnings, informal remarks.
7. Re-check every strong claim for accidental strengthening.
8. Run the quality checklist below, and verify any code by building it.

## Hard constraints

Do not alter code, commands, identifiers, or paths unless asked. Do not invent facts, guarantees, or implementation details. Do not turn typical behavior into guaranteed behavior, or opinion into fact. Do not remove edge cases that carry the argument. Do not replace precise terms with decorative synonyms. Do not make expert material simplistic or plain material academic. Do not convert explanatory prose into a procedural manual or a stack of bullets. Do not sacrifice correctness for readability.

## Quality checklist

1. Is every technical claim preserved accurately, with its qualification?
2. Are guarantees, tendencies, opinions, and implementation details clearly distinguished?
3. Are identifiers, code, and terms correct and consistent?
4. Does each section explain before it shows, and interpret after it shows?
5. Is prose the default and are lists/tables used only where they genuinely help?
6. Is the audience respected, neither over-explained nor left behind?
7. Are examples and analogies technically safe?
8. Is informality controlled and purposeful?
9. Does the chapter read continuously, and does it end with the five closing sections in order?
10. Would an experienced practitioner find it both credible and readable?

## Files

- [references/style-guide.md](references/style-guide.md) — the complete style guide: scope, tone, vocabulary, syntax, claim discipline, audience, intensity levels, relationship to other styles.
- [references/examples.md](references/examples.md) — calibration examples, including documentation-versus-literature contrasts and prose-before-code.
- [assets/chapter-template.md](assets/chapter-template.md) — the chapter skeleton.
