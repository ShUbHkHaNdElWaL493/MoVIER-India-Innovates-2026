cmake_minimum_required(VERSION 3.16)
project(project-x_diffbot_controller LANGUAGES CXX)

if(CMAKE_CXX_COMPILER_ID MATCHES "(GNU|Clang)")
  add_compile_options(-Wall -Wextra)
endif()

# 1. Find Dependencies
find_package(ament_cmake REQUIRED)
find_package(hardware_interface REQUIRED)
find_package(pluginlib REQUIRED)
find_package(rclcpp REQUIRED)
# find_package(LibSerial REQUIRED) - Removed due to missing config file
find_path(LibSerial_INCLUDE_DIR NAMES libserial/SerialStream.h PATHS /usr/include /usr/local/include)
find_library(LibSerial_LIBRARY NAMES serial PATHS /usr/lib/x86_64-linux-gnu /usr/lib /usr/local/lib)

if(LibSerial_INCLUDE_DIR AND LibSerial_LIBRARY)
  message(STATUS "Found LibSerial: ${LibSerial_LIBRARY}")
  if(NOT TARGET LibSerial::LibSerial)
    add_library(LibSerial::LibSerial UNKNOWN IMPORTED)
    set_target_properties(LibSerial::LibSerial PROPERTIES
      IMPORTED_LOCATION "${LibSerial_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${LibSerial_INCLUDE_DIR}"
    )
  endif()
else()
  message(FATAL_ERROR "Could not find LibSerial library or headers")
endif()


# 2. Create Library
add_library(
  ${PROJECT_NAME}
  SHARED
  hardwareInterface/projx-diffdrive.cpp
)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

# 3. Include Directories
# Note: Pointing to 'hardwareInterface/include' based on your ls output
target_include_directories(${PROJECT_NAME} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/hardwareInterface/include>
  $<INSTALL_INTERFACE:include>
)

# 4. Link Dependencies
ament_target_dependencies(
  ${PROJECT_NAME}
  hardware_interface
  pluginlib
  rclcpp
  rclcpp_lifecycle
)

target_link_libraries(${PROJECT_NAME} LibSerial::LibSerial)

# Note: 'serial' library removed (You are using native Linux sockets now)

# 5. Visibility Definitions
# IMPORTANT: This must match the #ifdef in your visibility_control.h
# Your file uses "PROKECTX_HW_BUILDING_DLL" (with the typo)
target_compile_definitions(${PROJECT_NAME} PRIVATE "PROKECTX_HW_BUILDING_DLL")

# 6. Export Hardware Plugin
# Note: Your ls showed 'projx_control.xml', not 'project-x_diffbot_controller.xml'
pluginlib_export_plugin_description_file(hardware_interface projx_control.xml)

# 7. Install Rules

# Install the Library
install(
  TARGETS ${PROJECT_NAME}
  EXPORT export_${PROJECT_NAME}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

# Install Headers
install(
  DIRECTORY hardwareInterface/include/
  DESTINATION include
)

# Install Config, Description, and Plugin XML
install(
  DIRECTORY config description launch
  DESTINATION share/${PROJECT_NAME}
)

install(
  FILES projx_control.xml
  DESTINATION share/${PROJECT_NAME}
)

# 8. Export for other packages
ament_export_targets(export_${PROJECT_NAME} HAS_LIBRARY_TARGET)
ament_export_dependencies(
  hardware_interface
  pluginlib
  rclcpp
  rclcpp_lifecycle
)

ament_package()