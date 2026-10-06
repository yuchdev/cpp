#!/usr/bin/env python3
"""Compile/decompile Markdown book chapters from chapter README.md files."""

from __future__ import annotations

import argparse
import json
import re
import sys
import unicodedata
from dataclasses import dataclass
from pathlib import Path

CHAPTER_DIR_RE = re.compile(r"^(?P<number>\d{2})(?:_|$)")
ATX_HEADING_RE = re.compile(r"^(?P<marks>#{1,6})[ \t]+(?P<title>.*?)[ \t]*#*[ \t]*$")
FENCE_RE = re.compile(r"^[ \t]{0,3}(?P<fence>\`{3,}|~{3,})")
LEADING_NUMBER_RE = re.compile(r"^\s*\d+(?:\.\d+)*\.?\s*")
NUMBER_PREFIX_RE = re.compile(
    r"^(?P<number>\d+(?:\.\d+)*)(?:\.(?=[ \t]|$))?(?:[ \t]+|$)"
)
H1_NUMBER_PREFIX_RE = re.compile(r"^\d{1,2}(?:\.(?=[ \t]|$))?(?:[ \t]+|$)")
CPP_VERSION_SUFFIX_RE = re.compile(
    r"\s*\(\s*C\+\+\d{2}(?:\s*(?:→|->|–|-|to)\s*C\+\+\d{2})?\s*\)\s*$",
    re.IGNORECASE,
)
MANIFEST_NAME = "manifest.json"


@dataclass(frozen=True)
class Heading:
    level: int
    title: str
    line: int


def warn(chapter: Path | str, message: str) -> None:
    print(f"WARNING: {chapter}: {message}", file=sys.stderr)


def iter_markdown_lines(text: str):
    """Yield (line_number, line, heading_match) while ignoring fenced code."""
    fence_char: str | None = None
    fence_length = 0

    for line_number, line in enumerate(text.splitlines(), start=1):
        fence_match = FENCE_RE.match(line)
        if fence_match:
            fence = fence_match.group("fence")
            char = fence[0]
            length = len(fence)
            if fence_char is None:
                fence_char, fence_length = char, length
                yield line_number, line, None
                continue
            if char == fence_char and length >= fence_length:
                fence_char, fence_length = None, 0
                yield line_number, line, None
                continue

        heading = None if fence_char is not None else ATX_HEADING_RE.match(line)
        yield line_number, line, heading


def parse_headings(text: str) -> list[Heading]:
    headings: list[Heading] = []
    for line_number, _line, match in iter_markdown_lines(text):
        if match:
            headings.append(
                Heading(len(match.group("marks")), match.group("title").strip(), line_number)
            )
    return headings


def chapter_title_from_h1(h1: Heading) -> str:
    title = H1_NUMBER_PREFIX_RE.sub("", h1.title, count=1).strip()
    return CPP_VERSION_SUFFIX_RE.sub("", title).strip()


def filename_component(title: str) -> str:
    pieces: list[str] = []
    separator = False
    for char in title:
        if char.isalnum() or char == "+":
            pieces.append(char)
            separator = False
        elif char.isspace() or unicodedata.category(char)[0] in {"P", "S"}:
            if pieces and not separator:
                pieces.append("_")
                separator = True
        elif pieces and not separator:
            pieces.append("_")
            separator = True
    return "".join(pieces).strip("_")


def numeric_prefix(heading: Heading) -> tuple[int, ...] | None:
    match = NUMBER_PREFIX_RE.match(heading.title)
    if not match:
        return None
    return tuple(int(part) for part in match.group("number").split("."))


def validate_headings(label: Path | str, chapter_number: int, headings: list[Heading]) -> int:
    warnings = 0

    def issue(message: str) -> None:
        nonlocal warnings
        warnings += 1
        warn(label, message)

    if not headings:
        issue("contains no ATX headings")
        return warnings

    if headings[0].level != 1:
        issue(
            f"first heading is H{headings[0].level} on line {headings[0].line}; "
            "chapter must start with H1"
        )

    h1s = [heading for heading in headings if heading.level == 1]
    if len(h1s) != 1:
        issue(f"expected exactly one H1, found {len(h1s)}")

    max_level = max(heading.level for heading in headings)
    if max_level < 3:
        issue(f"maximum heading depth is H{max_level}; expected at least H3")

    previous = headings[0]
    for heading in headings[1:]:
        if heading.level > previous.level + 1:
            issue(
                f"heading level jumps from H{previous.level} on line {previous.line} "
                f"to H{heading.level} on line {heading.line}"
            )
        previous = heading

    active: dict[int, tuple[int, ...]] = {}
    for heading in headings:
        prefix = numeric_prefix(heading)
        if prefix is None:
            issue(f"H{heading.level} on line {heading.line} is not numbered: {heading.title!r}")
            active.pop(heading.level, None)
            continue

        if len(prefix) != heading.level:
            issue(
                f"H{heading.level} on line {heading.line} has number "
                f"{'.'.join(map(str, prefix))}; expected {heading.level} component(s)"
            )

        if prefix[0] != chapter_number:
            issue(
                f"H{heading.level} on line {heading.line} belongs to chapter {prefix[0]}; "
                f"expected chapter {chapter_number}"
            )

        if heading.level == 1:
            expected = f"{chapter_number:02d}"
            if not re.match(rf"^{re.escape(expected)}(?:\.(?=[ \t]|$))?(?:[ \t]+|$)", heading.title):
                issue(f"H1 on line {heading.line} must start with two-digit chapter number {expected!r}")
        else:
            parent = active.get(heading.level - 1)
            if parent is not None and prefix[: heading.level - 1] != parent:
                issue(
                    f"H{heading.level} number {'.'.join(map(str, prefix))} on line "
                    f"{heading.line} does not match parent {'.'.join(map(str, parent))}"
                )

        active[heading.level] = prefix
        for deeper in list(active):
            if deeper > heading.level:
                del active[deeper]

    return warnings


def renumber_markdown(text: str, chapter_number: int) -> str:
    """Remove existing heading numbers and rebuild numbering from the heading tree."""
    counters = [0] * 6
    output: list[str] = []

    for _line_number, line, match in iter_markdown_lines(text):
        if not match:
            output.append(line)
            continue

        level = len(match.group("marks"))
        title = LEADING_NUMBER_RE.sub("", match.group("title").strip(), count=1).strip()

        if level == 1:
            counters = [0] * 6
            counters[0] = chapter_number
            number = f"{chapter_number:02d}."
        else:
            if counters[0] == 0:
                counters[0] = chapter_number
            counters[level - 1] += 1
            for index in range(level, 6):
                counters[index] = 0

            # If malformed input skips a level, create the missing parent as 1 so
            # the result is still deterministically numbered; --check will warn
            # about the structural jump separately.
            for index in range(1, level - 1):
                if counters[index] == 0:
                    counters[index] = 1

            number = ".".join(str(counters[index]) for index in range(level))

        output.append(f"{'#' * level} {number} {title}".rstrip())

    return "\n".join(output).rstrip() + "\n"


def find_chapters(input_dir: Path) -> list[tuple[int, Path]]:
    chapters: list[tuple[int, Path]] = []
    for child in input_dir.iterdir():
        if child.is_dir():
            match = CHAPTER_DIR_RE.match(child.name)
            if match:
                chapters.append((int(match.group("number")), child))
    return sorted(chapters, key=lambda item: (item[0], item[1].name))


def normalize_chunk(text: str) -> str:
    """Return a chapter ending with exactly one empty line."""
    return text.rstrip() + "\n\n"


def relative_source(path: Path) -> str:
    try:
        return path.resolve().relative_to(Path.cwd().resolve()).as_posix()
    except ValueError:
        return str(path.resolve())


def compile_book(input_dir: Path, output_dir: Path, do_renumber: bool, do_check: bool) -> int:
    chapters = find_chapters(input_dir)
    if not chapters:
        print(f"Error: no numbered chapter directories found in {input_dir}", file=sys.stderr)
        return 1

    output_dir.mkdir(parents=True, exist_ok=True)
    manifest_entries: list[dict[str, object]] = []
    book_chunks: list[str] = []
    used_names: set[str] = set()
    copied = 0
    total_warnings = 0

    for chapter_number, chapter_dir in chapters:
        readme = chapter_dir / "README.md"
        if not readme.is_file():
            warn(chapter_dir, "README.md is missing; chapter skipped")
            total_warnings += 1
            continue

        try:
            text = readme.read_text(encoding="utf-8")
        except (OSError, UnicodeError) as exc:
            warn(chapter_dir, f"cannot read README.md: {exc}; chapter skipped")
            total_warnings += 1
            continue

        if do_renumber:
            text = renumber_markdown(text, chapter_number)

        headings = parse_headings(text)

        if headings and headings[0].level == 1:
            title = chapter_title_from_h1(headings[0])
        else:
            title = chapter_dir.name[3:] if len(chapter_dir.name) > 3 else chapter_dir.name

        safe_title = filename_component(title)
        if not safe_title:
            warn(chapter_dir, "chapter title produces an empty filename; chapter skipped")
            total_warnings += 1
            continue

        output_name = f"{chapter_number:02d}.{safe_title}.md"
        if output_name in used_names:
            warn(chapter_dir, f"generated filename collision: {output_name}; chapter skipped")
            total_warnings += 1
            continue
        used_names.add(output_name)

        chapter_text = normalize_chunk(text)
        destination = output_dir / output_name
        destination.write_text(chapter_text, encoding="utf-8")
        book_chunks.append(chapter_text)

        manifest_entries.append(
            {
                "chapter": chapter_number,
                "compiled_file": output_name,
                "source_readme": relative_source(readme),
                "source_readme_absolute": str(readme.resolve()),
            }
        )
        copied += 1
        print(f"{readme} -> {destination}")

    book_name = f"{output_dir.name}.md"
    book_path = output_dir / book_name
    book_path.write_text("".join(book_chunks), encoding="utf-8")

    if do_check:
        compiled_chunks = split_book(book_path.read_text(encoding="utf-8"))
        for entry, chunk in zip(manifest_entries, compiled_chunks):
            chapter_number = int(entry["chapter"])
            total_warnings += validate_headings(
                f"{book_path} [chapter {chapter_number:02d}]",
                chapter_number,
                parse_headings(chunk),
            )

    manifest = {
        "version": 1,
        "input_directory": relative_source(input_dir),
        "book_file": book_name,
        "chapters": manifest_entries,
    }
    (output_dir / MANIFEST_NAME).write_text(
        json.dumps(manifest, indent=2, ensure_ascii=False) + "\n",
        encoding="utf-8",
    )

    print(f"Built {copied} chapter(s)")
    print(f"Book: {book_path}")
    print(f"Manifest: {output_dir / MANIFEST_NAME}")
    if do_check:
        print(f"Check warnings: {total_warnings}")
    return 0 if copied else 1


def split_book(text: str) -> list[str]:
    """Split a compiled book at H1 headings outside fenced code blocks."""
    lines = text.splitlines()
    starts: list[int] = []
    for line_number, _line, match in iter_markdown_lines(text):
        if match and len(match.group("marks")) == 1:
            starts.append(line_number - 1)

    if not starts:
        return []

    chunks: list[str] = []
    for index, start in enumerate(starts):
        end = starts[index + 1] if index + 1 < len(starts) else len(lines)
        chunks.append(normalize_chunk("\n".join(lines[start:end])))
    return chunks


def resolve_source_path(entry: dict[str, object]) -> Path:
    absolute = entry.get("source_readme_absolute")
    if isinstance(absolute, str):
        return Path(absolute).expanduser()

    source = entry.get("source_readme")
    if not isinstance(source, str):
        raise ValueError("manifest chapter has no source_readme")
    path = Path(source).expanduser()
    return path if path.is_absolute() else (Path.cwd() / path)


def decompile_book(book_dir: Path) -> int:
    manifest_path = book_dir / MANIFEST_NAME
    if not manifest_path.is_file():
        print(f"Error: manifest not found: {manifest_path}", file=sys.stderr)
        return 1

    try:
        manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    except (OSError, UnicodeError, json.JSONDecodeError) as exc:
        print(f"Error: cannot read manifest: {exc}", file=sys.stderr)
        return 1

    entries = manifest.get("chapters", [])
    book_file = manifest.get("book_file")
    if not isinstance(entries, list) or not isinstance(book_file, str):
        print("Error: invalid manifest format", file=sys.stderr)
        return 1

    book_path = book_dir / book_file
    if not book_path.is_file():
        print(f"Error: compiled book not found: {book_path}", file=sys.stderr)
        return 1

    chunks = split_book(book_path.read_text(encoding="utf-8"))
    if len(chunks) != len(entries):
        print(
            f"Error: book contains {len(chunks)} H1 chapter chunk(s), "
            f"manifest contains {len(entries)} chapter(s)",
            file=sys.stderr,
        )
        return 1

    written = 0
    for entry, chunk in zip(entries, chunks):
        if not isinstance(entry, dict):
            print("Error: invalid chapter entry in manifest", file=sys.stderr)
            return 1

        expected_chapter = entry.get("chapter")
        headings = parse_headings(chunk)
        actual_prefix = numeric_prefix(headings[0]) if headings else None
        actual_chapter = actual_prefix[0] if actual_prefix else None
        if not isinstance(expected_chapter, int) or actual_chapter != expected_chapter:
            print(
                f"Error: manifest expects chapter {expected_chapter}, "
                f"but corresponding book chunk starts with chapter {actual_chapter}",
                file=sys.stderr,
            )
            return 1

        try:
            destination = resolve_source_path(entry)
        except ValueError as exc:
            print(f"Error: {exc}", file=sys.stderr)
            return 1

        destination.parent.mkdir(parents=True, exist_ok=True)
        destination.write_text(normalize_chunk(chunk), encoding="utf-8")
        written += 1
        print(f"{book_path} -> {destination}")

    print(f"Decompiled {written} chapter(s)")
    return 0


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Compile chapter README files into a Markdown book, or decompile it back."
    )
    mode = parser.add_mutually_exclusive_group()
    mode.add_argument(
        "--compile",
        action="store_true",
        help="Compile INPUT_DIRECTORY into OUTPUT_DIRECTORY (default mode).",
    )
    mode.add_argument(
        "--decompile",
        action="store_true",
        help="Decompile OUTPUT_DIRECTORY book back to README.md files using manifest.json.",
    )
    parser.add_argument(
        "--check",
        action="store_true",
        help="Validate heading hierarchy and numbering after compilation/renumbering.",
    )
    parser.add_argument(
        "--renumber",
        action="store_true",
        help="Remove existing heading numbers and renumber the compiled chapters before --check.",
    )
    parser.add_argument(
        "paths",
        nargs="+",
        type=Path,
        metavar="PATH",
        help="Compile: INPUT_DIRECTORY OUTPUT_DIRECTORY. Decompile: OUTPUT_DIRECTORY.",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()

    if args.decompile:
        if args.check or args.renumber:
            print("Error: --decompile is incompatible with --check and --renumber", file=sys.stderr)
            return 2
        if len(args.paths) != 1:
            print("Error: --decompile requires exactly one OUTPUT_DIRECTORY", file=sys.stderr)
            return 2
        book_dir = args.paths[0].expanduser().resolve()
        if not book_dir.is_dir():
            print(f"Error: book directory does not exist: {book_dir}", file=sys.stderr)
            return 1
        return decompile_book(book_dir)

    if len(args.paths) != 2:
        print(
            "Error: --compile requires INPUT_DIRECTORY and OUTPUT_DIRECTORY",
            file=sys.stderr,
        )
        return 2

    input_dir = args.paths[0].expanduser().resolve()
    output_dir = args.paths[1].expanduser().resolve()
    if not input_dir.is_dir():
        print(f"Error: input directory does not exist: {input_dir}", file=sys.stderr)
        return 1

    return compile_book(input_dir, output_dir, args.renumber, args.check)


if __name__ == "__main__":
    sys.exit(main())
