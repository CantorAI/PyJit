# CMakeList.txt : CMake project for Grus, include source and define
#project for pyjit.org
cmake_minimum_required(VERSION 3.19)

set(PACKAGE_NAME "pyjit")
set(PROJECT_NAME "grus")
project (${PROJECT_NAME})

find_package (Python3 COMPONENTS  NumPy)

string(REPLACE "numpy/core/include" "" Python3_SITELIB ${Python3_NumPy_INCLUDE_DIRS})

message(STATUS
    "Python: version=${Python3_VERSION} interpreter=${Python3_EXECUTABLE}")
if(NOT Python3_FOUND AND Python3_Development_FOUND)
    #   find_package() will not abort the build if anything's missing.
    string(JOIN "\n" errmsg
        "  Python3 and/or development libs not found."
        "  - Python3_FOUND=${Python3_FOUND}"
        "  - Python3_Development_FOUND=${Python3_Development_FOUND}"
        "  - Python3_INCLUDE_DIRS=${Python3_INCLUDE_DIRS}"
    )
    message(FATAL_ERROR ${errmsg})
else()
    string(JOIN "\n" infoMsg
        "  - Python3_FOUND=${Python3_FOUND}"
        "  - Python3_Development_FOUND=${Python3_Development_FOUND}"
        "  - Python3_INCLUDE_DIRS=${Python3_INCLUDE_DIRS}"
        "  - Python3_NumPy_INCLUDE_DIRS=${Python3_NumPy_INCLUDE_DIRS}"
        "  - Python3_LIBRARY_DIRS=${Python3_LIBRARY_DIRS}"
        "  - Python3_LIBRARIES=${Python3_LIBRARIES}"
        "  - Python3_SITELIB=${Python3_SITELIB}"
        "  - Python3_RUNTIME_LIBRARY_DIRS=${Python3_RUNTIME_LIBRARY_DIRS}"
    )
    message(STATUS ${infoMsg})
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -DNPY_NO_DEPRECATED_API=NPY_1_7_API_VERSION")

FILE(GLOB Common_SRC common/*.cpp)

FILE(GLOB SRC grus/*.cpp)
FILE(GLOB HEAD grus/*.h)

FILE(GLOB PYTHON_FILES python/*.py)
FILE(GLOB JIT_INC_FILES python/Jit_Host.h python/Jit_Object.h)


add_library(${PROJECT_NAME} 
	SHARED
	${SRC}
    ${Common_SRC}
	${HEAD})

target_include_directories(${PROJECT_NAME} 
	PUBLIC 
    "common"
    "grus"
    "python"
	"${Python3_INCLUDE_DIRS}"
	"${Python3_NumPy_INCLUDE_DIRS}"
	)
target_link_directories(${PROJECT_NAME} PUBLIC 
	"\"${Python3_LIBRARY_DIRS}\""
	)

if (WIN32)
target_link_libraries(${PROJECT_NAME} PUBLIC 
    "${Python3_LIBRARIES}" Ws2_32
	)
else()
target_link_libraries(${PROJECT_NAME} PUBLIC 
	"${Python3_LIBRARIES}" uuid
	)
endif()

add_custom_command(
  TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory 
	${CMAKE_CURRENT_BINARY_DIR}/python)

add_custom_command(
  TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy 
    ${PYTHON_FILES}
    ${CMAKE_CURRENT_BINARY_DIR}/python/)

add_custom_command(
  TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E make_directory 
    ${Python3_SITELIB}/${PACKAGE_NAME}/)

if (WIN32)
add_custom_command(
  TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy 
    ${CMAKE_CURRENT_BINARY_DIR}/grus.dll
    ${Python3_SITELIB}/${PACKAGE_NAME}/grus.pyd)
add_custom_command(
  TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy 
    ${CMAKE_CURRENT_BINARY_DIR}/grus.pdb
    ${Python3_SITELIB}/${PACKAGE_NAME}/grus.pdb)
else()
add_custom_command(
  TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy 
    ${CMAKE_CURRENT_BINARY_DIR}/libgrus.so
    ${Python3_SITELIB}/${PACKAGE_NAME}/grus.so)
endif()

add_custom_command(
  TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy 
    ${PYTHON_FILES} 
    ${Python3_SITELIB}/${PACKAGE_NAME}/)
add_custom_command(
  TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy 
    ${JIT_INC_FILES}
    ${Python3_SITELIB}/${PACKAGE_NAME}/)

