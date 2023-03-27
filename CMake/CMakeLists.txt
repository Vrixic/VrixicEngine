cmake_minimum_required(VERSION 3.24)

set(PROJECT_NAME VrixicEngine)

# Directory where project code is located 
set(PROJECT_SOURCE_CODE_DIR ${CMAKE_SOURCE_DIR}/../Source)

set(ADDITIONAL_FILES_FOR_EXE
)

project(${PROJECT_NAME})

ADD_DEFINITIONS(-DUNICODE)
ADD_DEFINITIONS(-D_UNICODE)

# Read the file that contains all paths to all source files for the engine
file (STRINGS "AllSourceFiles.txt" SOURCE_FILES)

# Keep a copy of absolute paths 
set (SOURCE_FILES_ABSOLUTE_PATHS ${SOURCE_FILES})

list(APPEND SOURCE_FILES ${ADDITIONAL_FILES_FOR_EXE})

# Make source groups('folders') for all paths relative to /Engine/Source/, Source being root
foreach(Dir ${SOURCE_FILES})	
	get_filename_component(DirectoryPath ${Dir} DIRECTORY) 
	
	string(FIND ${DirectoryPath} Source/ EnginePathIndex REVERSE)
	string(LENGTH ${DirectoryPath} DirectoryPathLength)
	
	#message(STATUS ${DirectoryPath})
	
	string(SUBSTRING ${DirectoryPath} ${EnginePathIndex} ${DirectoryPathLength} DirectoryPath)
	
	#message(STATUS ${DirectoryPath})
	
	SOURCE_GROUP(${DirectoryPath}/ FILES ${Dir})
endforeach()

# Add debugging option
option(USE_DEBUG "Enter debug mode" OFF)
if (USE_DEBUG)
  add_compile_definitions(_DEBUG)
endif()

if (WIN32)	
	# add windows define  
	add_compile_definitions(PLATFORM_WINDOWS VE_BUILD_DLL)
	
	# shaderc_combined.lib in Vulkan requires this for debug & release (runtime shader compiling)
	# set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MD")
	
	#set and add all include directories 
	set (VulkanIncludeDir $ENV{VULKAN_SDK}/Include/)
		
	set (PROJECT_EXTERNAL_DIR ${PROJECT_SOURCE_CODE_DIR}/External)
		
	set (IncludeDirectories
		${VulkanIncludeDir}
		${PROJECT_EXTERNAL_DIR}/Optick/Includes/
		${PROJECT_EXTERNAL_DIR}/spdlog/Includes/
		)

	# set and add all libraries 
	set (LibraryDirectories 
		$ENV{VULKAN_SDK}/Lib/)
	
	set(PROJECT_SOLUTION_DIR ${CMAKE_SOURCE_DIR}/../Build/VrixicEngineBuild)
	set(RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOLUTION_DIR}/bin/$(Configuration)-$(Platform)/$(ProjectName)/)
	set(LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOLUTION_DIR}/lib/$(Configuration)-$(Platform)/$(ProjectName)/)
	set(CFG_INTDIR ${PROJECT_SOLUTION_DIR}/bin-int/$(Configuration)-$(Platform)/$(ProjectName)/)
	
	#message(STATUS ${RUNTIME_OUTPUT_DIRECTORY})
	
	# make the executable 
	set(CMAKE_CFG_INTDIR  ${CFG_INTDIR})
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${RUNTIME_OUTPUT_DIRECTORY})
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${LIBRARY_OUTPUT_DIRECTORY})
	
	#add_executable (${PROJECT_NAME} WIN32 ${SOURCE_FILES})
	#target_include_directories(${PROJECT_NAME} PUBLIC ${IncludeDirectories})
	#target_link_directories(${PROJECT_NAME} PUBLIC ${LibraryDirectories} spdlog::spdlog)
	
	add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES_ABSOLUTE_PATHS})
	target_include_directories(${PROJECT_NAME} PUBLIC ${IncludeDirectories})
	target_link_directories(${PROJECT_NAME} PUBLIC ${LibraryDirectories} spdlog::spdlog)

	target_compile_definitions(${PROJECT_NAME} PUBLIC SPDLOG_COMPILED_LIB)
	#target_compile_definitions(${PROJECT_NAME} PUBLIC SPDLOG_COMPILED_LIB)
	
	set_property(TARGET ${PROJECT_NAME} PROPERTY
             MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
			 
	#set_property(TARGET ${PROJECT_NAME} PROPERTY
	#		 MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")

	#message(STATUS $ENV{VULKAN_SDK}/Lib/)
	
	#add sandbox project
	INCLUDE_EXTERNAL_MSPROJECT(Sandbox ${CMAKE_SOURCE_DIR}/../Sandbox/Sandbox.vcxproj)
	
	# add custom post build event/command to copy the dll made form vrixic engine to sandbox
	add_custom_command(TARGET VrixicEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:VrixicEngine> ${PROJECT_SOLUTION_DIR}/bin/Debug-x64/Sandbox/
	COMMENT "Copying VrixicEngine dll to Sandbox build directory")
	
	# make sand box the start up project
	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Sandbox)
	
	include_directories(${PROJECT_SOURCE_CODE_DIR})
endif(WIN32)

