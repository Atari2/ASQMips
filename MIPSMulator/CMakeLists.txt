cmake_minimum_required (VERSION 3.15)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
include(FetchContent)
FetchContent_Declare(
	ARLib
	GIT_REPOSITORY "https://github.com/Atari2/ARLib"
	GIT_TAG master
)
FetchContent_MakeAvailable(ARLib)
add_executable(MIPSMulator 
    main.cpp
    InstructionParser.h
    InstructionParser.cpp
    DataParser.h
    DataParser.cpp
)
target_include_directories(MIPSMulator SYSTEM PUBLIC ${ARLib_SOURCE_DIR})
target_link_libraries(MIPSMulator PUBLIC ARLib)
if (WIN32)
	if (CMAKE_BUILD_TYPE STREQUAL "Debug")
		message(STATUS "Debug build")
		target_compile_definitions(MIPSMulator PUBLIC "DBG_NEW=new(_NORMAL_BLOCK,__FILE__,__LINE__)")
	else()
		message(STATUS "${CMAKE_BUILD_TYPE} build")
		target_compile_definitions(MIPSMulator PUBLIC "DBG_NEW=new")
	endif()
	target_link_libraries(MIPSMulator PUBLIC dbghelp)
else()
	target_compile_options(MIPSMulator PUBLIC "-fsanitize=leak,undefined" "-g")
	target_link_options(MIPSMulator PUBLIC "-fsanitize=leak,undefined")
	target_compile_definitions(MIPSMulator PUBLIC "DBG_NEW=new")
endif()