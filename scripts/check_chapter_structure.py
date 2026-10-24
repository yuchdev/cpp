#!/usr/bin/env python3
"""Validate Book 01 technical-literature chapter contracts (stdlib only)."""
from __future__ import annotations

import argparse
from pathlib import Path
import re
import sys

ENDING = (
    "Rules worth keeping in working memory",
    "Diagnostics, useful compiler settings and extensions",
    "Standards timeline",
    "Migration note for Java / Python / C# developers",
    "Further reading",
)
YEARS = ("C++98/03", "C++11", "C++14", "C++17", "C++20", "C++23", "C++26")
STUBS = re.compile(r"\b(?:TODO|TBD|FIXME|PLACEHOLDER|fill this in)\b|<placeholder>", re.I)
HEADING = re.compile(r"^(#{1,6})\s+(.+?)\s*$", re.M)
FENCE_OPEN = re.compile(r"^ {0,3}(`{3,}|~{3,})(.*)$")
FENCE_CLOSE = re.compile(r"^ {0,3}(`{3,}|~{3,})[ \t]*$")
CHAPTER = re.compile(r"^(\d{2})_")


def outside_fences(body: str) -> tuple[str, bool]:
    visible: list[str] = []
    opening: tuple[str, int] | None = None
    for line in body.splitlines(keepends=True):
        if opening is None:
            match = FENCE_OPEN.match(line)
            if match and not (match.group(1).startswith("`") and "`" in match.group(2)):
                delimiter = match.group(1)
                opening = (delimiter[0], len(delimiter))
                visible.append("\n" if line.endswith("\n") else "")
            else:
                visible.append(line)
            continue
        match = FENCE_CLOSE.match(line)
        if (match and match.group(1)[0] == opening[0]
                and len(match.group(1)) >= opening[1]):
            opening = None
        visible.append("\n" if line.endswith("\n") else "")
    return "".join(visible), opening is not None


def validate(path: Path) -> list[str]:
    body = path.read_text(encoding="utf-8")
    visible_body, unbalanced_fences = outside_fences(body)
    errors: list[str] = []
    chapter = CHAPTER.match(path.parent.name)
    if not chapter:
        return [f"{path}: not a numbered chapter directory"]
    headings = [(len(m.group(1)), m.group(2)) for m in HEADING.finditer(visible_body)]
    if not headings or headings[0][0] != 1 or not headings[0][1].startswith(chapter.group(1)):
        errors.append("H1 must start with chapter number")
    h2 = [h for level, h in headings if level == 2]
    if h2[-len(ENDING):] != list(ENDING):
        errors.append("final five H2 sections must match the chapter template in order")
    for h in ENDING:
        if h2.count(h) != 1:
            errors.append(f"expected exactly one H2: {h}")
    numeric = [int(m.group(1)) for h in h2 if (m := re.match(r"^(\d+)\.\s", h))]
    if not numeric or numeric != list(range(1, len(numeric) + 1)):
        errors.append(f"numbered H2 sections not consecutive: {numeric}")
    found_stubs = STUBS.findall(body)
    if found_stubs:
        errors.append(f"contains stub markers: {found_stubs[:3]}")
    if unbalanced_fences:
        errors.append("unbalanced fenced code blocks")
    timeline_start = visible_body.find("\n## Standards timeline")
    migration_start = visible_body.find("\n## Migration note")
    if timeline_start >= 0 and migration_start > timeline_start:
        timeline = visible_body[timeline_start:migration_start]
        missing = [std for std in YEARS if not re.search(r"^### " + re.escape(std) + r"\b", timeline, re.M)]
        if missing:
            errors.append(f"standards timeline missing: {', '.join(missing)}")
    if len(body) < 7000:
        errors.append("chapter is unexpectedly short for advanced technical literature")
    for item in errors:
        print(f"{path}: {item}", file=sys.stderr)
    if not errors:
        print(f"PASS {path} ({len(numeric)} numbered sections)")
    return errors

def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("paths", nargs="*", type=Path,
                    help="README.md paths; defaults to chapters 01–13")
    args = ap.parse_args()
    paths = args.paths or sorted(Path("01_low_level").glob("[0-9][0-9]_*/README.md"))
    if not paths:
        print("No chapter READMEs found", file=sys.stderr)
        return 2
    failures = sum(bool(validate(p)) for p in paths)
    return 1 if failures else 0

if __name__ == "__main__":
    raise SystemExit(main())
