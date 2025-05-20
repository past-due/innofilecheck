cmake_minimum_required(VERSION 3.5...3.24)
project(
	innofilecheck
	VERSION 0.1.0
	HOMEPAGE_URL https://github.com/past-due/innofilecheck
	LANGUAGES CXX
)

if(NOT WIN32)
	message( FATAL_ERROR "Only supports Windows" )
endif()

add_subdirectory(src)

file(GLOB INFO_FILES "*.txt" "*.md" "LICENSE")
INSTALL(FILES ${INFO_FILES}
		COMPONENT "Info" DESTINATION ".")
