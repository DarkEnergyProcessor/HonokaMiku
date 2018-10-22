cmake_minimum_required (VERSION 2.6)

project(HonokaMiku)
set(HONOKAMIKU_VERSION_MAJOR 5)
set(HONOKAMIKU_VERSION_MINOR 0)
set(HONOKAMIKU_VERSION_PATCH 3)
set(HONOKAMIKU_VERSION_STRING_RC "5.0.3")

if(MSVC)
	# excuse me wtf
	add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_DEPRECATE)
endif()

configure_file("${PROJECT_SOURCE_DIR}/src/VersionInfo.rc.in" "${PROJECT_BINARY_DIR}/VersionInfo.rc")
include_directories("${PROJECT_BINARY_DIR}")
add_definitions(-DHONOKAMIKU_CONFIGURED)
add_executable(HonokaMiku
	src/CN_Decrypter.cc
	src/EN_Decrypter.cc
	src/Helper.cc
	src/HonokaMiku.cc
	src/JP_Decrypter.cc
	src/TW_Decrypter.cc
	src/V1_Decrypter.cc
	src/V2_Decrypter.cc
	src/V3_Decrypter.cc
	${CMAKE_CURRENT_BINARY_DIR}/VersionInfo.rc
)
install(TARGETS HonokaMiku DESTINATION bin)
