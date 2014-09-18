cmake_minimum_required(VERSION 2.8)
project(push_service)

# set some project-wide flags
set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-overflow -DBOOST_SPIRIT_THREADSAFE" )

# we require boost with some linked libraries
find_package(Boost 1.53.0 COMPONENTS system)

set( LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/lib/pc CACHE PATH
        "Single Directory for all Libraries")

# all source files
file(GLOB_RECURSE ALL_SRC src/*.cpp)

# set includes
include_directories(${Boost_INCLUDE_DIRS})
include_directories("src")
include_directories("ext")

set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fpermissive" )

# compile push_service itself
add_library(push_service STATIC ${ALL_SRC})

# link final lib
target_link_libraries(push_service ${Boost_LIBRARIES})
