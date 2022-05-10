import os
import sys
import argparse


__doc__ = """Create CMakeLists.txt based on directory content"""

CMAKE_CONTENT = """set(TARGET {project_name})

file(GLOB SOURCES *.cpp *.h *.txt, *.md)

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


def underline_to_camel(string):
    """
    Convert underline-separated string to CamelCase
    """
    return "".join(x.capitalize() or " " for x in string.split("_"))


def create_project_cmake(project_dir, project_name, vs_folder):
    """
    Create CMakeLists.txt with the same name as the project directory
    :param project_dir: Full-qualified directory name
    :param project_name: C++ project name same as the directory name
    :param vs_folder: Visual Studio folder name for the solution organization
    """
    print("Creating CMakeLists.txt for {}".format(project_name))
    os.chdir(project_dir)
    with open("CMakeLists.txt", "w") as cmake_file:
        cmake_file.write(CMAKE_CONTENT.format(project_name=project_name, vs_folder=vs_folder))
    os.system('git add CMakeLists.txt')


def create_cmake_files(target_dir):
    """
    Create CMakeLists.txt in every subdirectory of target_dir
    :param target_dir: Directory with list of C++ projects
    """
    os.chdir(target_dir)
    subdirs = sorted([x for x in os.listdir(target_dir) if os.path.isdir(os.path.join(target_dir, x))])
    print(f"Project directories: {subdirs}")
    for subdir in subdirs:
        vs_folder = underline_to_camel(os.path.split(target_dir)[1])
        create_project_cmake(project_dir=os.path.join(target_dir, subdir), project_name=subdir, vs_folder=vs_folder)


def create_subdir_cmake(target_dir):
    """
    Create CMakeLists.txt for a set of close-related projects
    Iterate over directory and write all subdirectories in CMakeLists.txt
    add_subdirectory({subdirectory})
    """
    # Get list of subdirectories without files
    os.chdir(target_dir)
    subdirs = sorted([x for x in os.listdir(target_dir) if os.path.isdir(os.path.join(target_dir, x))])
    # Create CMakeLists.txt
    with open(os.path.join(target_dir, "CMakeLists.txt"), "w") as cmake_file:
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
            os.system(f"git mv {subdir} {new_subdir}")
    new_subdirs = sorted([x for x in os.listdir(target_dir) if os.path.isdir(os.path.join(target_dir, x))])
    print(f"New subdirectories: {new_subdirs}")


def change_project_name(project_name):
    """
    Change project and target name in CMakeLists.txt
    """
    os.chdir(project_name)
    with open("CMakeLists.txt", "r") as cmake_file:
        cmake_lines = cmake_file.readlines()
    with open("CMakeLists.txt", "w") as cmake_file:
        for line in cmake_lines:
            if "project(" in line:
                cmake_file.write(f"project({project_name})\n")
            elif "set(TARGET_NAME" in line:
                cmake_file.write(f"set(TARGET_NAME {project_name})\n")
            else:
                cmake_file.write(line)
    os.chdir("..")


def change_project_names(target_dir):
    """
    Call change_project_name for every subdirectory in target_dir
    """
    os.chdir(target_dir)
    subdirs = sorted([x for x in os.listdir(target_dir) if os.path.isdir(os.path.join(target_dir, x))])
    for subdir in subdirs:
        change_project_name(subdir)


def main():
    parser = argparse.ArgumentParser(description='Command-line params')
    parser.add_argument('--create-cmake',
                        help='Directory to create CMakeLists.txt in',
                        default="",
                        required=False)
    parser.add_argument('--numeric-dirs',
                        help='Create numeric subdirectories',
                        default="",
                        required=False)
    parser.add_argument('--create-subdir-cmake',
                        help='Create CMakeLists.txt in target_dir',
                        default="",
                        required=False)
    parser.add_argument('--all',
                        help='Effect of all options',
                        default="",
                        required=False)
    args = parser.parse_args()
    if len(args.create_cmake):
        destination_dir = os.path.abspath(args.create_cmake)
        print(f"Creating CMakeLists.txt in {destination_dir}")
        create_cmake_files(destination_dir)
    elif len(args.numeric_dirs):
        numeric_dirs = os.path.abspath(args.numeric_dirs)
        print(f"Creating numeric subdirectories in {numeric_dirs}")
        create_numeric_dirs(numeric_dirs)
    elif len(args.all):
        target_dir = os.path.abspath(args.all)
        print(f"Creating subdir and project CMakeLists.txt in {target_dir}")
        create_subdir_cmake(target_dir)
        create_cmake_files(target_dir)
        return 0
    elif len(args.create_subdir_cmake):
        target_dir = os.path.abspath(args.create_subdir_cmake)
        create_subdir_cmake(target_dir)
        print(f"Created subdir CMakeLists.txt in {target_dir}")
    os.system(f"git push origin master")
    return 0


###########################################################################
if __name__ == '__main__':
    sys.exit(main())
