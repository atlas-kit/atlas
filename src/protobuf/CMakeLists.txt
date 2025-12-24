# CMakeLists.txt for protobuf-generated files and appearances loader

find_package(Protobuf CONFIG REQUIRED)

# Generate C++ files from proto
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS appearances.proto)

# Create library with generated protobuf code and appearances loader
add_library(appearances_proto
	${PROTO_SRCS}
	${PROTO_HDRS}
	appearances.cpp
	appearances.h
)

# Include the generated headers directory and current source dir
target_include_directories(appearances_proto PUBLIC
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
)

# Link protobuf
target_link_libraries(appearances_proto PUBLIC protobuf::libprotobuf)

# Need access to otpch.h from parent directory
target_include_directories(appearances_proto PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)

# Disable warnings for generated code only
if(MSVC)
	set_source_files_properties(${PROTO_SRCS} PROPERTIES COMPILE_OPTIONS /w)
else()
	set_source_files_properties(${PROTO_SRCS} PROPERTIES COMPILE_OPTIONS -w)
endif()
