import os
import sys
import argparse


__doc__ = """Create CMakeLists.txt based on directory content"""


def create_cmake(target_dir):
    """
    Create list of sorted subdirectories in target_dir
    Iterate over list and write all subdirectories in CMakeLists.txt
    add_subdirectory({subdirectory})
    """
    # Get list of subdirectories without files
    os.chdir(target_dir)
    subdirs = sorted([x for x in os.listdir(target_dir) if os.path.isdir(os.path.join(target_dir, x))])
    # Create CMakeLists.txt
    with open(os.path.join(target_dir, "CMakeLists.txt"), "w") as cmake_file:
        cmake_file.write("cmake_minimum_required(VERSION 3.10)\n")
        cmake_file.write("\n")
        for subdir in subdirs:
            cmake_file.write(f"add_subdirectory({subdir})\n")


def create_numeric_dirs(target_dir):
    """
    Create list of sorted subdirectories in target_dir
    Perform command "git mv NN_subdirectory" for every subdirectory
    Where NN is counter from 00 to 99
    """
    # Get list of subdirectories without files
    os.chdir(target_dir)
    subdirs = sorted([x for x in os.listdir(target_dir) if os.path.isdir(os.path.join(target_dir, x))])
    print(f"Old subdirectories: {subdirs}")
    # Create CMakeLists.txt
    counter = 0
    for subdir in subdirs:
        new_subdir = f"{counter:02d}{subdir[2:]}"
        counter += 1
        if subdir != new_subdir:
            print(f"git mv {subdir} {new_subdir}")
            # subprocess.run(["git", "mv", subdir, new_subdir])
            os.system(f"git mv {subdir} {new_subdir}")
    new_subdirs = sorted([x for x in os.listdir(target_dir) if os.path.isdir(os.path.join(target_dir, x))])
    print(f"New subdirectories: {new_subdirs}")
    os.system(f"git commit --all -n -m 'Renamed subdirectories in {target_dir}'")
    os.system(f"git push origin master")


def main():
    parser = argparse.ArgumentParser(description='Command-line params')
    parser.add_argument('--destination-dir',
                        help='Directory to create CMakeLists.txt in',
                        default="",
                        required=False)
    parser.add_argument('--numeric-dirs',
                        help='Create numeric subdirectories',
                        default="",
                        required=False)
    args = parser.parse_args()
    if len(args.destination_dir):
        destination_dir = os.path.abspath(args.destination_dir)
        print(f"Creating CMakeLists.txt in {destination_dir}")
        create_cmake(destination_dir)
    elif len(args.numeric_dirs):
        numeric_dirs = os.path.abspath(args.numeric_dirs)
        print(f"Creating numeric subdirectories in {numeric_dirs}")
        create_numeric_dirs(numeric_dirs)
    return 0

###########################################################################
if __name__ == '__main__':
    sys.exit(main())
