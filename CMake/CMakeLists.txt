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
	add_compile_definitions(PLATFORM_WINDOWS)
	
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
	
	# make the executable 
	add_executable (${PROJECT_NAME} WIN32 ${SOURCE_FILES})
	target_include_directories(${PROJECT_NAME} PUBLIC ${IncludeDirectories})
	target_link_directories(${PROJECT_NAME} PUBLIC ${LibraryDirectories} spdlog::spdlog)

	target_compile_definitions(${PROJECT_NAME} PUBLIC SPDLOG_COMPILED_LIB)
	
	set_property(TARGET ${PROJECT_NAME} PROPERTY
             MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")

	#message(STATUS $ENV{VULKAN_SDK}/Lib/)
	
	include_directories(${PROJECT_SOURCE_CODE_DIR})
endif(WIN32)

