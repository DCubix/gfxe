cmake_minimum_required(VERSION 3.10)
project(gfxe_test VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS False)

file(GLOB_RECURSE SRC
	"*.h"
	"*.hpp"
	"*.cpp"
)

find_package(SDL2 REQUIRED SDL2)
add_definitions(-DSDL_MAIN_HANDLED)

add_executable(${PROJECT_NAME} ${SRC})
target_link_libraries(${PROJECT_NAME} PUBLIC SDL2 gfxe)

if (CMAKE_DL_LIBS)
	target_link_libraries(${PROJECT_NAME}
		${CMAKE_DL_LIBS}
	)
endif()