cmake_minimum_required(VERSION 3.5)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(cmake/Custom.cmake)

if(DEFINED ENV{VCPKG_DEFAULT_TRIPLET} AND NOT DEFINED VCPKG_TARGET_TRIPLET)
set(VCPKG_TARGET_TRIPLET "$ENV{VCPKG_DEFAULT_TRIPLET}" CACHE STRING "")
endif()

project("LuaAutoComplete")

include(cmake/CompileOptions.cmake)

find_package(Boost)
find_package(doctest CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

set(target "LuaAutoComplete")

file(GLOB_RECURSE Header_Files "src/*.h")
file(GLOB_RECURSE Source_Files "src/*.cpp")

GroupFiles(Header_Files "src/")
GroupFiles(Source_Files "src/")

add_executable(${target} ${Header_Files} ${Source_Files})

target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src) 

target_link_libraries(${target} 
	Boost::boost
	doctest::doctest
	nlohmann_json)

set_target_properties(${target} PROPERTIES ${DEFAULT_PROJECT_OPTIONS})
target_compile_options(${target} PRIVATE ${DEFAULT_COMPILE_OPTIONS})
target_link_libraries(${target} PRIVATE ${DEFAULT_LINKER_OPTIONS})

if(MSVC)
    set_source_files_properties(src/parser/chunk.cpp PROPERTIES COMPILE_FLAGS "/bigobj")
endif()

install(TARGETS ${target} RUNTIME DESTINATION release CONFIGURATIONS Release)
install(TARGETS ${target} RUNTIME DESTINATION release CONFIGURATIONS RelWithDebInfo)
install(TARGETS ${target} RUNTIME DESTINATION debug CONFIGURATIONS Debug)
