cmake_minimum_required(VERSION "3.25")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(MarketDataDissemination)

add_library(nlohmann_json INTERFACE)
target_include_directories(nlohmann_json INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty")

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
#set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Release>:Release>")

set(gRPC_MSVC_STATIC_RUNTIME ON CACHE BOOL "Link gRPC statically" FORCE)
set(protobuf_MSVC_STATIC_RUNTIME ON CACHE BOOL "Link Protobuf statically" FORCE)
set(ABSL_MSVC_STATIC_RUNTIME ON CACHE BOOL "Link Abseil statically" FORCE)

set(protobuf_INSTALL OFF CACHE BOOL "Disable Protobuf install" FORCE)
set(gRPC_INSTALL OFF CACHE BOOL "Disable gRPC install" FORCE)
set(absl_enable_install OFF CACHE BOOL "Disable Abseil install" FORCE)

add_subdirectory(thirdparty)
add_subdirectory(genproto)

file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp")
list(FILTER MY_SOURCES EXCLUDE REGEX ".*-main\\.cpp$")



add_executable("${CMAKE_PROJECT_NAME}_server" source/server-main.cpp ${MY_SOURCES})

target_include_directories("${CMAKE_PROJECT_NAME}_server" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")

target_link_libraries("${CMAKE_PROJECT_NAME}_server" PUBLIC
	proto-lib
    grpc++ 
    libprotobuf
    absl::flags 
    absl::flags_parse
    nlohmann_json)

add_executable("${CMAKE_PROJECT_NAME}_client" source/client-main.cpp ${MY_SOURCES})

target_include_directories("${CMAKE_PROJECT_NAME}_client" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")

target_link_libraries("${CMAKE_PROJECT_NAME}_client" PUBLIC
	proto-lib
    grpc++ 
    libprotobuf
    absl::flags 
    absl::flags_parse
    nlohmann_json)