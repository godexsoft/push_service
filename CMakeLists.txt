cmake_minimum_required(VERSION 2.8)
project(push_service)

# set some project-wide flags
set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-deprecated-declarations -Wno-overflow -DBOOST_SPIRIT_THREADSAFE -DBOOST_HAS_THREADS=1 -DBOOST_SPIRIT_USE_PHOENIX_V3=1" )
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations -Wno-overflow -DBOOST_SPIRIT_THREADSAFE -DBOOST_HAS_THREADS=1 -DBOOST_SPIRIT_USE_PHOENIX_V3=1" )

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)

if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

    if(COMPILER_SUPPORTS_LIBCPP)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    endif()
elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
    
    if(COMPILER_SUPPORTS_LIBCPP)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    endif()
else()
    message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

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
