cmake_minimum_required(VERSION 3.5)

# External dependencies
find_package(Qt5 COMPONENTS Core Widgets REQUIRED)

set(target gui)

file(GLOB_RECURSE Header_Files "*.h")
file(GLOB_RECURSE Source_Files "*.cpp")

# Regroup files by folder
GroupFiles(Header_Files)
GroupFiles(Source_Files)

add_executable(${target} ${Header_Files} ${Source_Files})

target_link_libraries(${target}
	${META_PROJECT_NAME}::lac
	${META_PROJECT_NAME}::editor
	)

target_include_directories(${target} 
	PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR})

# Default properties
set_target_properties(${target} PROPERTIES ${DEFAULT_PROJECT_OPTIONS})

# Compile options
target_compile_options(${target} PRIVATE ${DEFAULT_COMPILE_OPTIONS})

# Linker options
target_link_libraries(${target} PRIVATE ${DEFAULT_LINKER_OPTIONS})

# Project options
set_target_properties(${target} PROPERTIES FOLDER "Applications")

install(TARGETS ${target} RUNTIME DESTINATION release CONFIGURATIONS Release)
install(TARGETS ${target} RUNTIME DESTINATION debug CONFIGURATIONS Debug)

if(WIN32)
	# Copy the dependencies dlls
	foreach(export_lib Qt5::Widgets Qt5::Core Qt5::Gui)
		InstallDependency(${export_lib})
	endforeach()

	InstallDependency(Qt5::QWindowsIntegrationPlugin "/platforms")
endif()
