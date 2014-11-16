cmake_minimum_required(VERSION 2.8)

project(imageprocess)

find_package(ImageMagick COMPONENTS Magick++ REQUIRED)

include_directories(${ImageMagick_Magick++_INCLUDE_DIR})

# is needed for Debian, maybe Ubuntu as well
include_directories("/usr/include/x86_64-linux-gnu/ImageMagick-6/")
add_definitions(-DMAGICKCORE_QUANTUM_DEPTH=16 -DMAGICKCORE_HDRI_ENABLE=0)

add_executable(main main.cpp)
target_link_libraries(main ${ImageMagick_Magick++_LIBRARY})
