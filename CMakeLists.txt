cmake_minimum_required(VERSION 3.5)

# Include cmake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Include cmake modules
include(cmake/Custom.cmake)

include(GenerateExportHeader)

if(DEFINED ENV{VCPKG_DEFAULT_TRIPLET} AND NOT DEFINED VCPKG_TARGET_TRIPLET)
set(VCPKG_TARGET_TRIPLET "$ENV{VCPKG_DEFAULT_TRIPLET}" CACHE STRING "")
endif()

# Meta information about the project
set(META_PROJECT_NAME "LuaAutoComplete")

# Declare project
project(${META_PROJECT_NAME})

# Options to build some projects
option(BUILD_SHARED_LIBS "Build shared libraries." ON)
option(BUILD_UNIT_TESTS "Build the unit tests." OFF)
option(BUILD_TEST_APPLICATION "Build the test application." ON)
option(BUILD_QT_PROJECTS "Build the QT projects." ON)

# Generate folders for IDE targets (e.g., VisualStudio solutions)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Compiler settings and options
include(cmake/CompileOptions.cmake)

# Installation paths
if(UNIX AND SYSTEM_DIR_INSTALL)
	# Install into the system (/usr/bin or /usr/local/bin)
	set(INSTALL_ROOT      "share/${project}")       # /usr/[local]/share/<project>
	set(INSTALL_CMAKE     "share/${project}/cmake") # /usr/[local]/share/<project>/cmake
	set(INSTALL_EXAMPLES  "share/${project}")       # /usr/[local]/share/<project>
	set(INSTALL_DATA      "share/${project}")       # /usr/[local]/share/<project>
	set(INSTALL_BIN       "bin")                    # /usr/[local]/bin
	set(INSTALL_SHARED    "lib")                    # /usr/[local]/lib
	set(INSTALL_LIB       "lib")                    # /usr/[local]/lib
	set(INSTALL_INCLUDE   "include")                # /usr/[local]/include
	set(INSTALL_DOC       "share/doc/${project}")   # /usr/[local]/share/doc/<project>
	set(INSTALL_SHORTCUTS "share/applications")     # /usr/[local]/share/applications
	set(INSTALL_ICONS     "share/pixmaps")          # /usr/[local]/share/pixmaps
	set(INSTALL_INIT      "/etc/init")              # /etc/init (upstart init scripts)
else()
	# Install into local directory
	set(INSTALL_ROOT      ".")                      # ./
	set(INSTALL_CMAKE     "cmake")                  # ./lib/cmake
	set(INSTALL_EXAMPLES  ".")                      # ./
	set(INSTALL_DATA      ".")                      # ./
	set(INSTALL_BIN       "bin")                    # ./bin
	set(INSTALL_SHARED    "lib")                    # ./lib
	set(INSTALL_LIB       "lib")                    # ./lib
	set(INSTALL_INCLUDE   "include")                # ./include
	set(INSTALL_DOC       "doc")                    # ./doc
	set(INSTALL_SHORTCUTS "misc")                   # ./misc
	set(INSTALL_ICONS     "misc")                   # ./misc
	set(INSTALL_INIT      "misc")                   # ./misc
endif()

# 
# Project modules
# 

add_subdirectory("libs/lac")

if(BUILD_QT_PROJECTS)
	add_subdirectory("libs/editor")
	add_subdirectory("applications/gui")
endif()

if(BUILD_TEST_APPLICATION)
	add_subdirectory("applications/tests")
endif()

# Install cmake find script for the project
install(FILES ${META_PROJECT_NAME}Config.cmake DESTINATION ${INSTALL_CMAKE} COMPONENT dev)
