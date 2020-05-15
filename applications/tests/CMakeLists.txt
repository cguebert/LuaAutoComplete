cmake_minimum_required(VERSION 3.5)

find_package(doctest CONFIG REQUIRED)

set(target Tests)

file(GLOB_RECURSE Header_Files "*.h")
file(GLOB_RECURSE Source_Files "*.cpp")

# Regroup files by folder
GroupFiles(Header_Files)
GroupFiles(Source_Files)

add_executable(${target} ${Header_Files} ${Source_Files})

target_link_libraries(${target}
	${META_PROJECT_NAME}::core
	doctest::doctest
	)

target_include_directories(${target} 
	PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR})

if(WITH_NLOHMANN_JSON)
	target_compile_definitions(${target} PUBLIC WITH_NLOHMANN_JSON)
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
set_target_properties(${target} PROPERTIES FOLDER "Applications")

install(TARGETS ${target} RUNTIME DESTINATION release CONFIGURATIONS Release)
install(TARGETS ${target} RUNTIME DESTINATION debug CONFIGURATIONS Debug)

if(WIN32 AND BUILD_SHARED_LIBS)
	install(TARGETS core RUNTIME DESTINATION debug CONFIGURATIONS Debug)
	install(TARGETS core RUNTIME DESTINATION release CONFIGURATIONS Release)
endif()

