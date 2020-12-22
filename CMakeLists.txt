cmake_minimum_required (VERSION 3.0)

if(POLICY CMP0091)
	cmake_policy(SET CMP0091 NEW)
endif()

project(HonokaMiku LANGUAGES CXX)
set(HONOKAMIKU_VERSION_MAJOR 5)
set(HONOKAMIKU_VERSION_MINOR 0)
set(HONOKAMIKU_VERSION_PATCH 3)
set(HONOKAMIKU_VERSION_STRING_RC "5.0.3")

get_directory_property(HAS_PARENT PARENT_DIRECTORY)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/src/VersionInfo.rc.in" "${CMAKE_CURRENT_BINARY_DIR}/VersionInfo.rc")

# HonokaMiku library
add_library(HonokaMiku STATIC
	src/CN_Decrypter.cc
	src/EN_Decrypter.cc
	src/Helper.cc
	src/JP_Decrypter.cc
	src/TW_Decrypter.cc
	src/V1_Decrypter.cc
	src/V2_Decrypter.cc
	src/V3_Decrypter.cc
	${CMAKE_CURRENT_BINARY_DIR}/VersionInfo.rc
)
target_compile_definitions(HonokaMiku PUBLIC HONOKAMIKU_CONFIGURED)
target_include_directories(HonokaMiku PUBLIC "${CMAKE_CURRENT_BINARY_DIR}")
install(TARGETS HonokaMiku DESTINATION lib)

# HonokaMiku executable
if(NOT HAS_PARENT)
	add_executable(HonokaMikuExe
		src/HonokaMiku.cc
	)
	target_compile_definitions(HonokaMikuExe PUBLIC HONOKAMIKU_CONFIGURED)
	target_link_libraries(HonokaMikuExe HonokaMiku)
	set_target_properties(HonokaMikuExe PROPERTIES OUTPUT_NAME HonokaMiku)
	install(TARGETS HonokaMikuExe DESTINATION bin)
endif()

if(MSVC)
	# excuse me wtf
	target_compile_definitions(HonokaMiku PRIVATE
		_CRT_SECURE_NO_WARNINGS
		_CRT_SECURE_NO_DEPRECATE
	)

	if(NOT HAS_PARENT)
		target_compile_definitions(HonokaMikuExe PRIVATE
			_CRT_SECURE_NO_WARNINGS
			_CRT_SECURE_NO_DEPRECATE
		)
		set_target_properties(HonokaMiku PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
		set_target_properties(HonokaMikuExe PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
	endif()
endif()
