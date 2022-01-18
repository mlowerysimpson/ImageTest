cmake_minimum_required(VERSION 2.8)
project(ImageGrabTest)
find_package( OpenCV REQUIRED )
find_library(wiringPi_LIB wiringPi)

# Include headers
include_directories(${WIRINGPI_INCLUDE_DIRS})

#the file(GLOB...) allows for wildcard additions:
file(GLOB SOURCES "./*.cpp" "../RemoteControlTest/ImageUtil.cpp"
"../RemoteControlTest/MapUtil.cpp")
add_executable(ImageTest ${SOURCES})


target_link_libraries( ImageTest ${OpenCV_LIBS} )
target_link_libraries(ImageTest ${wiringPi_LIB} )
