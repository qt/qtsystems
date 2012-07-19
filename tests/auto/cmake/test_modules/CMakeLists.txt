
cmake_minimum_required(VERSION 2.8)

project(test_modules)

find_package(Qt5Gui REQUIRED)
find_package(Qt5Network REQUIRED)

find_package(Qt5PublishSubscribe REQUIRED)
find_package(Qt5ServiceFramework REQUIRED)
find_package(Qt5SystemInfo REQUIRED)

include_directories(
  ${Qt5PublishSubscribe_INCLUDE_DIRS}
  ${Qt5ServiceFramework_INCLUDE_DIRS}
  ${Qt5SystemInfo_INCLUDE_DIRS}
)

add_definitions(
  ${Qt5PublishSubscribe_DEFINITIONS}
  ${Qt5ServiceFramework_DEFINITIONS}
  ${Qt5SystemInfo_DEFINITIONS}
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Core_EXECUTABLE_COMPILE_FLAGS}")

add_executable(mainapp main.cpp)
target_link_libraries(mainapp
  ${Qt5PublishSubscribe_LIBRARIES}
  ${Qt5ServiceFramework_LIBRARIES}
  ${Qt5SystemInfo_LIBRARIES}
)
