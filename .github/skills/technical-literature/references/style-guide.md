# Technical Literature Style Guide

Priority order for all decisions: meaning → emotional/functional intent → readability → style coloring. When translating or rewriting, apply this guide to the requested style; when writing originally, apply it as the house style.

## 1. Purpose

Technical Literature is for books, chapters, articles, tutorials, educational materials, engineering essays, and other long-form technical prose intended to explain technology rather than merely document how to operate it.

It combines the precision and terminology discipline of technical documentation with the readability, continuity, and authorial flexibility of professional technical writing. Unlike strict technical documentation, it may use analogy, historical context, rhetorical emphasis, humor, informal asides, and a recognizable authorial voice when these improve understanding and are appropriate to the source.

## 2. One-sentence definition

Translate or rewrite into accurate, readable technical prose that preserves technical meaning and terminology while allowing explanation, narrative flow, examples, analogy, and controlled informality when useful.

## 3. When to use

* technical books and book chapters,
* programming and engineering literature,
* advanced tutorials,
* educational articles,
* technology essays,
* architecture and design discussions,
* technical blog posts,
* explanatory material for experienced practitioners,
* conference or lecture material converted to prose,
* historical or conceptual discussions of technology,
* mixed technical/professional prose where strict documentation style would be too dry.

## 4. When not to use

Do not use as the main style for:

* API reference documentation,
* operational runbooks,
* exact installation procedures,
* CLI reference material,
* legal or compliance documents,
* strict academic papers,
* marketing copy,
* literary fiction,
* UI microcopy.

For documents that contain both explanatory prose and exact procedures, Technical Literature may be the primary style while Technical Documentation rules govern commands, identifiers, procedural blocks, and machine-sensitive text.

## 5. Tone targets

* Technically authoritative.
* Clear and explanatory.
* Professional by default.
* Natural rather than bureaucratic.
* Engaging without becoming journalistic hype.
* Capable of controlled informality.
* Comfortable with an expert authorial voice.
* Precise without sounding like reference documentation.

The reader should feel that a knowledgeable engineer or technical author is explaining the subject, not that a specification is being generated.

## 6. Core principles

### Technical truth comes first

Do not improve style at the cost of technical correctness. Preserve conditions, limitations, uncertainty, distinctions, and edge cases.

### Explain, do not merely state

When the source explains why something works, why it matters, or why a common intuition fails, preserve that explanatory structure. Do not flatten conceptual discussion into documentation-style instructions.

### Preserve machine-sensitive text

As in Technical Documentation, do not translate or casually alter:

* code identifiers,
* filenames,
* directory paths,
* URLs,
* CLI commands,
* configuration keys,
* environment variables,
* placeholders,
* version strings,
* language keywords,
* literal error messages unless explicitly requested.

### Preserve the author's teaching strategy

Examples, counterexamples, analogies, historical remarks, warnings, questions, and deliberate repetitions may be pedagogically important. Do not remove them merely to make the text shorter.

### Allow controlled personality

Technical literature may sound human. When supported by the source or requested style level, allow:

* first and second person,
* rhetorical questions,
* short informal remarks,
* mild humor,
* idiomatic phrasing,
* memorable analogies,
* opinionated but technically grounded observations.

Do not introduce these merely for decoration when the source is neutral.

### Distinguish rule from intuition

Technical writing often moves between exact rules and simplified mental models. Preserve that distinction. Do not present an analogy, heuristic, implementation tendency, or rule of thumb as a language, protocol, or system guarantee.

### Prose first, structure second

Reasoning lives in paragraphs. An extended paragraph precedes each example: it states the idea, exposes the intuition that fails, names the governing rule, and says what the code will show. Lists and tables are reserved for closed enumerations and genuine side-by-side comparison, and are themselves introduced and interpreted in prose.

## 7. Vocabulary preferences

Prefer:

* precise established technical terminology,
* concrete verbs,
* conventional names used by practitioners,
* explain, demonstrate, observe, consider, compare, distinguish,
* guarantee, require, permit, imply,
* implementation, behavior, constraint, invariant, trade-off, edge case,
* rule of thumb, in practice, conceptually.

Informal technical vocabulary is allowed when normal for the audience:

* bug, crash, gotcha, footgun,
* bottleneck, hot path, under the hood,
* cheap or expensive when computational cost is meant.

Use such vocabulary deliberately, not as filler.

Avoid:

* unnecessary academic inflation,
* corporate buzzwords,
* vague claims such as simply, obviously, trivial, or easy when the matter is not genuinely simple,
* excessive slang,
* marketing superlatives,
* invented terminology when an established term exists.

## 8. Syntax preferences

Use:

* short to medium sentences for core claims,
* longer sentences when they express a coherent technical relationship,
* clear paragraph progression,
* active voice where natural,
* examples immediately after difficult concepts (and an explanatory paragraph before them),
* explicit contrast for easily confused concepts,
* lists and tables when they genuinely improve comparison,
* code formatting for machine-sensitive terms,
* occasional rhetorical questions or emphatic short sentences.

Avoid:

* converting every paragraph into bullets,
* documentation-like imperative mood throughout explanatory chapters,
* monotonous sentence structure,
* deeply nested academic syntax,
* excessive conversational fragments,
* hiding important qualifications in parenthetical clutter.

Technical literature should read continuously as prose, not as a sequence of tickets, API entries, or runbook steps.

## 9. Translation and rewriting priorities

1. Technical correctness.
2. Conceptual meaning and distinctions.
3. Terminology consistency.
4. Preservation of code and machine-sensitive text.
5. Pedagogical intent.
6. Readability and flow.
7. Authorial voice.
8. Stylistic coloring.

When technical precision and elegance conflict, precision wins. When several technically correct formulations exist, prefer the one that teaches the concept most clearly.

## 10. Compression and expansion rules

Allowed:

* split overloaded sentences,
* combine short fragments when this improves technical flow,
* add small connective phrases,
* make an implicit logical relationship explicit when directly supported by the source,
* reorganize paragraph breaks,
* retain or clarify useful repetition,
* replace a source-language idiom with a natural technical equivalent,
* make an analogy natural in English while preserving its function,
* use conventional English technical terminology instead of literal calques,
* expand a bullet-list fact into connected prose, provided no claim is added that the source does not support.

Not allowed:

* invent technical facts,
* invent guarantees,
* remove exceptions or qualifications,
* strengthen implementation-dependent behavior into a standard guarantee,
* turn opinion into fact,
* add unsupported historical claims,
* replace exact terminology merely for stylistic variety,
* simplify a concept until an important distinction disappears.

## 11. Formatting rules

Use inline code formatting for:

* `ClassName`,
* `function_name`,
* `variable`,
* `keyword`,
* `ENV_VAR`,
* `--flag`,
* `/path`,
* `config.key`,
* short expressions such as `sizeof(char)`.

Use fenced code blocks for substantial examples. Preserve code exactly unless the task explicitly includes code correction or modernization.

Prefer prose around code that explains:

1. what the example demonstrates (before the code),
2. what the reader should notice,
3. what limitation or pitfall matters.

Do not narrate every obvious line of code.

## 12. Technical claim discipline

Technical literature frequently contains claims with different strengths. Preserve them.

Distinguish:

* specified behavior,
* implementation-defined behavior,
* unspecified behavior,
* undefined behavior,
* de facto convention,
* common implementation,
* historical behavior,
* recommendation,
* personal opinion,
* simplified explanatory model.

Preserve modal force carefully:

* must / shall / required,
* may / permitted,
* can / possible,
* typically / commonly,
* usually,
* often,
* sometimes,
* implementation-dependent,
* not guaranteed.

Never silently convert one category into another.

## 13. Audience handling

Assume the audience specified by the source or task.

For expert audiences:

* do not over-explain elementary concepts,
* retain precise terminology,
* emphasize obscure behavior, trade-offs, and edge cases,
* allow denser argument where useful.

For intermediate audiences (the default for this course: junior-to-mid C++ developers):

* define uncommon terms on first use,
* introduce difficult concepts progressively,
* use examples before edge cases when appropriate.

For broad audiences:

* minimize unexplained jargon,
* use analogies carefully,
* preserve technical accuracy even when simplifying.

Do not infantilize the reader.

## 14. Intensity levels

### Level 1 — Professional technical prose

Clear, restrained, and mostly formal. Suitable for engineering articles, design discussions, reports, and explanatory documentation. Personality is limited; precision and clarity dominate.

### Level 2 — Technical book

The default level. Continuous explanatory prose with strong pedagogical structure, examples, comparisons, occasional rhetorical questions, and a visible but disciplined authorial voice. Suitable for programming books, advanced tutorials, and educational chapters.

### Level 3 — Authorial technical writing

More conversational and distinctive while remaining technically rigorous. May use humor, sharp observations, memorable analogies, informal practitioner vocabulary, first/second person, and stronger rhetorical rhythm. Use only when the source or task supports such a voice. Do not turn technical literature into entertainment or opinion journalism.

## 15. Examples

See [examples.md](examples.md).

## 16. Relationship to other styles

### Technical Documentation

Use Technical Documentation when the reader's primary question is:

> What exactly must I do, configure, call, or verify?

Use Technical Literature when the primary question is:

> How does this work, why does it matter, and what should I understand about it?

Technical Literature inherits Technical Documentation's strict treatment of code, identifiers, commands, and terminology, but permits substantially more explanatory and authorial prose, and uses far fewer lists.

### Formal Academic

Use Formal Academic when structured argument, research framing, theoretical precision, and scholarly restraint dominate. Use Technical Literature when teaching and engineering understanding dominate. Technical Literature may be rigorous without sounding academic.

### Business Professional

Use Business Professional for decisions, recommendations, status, responsibilities, proposals, and workplace communication. Technical Literature may borrow its professional clarity, but its goal is understanding rather than organizational action.

### Journalistic

Journalistic style prioritizes article flow and reader engagement. Technical Literature may borrow hooks and readable transitions, but technical qualification and pedagogical completeness take priority over momentum.

### Neutral Modern

Neutral Modern remains appropriate for ordinary explanatory prose without substantial technical density. Switch to Technical Literature when terminology, code, technical distinctions, or engineering reasoning become central.

## 17. Operational algorithm

1. Identify the target audience and assumed technical level.
2. Identify machine-sensitive text and protect it.
3. Identify exact technical claims, qualifications, guarantees, and exceptions.
4. Identify the teaching structure: concept, motivation, example, contrast, pitfall, conclusion.
5. Translate or rewrite into natural technical English.
6. Preserve established terminology consistently.
7. Improve paragraph flow without flattening the author's reasoning.
8. Preserve useful examples, analogies, warnings, and informal remarks.
9. Check every strong technical claim for accidental strengthening.
10. Verify that the result is both technically defensible and readable as continuous prose.

## 18. Hard constraints

Do not:

* alter code, commands, identifiers, or paths unless explicitly requested,
* invent technical facts or implementation details,
* turn typical behavior into guaranteed behavior,
* remove edge cases that affect the argument,
* replace precise terms with decorative synonyms,
* make expert material artificially simplistic,
* make straightforward material artificially academic,
* convert explanatory prose into a procedural manual,
* introduce jokes, slang, or rhetorical flourishes that change the source voice,
* sacrifice correctness for readability.

## 19. Quality checklist

1. Is every technical claim preserved accurately?
2. Are guarantees, tendencies, opinions, and implementation details clearly distinguished?
3. Are identifiers, code, commands, and technical terms preserved correctly?
4. Does the text explain rather than merely prescribe?
5. Is the assumed audience respected?
6. Are examples and analogies technically safe?
7. Is terminology consistent?
8. Does the prose flow naturally as a chapter or article?
9. Is informality controlled and purposeful?
10. Would an experienced practitioner find the text both credible and readable?
