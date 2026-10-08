# python
import os
import re
import sys
import argparse
import subprocess

CMAKE_CONTENT = """set(TARGET {project_name})

file(GLOB SOURCES *.cpp *.h)

include_directories(
    ${{CMAKE_SOURCE_DIR}}
)

add_executable(${{TARGET}} ${{SOURCES}})
set_property(TARGET ${{TARGET}} PROPERTY FOLDER "{vs_folder}")

target_link_libraries(${{TARGET}}
PRIVATE
    utilities
)
"""

NN_PATTERN = re.compile(r"^(\d{2})(.+)$")


def underline_to_camel(string: str) -> str:
    return "".join(x.capitalize() or " " for x in string.split("_"))


def list_subdirs(target_dir: str) -> list[str]:
    return sorted([x for x in os.listdir(target_dir) if os.path.isdir(os.path.join(target_dir, x))])


def is_sequential_from_01(subdirs: list[str]) -> bool:
    # All subdirs must match NN* and be 01, 02, ... in the sorted order
    for idx, name in enumerate(subdirs, start=1):
        m = NN_PATTERN.match(name)
        if not m:
            return False
        try:
            nn = int(m.group(1))
        except ValueError:
            return False
        if nn != idx:
            return False
    return True


def apply_sequential_rename(target_dir: str, subdirs: list[str]) -> list[str]:
    # Rename to 01..NN preserving suffix after first two digits or whole name if no NN prefix
    new_names = []
    for idx, name in enumerate(subdirs, start=1):
        m = NN_PATTERN.match(name)
        suffix = m.group(2) if m else name
        new_name = f"{idx:02d}{suffix}"
        if new_name != name:
            old_path = os.path.join(target_dir, name)
            new_path = os.path.join(target_dir, new_name)
            subprocess.run(["git", "mv", old_path, new_path], check=False)
        new_names.append(new_name)
    return sorted(new_names)


def write_project_cmake(project_dir: str, project_name: str, vs_folder: str) -> None:
    cmake_path = os.path.join(project_dir, "CMakeLists.txt")
    with open(cmake_path, "w") as f:
        f.write(CMAKE_CONTENT.format(project_name=project_name, vs_folder=vs_folder))


def write_root_cmake(target_dir: str, subdirs: list[str]) -> None:
    root_cmake = os.path.join(target_dir, "CMakeLists.txt")
    with open(root_cmake, "w") as f:
        for subdir in subdirs:
            f.write(f"add_subdirectory({subdir})\n")


def ensure_cmake_files(target_dir: str, subdirs: list[str]) -> None:
    vs_folder = underline_to_camel(os.path.basename(target_dir))
    for subdir in subdirs:
        proj_dir = os.path.join(target_dir, subdir)
        write_project_cmake(proj_dir, subdir, vs_folder)
    write_root_cmake(target_dir, subdirs)


def git_commit(target_dir: str) -> None:
    # Add entire directory and commit (no push)
    subprocess.run(["git", "add", target_dir], check=False)
    subprocess.run([
        "git", "commit", "--all", "--message", f"Generate CMake files for {os.path.basename(target_dir)}"
    ], check=False)


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Create CMake files and optionally sequentially rename subdirectories."
    )
    parser.add_argument(
        "directory",
        help="Target directory path"
    )
    parser.add_argument(
        "--sequential-rename",
        action="store_true",
        help="Sequentially rename subdirs starting from 01 if needed"
    )
    args = parser.parse_args()

    target_dir = os.path.abspath(args.directory)
    if not os.path.isdir(target_dir):
        print(f"Error: `{target_dir}` is not a directory")
        return 1

    subdirs = list_subdirs(target_dir)

    if args.sequential_rename:
        if not is_sequential_from_01(subdirs):
            subdirs = apply_sequential_rename(target_dir, subdirs)

    # Re-list subdirs after potential rename
    subdirs = list_subdirs(target_dir)

    # Create CMakeLists.txt files
    ensure_cmake_files(target_dir, subdirs)

    # Add and commit entire directory (no push)
    git_commit(target_dir)

    print(f"Done for `{target_dir}`")
    return 0


if __name__ == "__main__":
    sys.exit(main())
