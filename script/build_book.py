#!/usr/bin/env python3
"""Build a Markdown book part from chapter README files.

Each immediate numbered subdirectory of the input directory is treated as a
chapter. Its README.md is copied to the output directory and renamed from the
chapter's number and H1 title.

The build is deliberately non-fatal for chapter-format problems: validation
issues are printed as warnings, while readable chapters are still copied.
"""

from __future__ import annotations

import argparse
import re
import shutil
import sys
import unicodedata
from dataclasses import dataclass
from pathlib import Path

CHAPTER_DIR_RE = re.compile(r"^(?P<number>\d{2})(?:_|$)")
ATX_HEADING_RE = re.compile(r"^(?P<marks>#{1,6})[ \t]+(?P<title>.*?)[ \t]*#*[ \t]*$")
FENCE_RE = re.compile(r"^[ \t]{0,3}(?P<fence>`{3,}|~{3,})")
NUMBER_PREFIX_RE = re.compile(r"^(?P<number>\d+(?:\.\d+)*)\.(?:[ \t]+|$)")
H1_NUMBER_PREFIX_RE = re.compile(r"^\d{1,2}\.(?:[ \t]+|$)")
CPP_VERSION_SUFFIX_RE = re.compile(
    r"\s*\(\s*C\+\+\d{2}(?:\s*(?:→|->|–|-|to)\s*C\+\+\d{2})?\s*\)\s*$",
    re.IGNORECASE,
)


@dataclass(frozen=True)
class Heading:
    level: int
    title: str
    line: int


def warn(chapter: Path, message: str) -> None:
    """Print one chapter validation warning."""
    print(f"WARNING: {chapter}: {message}", file=sys.stderr)


def parse_headings(text: str) -> list[Heading]:
    """Return ATX headings outside fenced code blocks."""
    headings: list[Heading] = []
    fence_char: str | None = None
    fence_length = 0

    for line_number, line in enumerate(text.splitlines(), start=1):
        fence_match = FENCE_RE.match(line)
        if fence_match:
            fence = fence_match.group("fence")
            current_char = fence[0]
            current_length = len(fence)

            if fence_char is None:
                fence_char = current_char
                fence_length = current_length
                continue

            if current_char == fence_char and current_length >= fence_length:
                fence_char = None
                fence_length = 0
                continue

        if fence_char is not None:
            continue

        match = ATX_HEADING_RE.match(line)
        if match:
            headings.append(
                Heading(
                    level=len(match.group("marks")),
                    title=match.group("title").strip(),
                    line=line_number,
                )
            )

    return headings


def chapter_title_from_h1(h1: Heading) -> str:
    """Return the display title used to form the generated filename."""
    title = H1_NUMBER_PREFIX_RE.sub("", h1.title, count=1).strip()
    title = CPP_VERSION_SUFFIX_RE.sub("", title).strip()
    return title


def filename_component(title: str) -> str:
    """Convert a chapter title to a punctuation-free underscore-separated name.

    Unicode letters and digits are retained. '+' is retained because it is part
    of the language name C++. Other punctuation and symbols become separators.
    """
    pieces: list[str] = []
    previous_was_separator = False

    for char in title:
        if char.isalnum() or char == "+":
            pieces.append(char)
            previous_was_separator = False
            continue

        category = unicodedata.category(char)
        if char.isspace() or category[0] in {"P", "S"}:
            if pieces and not previous_was_separator:
                pieces.append("_")
                previous_was_separator = True
            continue

        if pieces and not previous_was_separator:
            pieces.append("_")
            previous_was_separator = True

    return "".join(pieces).strip("_")


def numeric_prefix(heading: Heading) -> tuple[int, ...] | None:
    """Parse the dotted numeric prefix at the beginning of a heading."""
    match = NUMBER_PREFIX_RE.match(heading.title)
    if not match:
        return None
    return tuple(int(part) for part in match.group("number").split("."))


def validate_headings(chapter_dir: Path, chapter_number: int, headings: list[Heading]) -> None:
    """Warn about Markdown hierarchy, depth, and numbering violations."""
    if not headings:
        warn(chapter_dir, "README.md contains no ATX headings")
        return

    if headings[0].level != 1:
        warn(
            chapter_dir,
            f"first heading is H{headings[0].level} on line {headings[0].line}; chapter must start with H1",
        )

    h1_headings = [heading for heading in headings if heading.level == 1]
    if len(h1_headings) != 1:
        warn(chapter_dir, f"expected exactly one H1, found {len(h1_headings)}")

    max_level = max(heading.level for heading in headings)
    if max_level < 3:
        warn(chapter_dir, f"maximum heading depth is H{max_level}; expected at least H3")

    previous = headings[0]
    for heading in headings[1:]:
        if heading.level > previous.level + 1:
            warn(
                chapter_dir,
                f"heading level jumps from H{previous.level} on line {previous.line} "
                f"to H{heading.level} on line {heading.line}",
            )
        previous = heading

    # Track the most recent valid numeric prefix at every active level. This lets
    # us verify that e.g. H3 "9.2.1" really belongs below H2 "9.2".
    active_prefixes: dict[int, tuple[int, ...]] = {}

    for heading in headings:
        prefix = numeric_prefix(heading)
        if prefix is None:
            warn(
                chapter_dir,
                f"H{heading.level} on line {heading.line} is not numbered: {heading.title!r}",
            )
            active_prefixes.pop(heading.level, None)
            continue

        expected_components = heading.level
        if len(prefix) != expected_components:
            warn(
                chapter_dir,
                f"H{heading.level} on line {heading.line} has number "
                f"{'.'.join(map(str, prefix))}; expected {expected_components} component(s)",
            )

        if not prefix or prefix[0] != chapter_number:
            warn(
                chapter_dir,
                f"H{heading.level} on line {heading.line} belongs to chapter "
                f"{prefix[0] if prefix else '?'}; expected chapter {chapter_number}",
            )

        if heading.level == 1:
            h1_match = re.match(r"^(?P<number>\\d{2})(?:\\.(?=[ \\t]|$))?(?:[ \\t]+|$)", heading.title)
            expected_h1 = f"{chapter_number:02d}"
            if h1_match is None or h1_match.group("number") != expected_h1:
                warn(
                    chapter_dir,
                    f"H1 on line {heading.line} must start with two-digit chapter number {expected_h1!r}",
                )
        elif heading.level > 1:
            parent_prefix = active_prefixes.get(heading.level - 1)
            if parent_prefix is not None and prefix[: heading.level - 1] != parent_prefix:
                warn(
                    chapter_dir,
                    f"H{heading.level} number {'.'.join(map(str, prefix))} on line "
                    f"{heading.line} does not match parent "
                    f"{'.'.join(map(str, parent_prefix))}",
                )

        active_prefixes[heading.level] = prefix
        for deeper_level in list(active_prefixes):
            if deeper_level > heading.level:
                del active_prefixes[deeper_level]


def find_chapters(input_dir: Path) -> list[tuple[int, Path]]:
    """Find immediate NN_* chapter directories in numeric order."""
    chapters: list[tuple[int, Path]] = []

    for child in input_dir.iterdir():
        if not child.is_dir():
            continue

        match = CHAPTER_DIR_RE.match(child.name)
        if match:
            chapters.append((int(match.group("number")), child))

    chapters.sort(key=lambda item: (item[0], item[1].name))
    return chapters


def build_book(input_dir: Path, output_dir: Path) -> int:
    """Validate and copy all chapter README files."""
    chapters = find_chapters(input_dir)
    if not chapters:
        print(f"Error: no numbered chapter directories found in {input_dir}", file=sys.stderr)
        return 1

    output_dir.mkdir(parents=True, exist_ok=True)
    copied = 0
    used_names: set[str] = set()

    for chapter_number, chapter_dir in chapters:
        readme = chapter_dir / "README.md"
        if not readme.is_file():
            warn(chapter_dir, "README.md is missing; chapter skipped")
            continue

        try:
            text = readme.read_text(encoding="utf-8")
        except (OSError, UnicodeError) as exc:
            warn(chapter_dir, f"cannot read README.md: {exc}; chapter skipped")
            continue

        headings = parse_headings(text)
        validate_headings(chapter_dir, chapter_number, headings)

        if headings and headings[0].level == 1:
            title = chapter_title_from_h1(headings[0])
        else:
            title = chapter_dir.name[3:] if len(chapter_dir.name) > 3 else chapter_dir.name

        safe_title = filename_component(title)
        if not safe_title:
            warn(chapter_dir, "chapter title produces an empty filename; chapter skipped")
            continue

        output_name = f"{chapter_number:02d}.{safe_title}.md"
        if output_name in used_names:
            warn(chapter_dir, f"generated filename collision: {output_name}; chapter skipped")
            continue

        used_names.add(output_name)
        destination = output_dir / output_name
        shutil.copyfile(readme, destination)
        copied += 1
        print(f"{readme} -> {destination}")

    print(f"Copied {copied} chapter(s) to {output_dir}")
    return 0 if copied else 1


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Copy chapter README.md files into a numbered Markdown book directory."
    )
    parser.add_argument(
        "input_directory",
        type=Path,
        help="Book part directory, for example 01_low_level",
    )
    parser.add_argument(
        "output_directory",
        type=Path,
        help="Destination directory, for example Advanced_C++_Book_01",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    input_dir = args.input_directory.expanduser().resolve()
    output_dir = args.output_directory.expanduser().resolve()

    if not input_dir.is_dir():
        print(f"Error: input directory does not exist or is not a directory: {input_dir}", file=sys.stderr)
        return 1

    return build_book(input_dir, output_dir)


if __name__ == "__main__":
    sys.exit(main())
