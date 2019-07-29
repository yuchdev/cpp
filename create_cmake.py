import os, sys, shutil, stat


###########################################################################
def on_rm_error(*args):
    '''
    In case the file or directory is read-only and we need to delete it
    this function will help to remove 'read-only' attribute
    :param args: (func, path, exc_info) yuple
    '''
    # path contains the path of the file that couldn't be removed
    # let's just assume that it's read-only and unlink it.
    _, path, _ = args
    os.chmod(path, stat.S_IWRITE)
    os.unlink(path)


SOLUTION_NAME = 'cpp'
PATH = '.'

# project=0
# target=1
# std=2
# -pthread=3
# arch=4

BASE_CMAKE_TXT = '''cmake_minimum_required(VERSION 2.8)
project({0} CXX)
set(TARGET {1})


if( ("UNIX") AND ("${{CMAKE_CXX_COMPILER_ID}}" STREQUAL "Clang") )

    # using Clang
    message("UNIX congiguration, Clang")
    set(CMAKE_CXX_FLAGS "-m{4} -std={2}")
    set(CMAKE_EXE_LINKER_FLAGS "-std={2} {3} -Wl")
elseif( ("UNIX") AND ("${{CMAKE_CXX_COMPILER_ID}}" STREQUAL "GNU") )

    # using GCC
    message("UNIX congiguration, GCC")
    message("WARNING: gcc-multilib is required on x64 systems for the cross-compilation!")
    set(CMAKE_CXX_FLAGS "-m{4} -std={2} {3}")
    set(CMAKE_EXE_LINKER_FLAGS "-std={2} {3} -Wl,--no-as-needed")

elseif("WIN32")

  message("Windows configuration")
  set(CMAKE_CXX_FLAGS "/EHa")

endif()

########################################################

# CMake policies
cmake_policy(SET CMP0015 NEW)
'''

cmake_text = '''cmake_minimum_required(VERSION 2.8)
project({0} CXX)
set(TARGET {1})

########################################################
# source files
# source files
file(GLOB SOURCES *.cpp *.h *.txt)

########################################################
# include directories
set(INCLUDE_DIRS ${{INCLUDE_DIRS}} . )
include_directories(${{INCLUDE_DIRS}})

########################################################
add_executable(${{TARGET}} ${{SOURCES}})
'''

def create_cmake(path, project):
    '''
    :param path: path to the solution
    :param project: project name
    '''
    project_path = os.path.join(path, project)
    f = open(os.path.join(project_path, 'CMakeLists.txt'), 'w')
    f.write(cmake_text.format(project, project))
    f.close()


def create_base_cmake(root_path, solution_name, projects_list):
    '''
    :param root_path: path to the solution
    :param solution_name: string name of the solution
    :param projects_list: list of projects (subdirectory names)
    '''
    f = open(os.path.join(root_path, 'CMakeLists.txt'), 'w')
    # project, target, c++11, -pthread, 32/64
    f.write(BASE_CMAKE_TXT.format(solution_name, solution_name, 'c++11', '', '32'))
    for project in projects_list:
        f.write('add_subdirectory({0})\n'.format(project))
    f.close()

def mass_create_cmake(root_path, solution_name):
    '''
    :param root_path: path to the solution
    :param solution_name: string name of the solution
    '''
    projects_list = []
    for project in os.listdir(root_path):
        if os.path.isdir(project):
                create_cmake(root_path, project)
                projects_list.append(project)

    create_base_cmake(root_path, solution_name, projects_list)


def main(argv):
    try:
        if os.path.isdir('build-cmake'):
            shutil.rmtree('build-cmake', onerror=on_rm_error)
        mass_create_cmake(PATH, SOLUTION_NAME)
        os.mkdir('build-cmake')
        os.chdir('build-cmake')
        return os.system('cmake ..')
    except IndexError:
        print("Usage: python create_cmake.py <solution_name> <solution_path>")
    return 0

###################################
if __name__ == "__main__":
    sys.exit(main(sys.argv))
