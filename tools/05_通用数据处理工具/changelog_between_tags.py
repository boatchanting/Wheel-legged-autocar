#!/usr/bin/env python3
"""Generate grouped changelog between two git tags (or refs)."""
from __future__ import annotations

import argparse
import re
import subprocess
import sys
from collections import OrderedDict

CATEGORY_PATTERN = re.compile(r"^【([^】]+)】\s*(.+)$")
FORMAT_PATTERNS = [
    re.compile(r"^fmt(\(.+\))?:", re.IGNORECASE),
    re.compile(r"^format(\(.+\))?:", re.IGNORECASE),
    re.compile(r"^style(\(.+\))?:", re.IGNORECASE),
    re.compile(r"^chore\(format\):", re.IGNORECASE),
]
MERGE_PATTERNS = [
    re.compile(r"^merge\b", re.IGNORECASE),
]


def run_git(cmd: list[str]) -> str:
    return subprocess.check_output(["git", *cmd], text=True).strip()


def parse_commits(from_tag: str, to_tag: str, include_merge: bool, include_format: bool) -> OrderedDict[str, list[str]]:
    log = run_git(["log", "--pretty=format:%H%x09%s", f"{from_tag}..{to_tag}"])
    groups: OrderedDict[str, list[str]] = OrderedDict()
    if not log:
        return groups

    for line in log.splitlines():
        _sha, subject = line.split("\t", 1)
        if not include_merge and any(p.search(subject) for p in MERGE_PATTERNS):
            continue
        if not include_format and any(p.search(subject) for p in FORMAT_PATTERNS):
            continue

        m = CATEGORY_PATTERN.match(subject)
        if m:
            category = m.group(1).strip()
            message = re.sub(r"^[：:：\-\s]+", "", m.group(2).strip())
        else:
            category, message = "其他", subject.strip()

        if category not in groups:
            groups[category] = []
        groups[category].append(message)

    return groups


def format_output(groups: OrderedDict[str, list[str]], markdown: bool, from_tag: str, to_tag: str) -> str:
    if markdown:
        out = [f"## Changelog ({from_tag} -> {to_tag})", ""]
        for category, items in groups.items():
            out.append(f"### {category}")
            out.extend([f"- {item}" for item in items])
            out.append("")
        if len(out) == 2:
            out.append("- 无有效变更")
        return "\n".join(out).rstrip() + "\n"

    out = [f"Changelog ({from_tag} -> {to_tag})"]
    for category, items in groups.items():
        out.append(f"\n[{category}]")
        out.extend([f"- {item}" for item in items])
    if len(out) == 1:
        out.append("\n- 无有效变更")
    return "\n".join(out) + "\n"


def main() -> int:
    parser = argparse.ArgumentParser(description="Generate changelog between two tags")
    parser.add_argument("from_tag")
    parser.add_argument("to_tag")
    parser.add_argument("--markdown", action="store_true", help="output markdown format")
    parser.add_argument("--include-merge", action="store_true", help="include merge commits")
    parser.add_argument("--include-format", action="store_true", help="include format/style commits")
    args = parser.parse_args()

    try:
        run_git(["rev-parse", "--verify", args.from_tag])
        run_git(["rev-parse", "--verify", args.to_tag])
    except subprocess.CalledProcessError:
        print("Error: from_tag 或 to_tag 不存在", file=sys.stderr)
        return 2

    groups = parse_commits(args.from_tag, args.to_tag, args.include_merge, args.include_format)
    print(format_output(groups, args.markdown, args.from_tag, args.to_tag), end="")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
