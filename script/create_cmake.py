import os
import sys
import argparse


__doc__ = """Create CMakeLists.txt based on directory content"""

CMAKE_CONTENT = """set(TARGET {project_name})

file(GLOB SOURCES *.cpp *.h *.txt)

include_directories(
    ${CMAKE_SOURCE_DIR}/utilities
)

add_executable(${TARGET} ${SOURCES})
set_property(TARGET ${TARGET} PROPERTY FOLDER "{vs_folder}")

target_link_libraries(${TARGET}    
PRIVATE
    utilities
)

"""


def create_project_cmake(project_name, vs_folder):
    """
    Create CMakeLists.txt with the same name as the project directory
    :param project_name: C++ project name same as the directory name
    :param vs_folder: Visual Studio folder name for the solution organization
    """
    with open("CMakeLists.txt", "w") as cmake_file:
        cmake_file.write(CMAKE_CONTENT.format(project_name=project_name, vs_folder=vs_folder))


def create_cmake_files(target_dir):
    """
    Create CMakeLists.txt in every subdirectory of target_dir
    :param target_dir: Directory with list of C++ projects
    """
    os.chdir(target_dir)
    subdirs = sorted([x for x in os.listdir(target_dir) if os.path.isdir(os.path.join(target_dir, x))])
    for subdir in subdirs:
        create_list_cmake(subdir)


def create_list_cmake(target_dir):
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
        cmake_file.write("\n")
        for subdir in subdirs:
            cmake_file.write(f"add_subdirectory({subdir})\n")
    os.system(f"git commit --all -n -m 'Created CMakeLists.txt in {target_dir}'")


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
    os.system(f"git commit --all -n -m 'Changed project name in {project_name}'")
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
    parser.add_argument('--destination-dir',
                        help='Directory to create CMakeLists.txt in',
                        default="",
                        required=False)
    parser.add_argument('--numeric-dirs',
                        help='Create numeric subdirectories',
                        default="",
                        required=False)
    parser.add_argument('--change-project-name',
                        help='Change project names in target_dir',
                        default="",
                        required=False)
    parser.add_argument('--all',
                        help='Effect of all options',
                        default="",
                        required=False)
    args = parser.parse_args()
    if len(args.all):
        target_dir = os.path.abspath(args.all)
        print(f"Creating numeric subdirectories, CMakeLists.txt and change project names in {target_dir}")
        create_numeric_dirs(target_dir)
        create_list_cmake(target_dir)
        change_project_names(target_dir)
        return 0
    elif len(args.numeric_dirs):
        numeric_dirs = os.path.abspath(args.numeric_dirs)
        print(f"Creating numeric subdirectories in {numeric_dirs}")
        create_numeric_dirs(numeric_dirs)
    elif len(args.destination_dir):
        destination_dir = os.path.abspath(args.destination_dir)
        print(f"Creating CMakeLists.txt in {destination_dir}")
        create_list_cmake(destination_dir)
    elif len(args.change_project_name):
        target_dir = os.path.abspath(args.change_project_name)
        change_project_names(target_dir)
        print(f"Changed project names in {target_dir}")
    os.system(f"git push origin master")
    return 0


###########################################################################
if __name__ == '__main__':
    sys.exit(main())
