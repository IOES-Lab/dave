# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.28

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/abhimanyu/dave_ws/src/dave/dave_interfaces

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/abhimanyu/dave_ws/src/dave/build/dave_interfaces

# Utility rule file for dave_interfaces__rosidl_generator_type_description.

# Include any custom commands dependencies for this target.
include CMakeFiles/dave_interfaces__rosidl_generator_type_description.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/dave_interfaces__rosidl_generator_type_description.dir/progress.make

CMakeFiles/dave_interfaces__rosidl_generator_type_description: rosidl_generator_type_description/dave_interfaces/msg/UsblCommand.json
CMakeFiles/dave_interfaces__rosidl_generator_type_description: rosidl_generator_type_description/dave_interfaces/msg/UsblResponse.json
CMakeFiles/dave_interfaces__rosidl_generator_type_description: rosidl_generator_type_description/dave_interfaces/msg/Location.json
CMakeFiles/dave_interfaces__rosidl_generator_type_description: rosidl_generator_type_description/dave_interfaces/srv/SetOriginSphericalCoord.json
CMakeFiles/dave_interfaces__rosidl_generator_type_description: rosidl_generator_type_description/dave_interfaces/srv/GetOriginSphericalCoord.json
CMakeFiles/dave_interfaces__rosidl_generator_type_description: rosidl_generator_type_description/dave_interfaces/srv/TransformToSphericalCoord.json
CMakeFiles/dave_interfaces__rosidl_generator_type_description: rosidl_generator_type_description/dave_interfaces/srv/TransformFromSphericalCoord.json

rosidl_generator_type_description/dave_interfaces/msg/UsblCommand.json: /opt/ros/jazzy/lib/rosidl_generator_type_description/rosidl_generator_type_description
rosidl_generator_type_description/dave_interfaces/msg/UsblCommand.json: /opt/ros/jazzy/lib/python3.12/site-packages/rosidl_generator_type_description/__init__.py
rosidl_generator_type_description/dave_interfaces/msg/UsblCommand.json: rosidl_adapter/dave_interfaces/msg/UsblCommand.idl
rosidl_generator_type_description/dave_interfaces/msg/UsblCommand.json: rosidl_adapter/dave_interfaces/msg/UsblResponse.idl
rosidl_generator_type_description/dave_interfaces/msg/UsblCommand.json: rosidl_adapter/dave_interfaces/msg/Location.idl
rosidl_generator_type_description/dave_interfaces/msg/UsblCommand.json: rosidl_adapter/dave_interfaces/srv/SetOriginSphericalCoord.idl
rosidl_generator_type_description/dave_interfaces/msg/UsblCommand.json: rosidl_adapter/dave_interfaces/srv/GetOriginSphericalCoord.idl
rosidl_generator_type_description/dave_interfaces/msg/UsblCommand.json: rosidl_adapter/dave_interfaces/srv/TransformToSphericalCoord.idl
rosidl_generator_type_description/dave_interfaces/msg/UsblCommand.json: rosidl_adapter/dave_interfaces/srv/TransformFromSphericalCoord.idl
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --blue --bold --progress-dir=/home/abhimanyu/dave_ws/src/dave/build/dave_interfaces/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Generating type hashes for ROS interfaces"
	/usr/bin/python3 /opt/ros/jazzy/lib/rosidl_generator_type_description/rosidl_generator_type_description --generator-arguments-file /home/abhimanyu/dave_ws/src/dave/build/dave_interfaces/rosidl_generator_type_description__arguments.json

rosidl_generator_type_description/dave_interfaces/msg/UsblResponse.json: rosidl_generator_type_description/dave_interfaces/msg/UsblCommand.json
	@$(CMAKE_COMMAND) -E touch_nocreate rosidl_generator_type_description/dave_interfaces/msg/UsblResponse.json

rosidl_generator_type_description/dave_interfaces/msg/Location.json: rosidl_generator_type_description/dave_interfaces/msg/UsblCommand.json
	@$(CMAKE_COMMAND) -E touch_nocreate rosidl_generator_type_description/dave_interfaces/msg/Location.json

rosidl_generator_type_description/dave_interfaces/srv/SetOriginSphericalCoord.json: rosidl_generator_type_description/dave_interfaces/msg/UsblCommand.json
	@$(CMAKE_COMMAND) -E touch_nocreate rosidl_generator_type_description/dave_interfaces/srv/SetOriginSphericalCoord.json

rosidl_generator_type_description/dave_interfaces/srv/GetOriginSphericalCoord.json: rosidl_generator_type_description/dave_interfaces/msg/UsblCommand.json
	@$(CMAKE_COMMAND) -E touch_nocreate rosidl_generator_type_description/dave_interfaces/srv/GetOriginSphericalCoord.json

rosidl_generator_type_description/dave_interfaces/srv/TransformToSphericalCoord.json: rosidl_generator_type_description/dave_interfaces/msg/UsblCommand.json
	@$(CMAKE_COMMAND) -E touch_nocreate rosidl_generator_type_description/dave_interfaces/srv/TransformToSphericalCoord.json

rosidl_generator_type_description/dave_interfaces/srv/TransformFromSphericalCoord.json: rosidl_generator_type_description/dave_interfaces/msg/UsblCommand.json
	@$(CMAKE_COMMAND) -E touch_nocreate rosidl_generator_type_description/dave_interfaces/srv/TransformFromSphericalCoord.json

dave_interfaces__rosidl_generator_type_description: CMakeFiles/dave_interfaces__rosidl_generator_type_description
dave_interfaces__rosidl_generator_type_description: rosidl_generator_type_description/dave_interfaces/msg/Location.json
dave_interfaces__rosidl_generator_type_description: rosidl_generator_type_description/dave_interfaces/msg/UsblCommand.json
dave_interfaces__rosidl_generator_type_description: rosidl_generator_type_description/dave_interfaces/msg/UsblResponse.json
dave_interfaces__rosidl_generator_type_description: rosidl_generator_type_description/dave_interfaces/srv/GetOriginSphericalCoord.json
dave_interfaces__rosidl_generator_type_description: rosidl_generator_type_description/dave_interfaces/srv/SetOriginSphericalCoord.json
dave_interfaces__rosidl_generator_type_description: rosidl_generator_type_description/dave_interfaces/srv/TransformFromSphericalCoord.json
dave_interfaces__rosidl_generator_type_description: rosidl_generator_type_description/dave_interfaces/srv/TransformToSphericalCoord.json
dave_interfaces__rosidl_generator_type_description: CMakeFiles/dave_interfaces__rosidl_generator_type_description.dir/build.make
.PHONY : dave_interfaces__rosidl_generator_type_description

# Rule to build all files generated by this target.
CMakeFiles/dave_interfaces__rosidl_generator_type_description.dir/build: dave_interfaces__rosidl_generator_type_description
.PHONY : CMakeFiles/dave_interfaces__rosidl_generator_type_description.dir/build

CMakeFiles/dave_interfaces__rosidl_generator_type_description.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/dave_interfaces__rosidl_generator_type_description.dir/cmake_clean.cmake
.PHONY : CMakeFiles/dave_interfaces__rosidl_generator_type_description.dir/clean

CMakeFiles/dave_interfaces__rosidl_generator_type_description.dir/depend:
	cd /home/abhimanyu/dave_ws/src/dave/build/dave_interfaces && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/abhimanyu/dave_ws/src/dave/dave_interfaces /home/abhimanyu/dave_ws/src/dave/dave_interfaces /home/abhimanyu/dave_ws/src/dave/build/dave_interfaces /home/abhimanyu/dave_ws/src/dave/build/dave_interfaces /home/abhimanyu/dave_ws/src/dave/build/dave_interfaces/CMakeFiles/dave_interfaces__rosidl_generator_type_description.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : CMakeFiles/dave_interfaces__rosidl_generator_type_description.dir/depend

