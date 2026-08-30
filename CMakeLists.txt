cmake_minimum_required(VERSION 3.22)
project(led_sensor_plugin VERSION 1.0 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Gazebo Harmonic stack
find_package(gz-sim8 REQUIRED)
find_package(gz-transport13 REQUIRED)
find_package(gz-plugin2 REQUIRED)
find_package(gz-msgs10 REQUIRED)
find_package(sdformat14 REQUIRED)

add_library(LedSensor SHARED
  src/led_sensor.cc
)

target_include_directories(LedSensor PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(LedSensor PUBLIC
  gz-sim8::gz-sim8
  gz-transport13::gz-transport13
  gz-plugin2::gz-plugin2
  gz-msgs10::gz-msgs10
  sdformat14::sdformat14
)