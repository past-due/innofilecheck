cmake_minimum_required(VERSION 3.5...3.24)

if(NOT WIN32)
	message( FATAL_ERROR "Only supports Windows" )
endif()

if (MSVC)
	# Suppress all the standard libraries - the only ones needed are added later
	set(CMAKE_CXX_STANDARD_LIBRARIES "")
	set(CMAKE_C_STANDARD_LIBRARIES "")
endif()

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp codesigncheck.cpp crt.cpp utils.cpp versioninfo.cpp)

# File VERSIONINFO
if (NOT DEFINED PROJECT_VERSION_MAJOR OR "${PROJECT_VERSION_MAJOR}" STREQUAL "")
	set(PROJECT_VERSION_MAJOR 1)
endif()
if (NOT DEFINED PROJECT_VERSION_MINOR OR "${PROJECT_VERSION_MINOR}" STREQUAL "")
	set(PROJECT_VERSION_MINOR 0)
endif()
if (NOT DEFINED PROJECT_VERSION_PATCH OR "${PROJECT_VERSION_PATCH}" STREQUAL "")
	set(PROJECT_VERSION_PATCH 0)
endif()
if (NOT DEFINED PROJECT_VERSION_REVISION OR "${PROJECT_VERSION_REVISION}" STREQUAL "")
	set(PROJECT_VERSION_REVISION 0)
endif()
set(VERINFO_PRODUCTVERSION "${PROJECT_VERSION_MAJOR},${PROJECT_VERSION_MINOR},${PROJECT_VERSION_PATCH},${PROJECT_VERSION_REVISION}")
set(VERINFO_PRODUCTVERSION_STR "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}.${PROJECT_VERSION_REVISION}")
set (_rc_file "${CMAKE_CURRENT_BINARY_DIR}/innofilecheck.rc")
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/innofilecheck.rc.in" "${_rc_file}" @ONLY)
list(APPEND SOURCE_FILES "${_rc_file}")

# Unicode DLL
add_library(filecheck_unicode MODULE ${SOURCE_FILES})
target_compile_definitions(filecheck_unicode PRIVATE "UNICODE" "_UNICODE")
set_target_properties(filecheck_unicode PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_BINARY_DIR}")
set_target_properties(filecheck_unicode PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}")
set_target_properties(filecheck_unicode PROPERTIES LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_BINARY_DIR}")
set_target_properties(filecheck_unicode PROPERTIES LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}")
set_target_properties(filecheck_unicode PROPERTIES OUTPUT_NAME "innofilecheck")

if (MSVC)
	# Do not link the CRT (or any default libs)
	set_target_properties(filecheck_unicode PROPERTIES LINK_FLAGS "/NODEFAULTLIB /ENTRY:DllMain")
	# Do not use /GS (it requires the CRT)
	target_compile_options(filecheck_unicode PRIVATE "/GS-")
	# *Do* explicitly link to kernel32.lib and user32.lib, which are required
	target_link_libraries(filecheck_unicode PRIVATE kernel32 user32)
	# Ensure DEP and ASLR
	set_property(TARGET filecheck_unicode APPEND_STRING PROPERTY "LINK_FLAGS" " /NXCOMPAT /DYNAMICBASE")
endif()

###########################
# Install Rules

install(TARGETS filecheck_unicode COMPONENT Plugins DESTINATION ".")

# Copy the source files over into the "src" folder
file(GLOB SRC_FILES "*.txt" "*.cpp" "*.c" "*.h" "*.rc" "*.in")
INSTALL(FILES ${SRC_FILES} COMPONENT Source DESTINATION "src")
