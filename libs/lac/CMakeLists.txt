cmake_minimum_required(VERSION 3.5)

# External dependencies
find_package(Boost)
find_package(doctest CONFIG REQUIRED)

if(WITH_NLOHMANN_JSON)
	find_package(nlohmann_json CONFIG REQUIRED)
endif()

# Target name
set(target "core")

# Set API export file and macro
string(TOUPPER ${target} target_upper)
set(export_file  "include/lac/${target}_api.h")
set(export_macro "${target_upper}_API")

# Sources
file(GLOB_RECURSE Header_Files "*.h")
file(GLOB_RECURSE Source_Files "*.cpp")

if(NOT WITH_NLOHMANN_JSON)
	set(pattern "${CMAKE_CURRENT_SOURCE_DIR}/parser/printer(.h|.cpp)")
	string(REGEX REPLACE ${pattern} "" Header_Files "${Header_Files}")
	string(REGEX REPLACE ${pattern} "" Source_Files "${Source_Files}")
endif()

# Group source files
GroupFiles(Header_Files)
GroupFiles(Source_Files)

add_library(${target} ${Header_Files} ${Source_Files})

# This changes the filename to `lac_core`
set_target_properties(${target} PROPERTIES PREFIX "lac_")

# Create namespaced alias
add_library(${META_PROJECT_NAME}::${target} ALIAS ${target})

# Export library for downstream projects
export(TARGETS ${target} NAMESPACE ${META_PROJECT_NAME}:: FILE ${PROJECT_BINARY_DIR}/cmake/${target}/${target}-export.cmake)

# Create API export header
generate_export_header(${target} EXPORT_FILE_NAME ${export_file} EXPORT_MACRO_NAME ${export_macro})

# Include directories
target_include_directories(${target} 
	PRIVATE
	${CMAKE_SOURCE_DIR}/libs
	${CMAKE_CURRENT_BINARY_DIR}/include
	INTERFACE
	$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/libs>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include> 
	$<INSTALL_INTERFACE:include>)

# Libraries
target_link_libraries(${target} 
	PUBLIC
	Boost::boost
	PRIVATE
	doctest::doctest)

	# Compile definitions
target_compile_definitions(${target} 
	PUBLIC
	$<$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>:${target_upper}_STATIC_DEFINE>
	${DEFAULT_COMPILE_DEFINITIONS}
)

if(WITH_NLOHMANN_JSON)
	target_compile_definitions(${target} PUBLIC WITH_NLOHMANN_JSON)
	target_link_libraries(${target} PRIVATE nlohmann_json)
endif()

# Default properties
set_target_properties(${target} PROPERTIES ${DEFAULT_PROJECT_OPTIONS})

# Compile options
target_compile_options(${target} PRIVATE ${DEFAULT_COMPILE_OPTIONS})

if(NOT BUILD_UNIT_TESTS)
	target_compile_options(${target} PRIVATE "-DDOCTEST_CONFIG_DISABLE")
endif()

# Linker options
target_link_libraries(${target} PRIVATE ${DEFAULT_LINKER_OPTIONS})

# Project options
set_target_properties(${target} PROPERTIES FOLDER "Libraries")

if(MSVC)
    set_source_files_properties(parser/chunk.cpp PROPERTIES COMPILE_FLAGS "/bigobj")
endif()


# 
# Deployment
# 

# Library
install(TARGETS ${target}
	EXPORT  "${target}-export"            COMPONENT dev
	RUNTIME DESTINATION ${INSTALL_BIN}    COMPONENT runtime
	LIBRARY DESTINATION ${INSTALL_SHARED} COMPONENT runtime
	ARCHIVE DESTINATION ${INSTALL_LIB}    COMPONENT dev
)

# Header files
install(DIRECTORY
	${CMAKE_CURRENT_SOURCE_DIR} DESTINATION ${INSTALL_INCLUDE} 
	COMPONENT dev
	FILES_MATCHING PATTERN "*.h"
)

# Generated header files
install(DIRECTORY
	${CMAKE_CURRENT_BINARY_DIR}/include/lac DESTINATION ${INSTALL_INCLUDE}
	COMPONENT dev
)

# CMake config
install(EXPORT ${target}-export
	NAMESPACE   ${META_PROJECT_NAME}::
	DESTINATION ${INSTALL_CMAKE}/${target}
	COMPONENT   dev
)

if(WIN32 AND BUILD_SHARED_LIBS)
	install(TARGETS ${target} RUNTIME DESTINATION release CONFIGURATIONS Release)
	install(TARGETS ${target} RUNTIME DESTINATION debug CONFIGURATIONS Debug)
endif()
