cmake_minimum_required (VERSION 3.0)

set(PROJECT_NAME "ebs_datagen")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/bin)


macro(print_all_variables)
    message(STATUS "print_all_variables------------------------------------------{")
    get_cmake_property(_variableNames VARIABLES)
    foreach (_variableName ${_variableNames})
        message(STATUS "${_variableName}=${${_variableName}}")
    endforeach()
    message(STATUS "print_all_variables------------------------------------------}")
endmacro()

project (${PROJECT_NAME} LANGUAGES CXX)

include_directories("third-party")

set (CMAKE_CXX_STANDARD 20)

file(GLOB_RECURSE SRC_FILES 
	src/*.cpp 
	src/*.c
)
           
add_executable(${PROJECT_NAME} ${SRC_FILES})

print_all_variables()