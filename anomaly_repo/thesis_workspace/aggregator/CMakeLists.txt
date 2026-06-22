cmake_minimum_required(VERSION 3.8)
project(aggregator)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(n2k_ros2_shared_msgs REQUIRED)
find_package(std_msgs REQUIRED)

add_executable(aggregator src/aggregator.cpp)

ament_target_dependencies(aggregator rclcpp n2k_ros2_shared_msgs std_msgs)

install(TARGETS aggregator DESTINATION lib/${PROJECT_NAME})

ament_package()
