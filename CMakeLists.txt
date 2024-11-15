cmake_minimum_required(VERSION 3.16)
project(PathKit)

set(CMAKE_CXX_STANDARD 17)

include_directories(./)
add_executable(PathKitDemo
    main.cpp
    # unity_pathkit_all.cpp
    unity_pathkit_core.cpp
    unity_pathkit_misc.cpp
    unity_pathkit_pathops1.cpp
    unity_pathkit_pathops2.cpp
    
    )
target_compile_options(PathKitDemo PRIVATE
    -Wno-unused-function
    -Wno-unused-variable
    )
