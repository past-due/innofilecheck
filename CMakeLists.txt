cmake_minimum_required(VERSION 3.5...3.24)
project(innofilecheck)

if(NOT WIN32)
	message( FATAL_ERROR "Only supports Windows" )
endif()

add_subdirectory(src)

file(GLOB INFO_FILES "*.txt" "*.md" "LICENSE")
INSTALL(FILES ${INFO_FILES}
		COMPONENT "Info" DESTINATION ".")
