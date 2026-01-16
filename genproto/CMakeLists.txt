set(PROTO_SRC_DIR "${CMAKE_SOURCE_DIR}/proto")
set(PROTO_GEN_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

file(GLOB PROTO_FILES "${PROTO_SRC_DIR}/*.proto")

set(_PROTOC_EXE $<TARGET_FILE:protobuf::protoc>)
set(_GRPC_PLUGIN_EXE $<TARGET_FILE:grpc_cpp_plugin>)

foreach(proto_file ${PROTO_FILES})
    get_filename_component(proto_name ${proto_file} NAME_WE)

    set(_pb_cc "${PROTO_GEN_DIR}/${proto_name}.pb.cc")`
    set(_pb_h  "${PROTO_GEN_DIR}/${proto_name}.pb.h")
    set(_grpc_cc "${PROTO_GEN_DIR}/${proto_name}.grpc.pb.cc")
    set(_grpc_h  "${PROTO_GEN_DIR}/${proto_name}.grpc.pb.h")


    add_custom_command(
        OUTPUT ${_pb_cc} ${_pb_h} ${_grpc_cc} ${_grpc_h}
        COMMAND ${_PROTOC_EXE}
        ARGS --grpc_out=${PROTO_GEN_DIR}
             --cpp_out=${PROTO_GEN_DIR}
             -I "${PROTO_SRC_DIR}"
             --plugin=protoc-gen-grpc=${_GRPC_PLUGIN_EXE}
             "${proto_file}"
        DEPENDS "${proto_file}" protobuf::protoc grpc_cpp_plugin
        COMMENT "Compiling proto: ${proto_name}"
        VERBATIM
    )

    list(APPEND PROTO_GENERATED_FILES ${_pb_cc} ${_pb_h} ${_grpc_cc} ${_grpc_h})

endforeach()


add_library(proto-lib STATIC ${PROTO_GENERATED_FILES})

target_include_directories(proto-lib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(proto-lib PUBLIC
    grpc++
    libprotobuf
)