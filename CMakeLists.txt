cmake_minimum_required(VERSION 3.22 FATAL_ERROR)

set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

# Ensure to pick up the default triplet from the environment if any. This helps
# driving the vcpkg triplet in the same way either when starting vcpkg directly,
# or when letting CMake start vcpkg at configure/generate time.
# Note: this logic must happen before PROJECT command.
if (DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif ()

if (DEFINED ENV{VCPKG_DEFAULT_TRIPLET} AND NOT DEFINED VCPKG_TARGET_TRIPLET)
    set(VCPKG_TARGET_TRIPLET "$ENV{VCPKG_DEFAULT_TRIPLET}" CACHE STRING "The vcpkg triplet")
endif ()

set(VCPKG_MANIFEST_NO_DEFAULT_FEATURES ON)

set(BOOST_REQUIRED_VERSION 1.71.0)

option(ENABLE_HTTP "Enable HTTP support" ON)
if (ENABLE_HTTP)
    list(APPEND VCPKG_MANIFEST_FEATURES "http")
    set(BOOST_REQUIRED_VERSION 1.75.0)
endif ()

option(USE_LIBMYSQL "Use libmysql instead of libmariadb (vcpkg only)" OFF)
if (USE_LIBMYSQL)
    list(APPEND VCPKG_MANIFEST_FEATURES "libmysql")
else ()
    list(APPEND VCPKG_MANIFEST_FEATURES "libmariadb")
endif ()

# Selects the Boost.MySQL backend (src/database_boost.cpp) instead of the libmariadb/libmysql C
# client backend (src/database.cpp). The public Database/DBResult API is identical either way.
option(USE_BOOST_MYSQL "Use the Boost.MySQL database backend instead of the C client (vcpkg only)" OFF)
if (USE_BOOST_MYSQL)
    list(APPEND VCPKG_MANIFEST_FEATURES "boost-mysql")
endif ()

option(BUILD_TESTING "Build the testing tree" OFF)
if (BUILD_TESTING)
    list(APPEND VCPKG_MANIFEST_FEATURES "unit-tests")
endif ()

option(BUILD_BENCHMARKING "Build the benchmarking tree" OFF)
if (BUILD_BENCHMARKING)
    list(APPEND VCPKG_MANIFEST_FEATURES "benchmarks")
endif ()

project(tfs CXX)

if (ENABLE_HTTP)
    add_compile_definitions(HTTP)
endif ()

# *****************************************************************************
# Append cmake search path
# *****************************************************************************
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

# *****************************************************************************
# Include cmake tools
# *****************************************************************************
include(MessageColors)
include(LoggingHelper)

# *****************************************************************************
# Options
# *****************************************************************************
option(ENABLE_ASAN "Enable address sanitizer" OFF)
option(ENABLE_UNITY_BUILD "Enable unity build" ON)
option(OPTIONS_ENABLE_CCACHE "Enable ccache" OFF)
option(OPTIONS_ENABLE_SCCACHE "Enable sccache" OFF)
option(OPTIONS_ENABLE_IPO "Check and enable interprocedural optimization (IPO/LTO)" ON)

# *****************************************************************************
# Options Code
# *****************************************************************************

# === ASAN ===
if (ENABLE_ASAN)
    log_option_enabled("asan")
    if (MSVC)
        add_compile_options(/fsanitize=address)
        add_link_options(/fsanitize=address)
    else ()
        add_compile_options(-fsanitize=address)
        add_link_options(-fsanitize=address)
    endif ()
else ()
    log_option_disabled("asan")
endif ()

if (OPTIONS_ENABLE_CCACHE AND OPTIONS_ENABLE_SCCACHE)
    log_war("Both OPTIONS_ENABLE_CCACHE and OPTIONS_ENABLE_SCCACHE are ON; sccache will take precedence.")
endif ()

# === CCACHE ===
if (OPTIONS_ENABLE_CCACHE)
    find_program(CCACHE ccache)
    if (CCACHE)
        log_option_enabled("ccache")
        set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE})
        set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE})
    else ()
        log_option_disabled("ccache")
    endif ()
endif ()

# === SCCACHE ===
if (OPTIONS_ENABLE_SCCACHE)
    find_program(SCCACHE_PATH sccache)
    if (SCCACHE_PATH)
        log_option_enabled("sccache")
        set(CMAKE_C_COMPILER_LAUNCHER ${SCCACHE_PATH})
        set(CMAKE_CXX_COMPILER_LAUNCHER ${SCCACHE_PATH})
    else ()
        log_option_disabled("sccache")
    endif ()
endif ()

# === Unity Build ===
if (ENABLE_UNITY_BUILD)
    log_option_enabled("unity build")
else ()
    log_option_disabled("unity build")
endif ()
set(CMAKE_UNITY_BUILD ${ENABLE_UNITY_BUILD})

if (BUILD_TESTING OR BUILD_BENCHMARKING)
    include(CTest)
endif ()

# *****************************************************************************
# Compiler Options
# *****************************************************************************
if (MSVC)
    add_compile_options(/bigobj /utf-8 /MP /EHsc)
else ()
    add_compile_options(-Wall -Wextra -Wnon-virtual-dtor -Wno-error=old-style-cast -pedantic -Werror -pipe -fvisibility=hidden)
endif ()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    add_compile_options(-fno-strict-aliasing)
endif ()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    add_compile_options(-Wimplicit-fallthrough -Wmove)
endif ()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# *****************************************************************************
# Packages / Dependencies
# *****************************************************************************
find_package(Lua REQUIRED)
# Boost.MySQL needs OpenSSL::SSL for TLS; the C client backend only needs Crypto.
if (USE_BOOST_MYSQL)
    find_package(OpenSSL 3.0.0 REQUIRED COMPONENTS Crypto SSL)
else ()
    find_package(OpenSSL 3.0.0 REQUIRED COMPONENTS Crypto)
endif ()
find_package(PugiXML CONFIG REQUIRED)
find_package(simdutf CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(Threads REQUIRED)
find_package(ZLIB REQUIRED)

# Look for vcpkg-provided packages first
if (USE_LIBMYSQL)
    find_package(unofficial-libmysql CONFIG)
    if (unofficial-libmysql_FOUND)
        set(MYSQL_CLIENT_LIBS "unofficial::libmysql::libmysql")
    endif ()
else ()
    find_package(unofficial-libmariadb CONFIG)
    if (unofficial-libmariadb_FOUND)
        set(MYSQL_CLIENT_LIBS "unofficial::libmariadb")
    endif ()
endif ()

if (NOT MYSQL_CLIENT_LIBS)
    message(STATUS "Using MySQL client library from system")
    find_package(MySQL REQUIRED)
endif ()

set(BOOST_REQUIRED_COMPONENTS iostreams)
if (ENABLE_HTTP)
    list(APPEND BOOST_REQUIRED_COMPONENTS json)
endif ()
if (BUILD_TESTING)
    list(APPEND BOOST_REQUIRED_COMPONENTS unit_test_framework)
endif ()
find_package(Boost ${BOOST_REQUIRED_VERSION} REQUIRED COMPONENTS ${BOOST_REQUIRED_COMPONENTS})

# Boost.MySQL is shipped as a separate vcpkg port exporting the Boost::mysql target. It is only
# required when the Boost.MySQL backend is selected.
if (USE_BOOST_MYSQL)
    find_package(boost_mysql CONFIG REQUIRED)
endif ()

if (BUILD_BENCHMARKING)
    find_package(benchmark REQUIRED)
endif ()

include_directories(${Boost_INCLUDE_DIRS} ${OPENSSL_INCLUDE_DIR} ${LUA_INCLUDE_DIR} ${MYSQL_INCLUDE_DIR} ${PUGIXML_INCLUDE_DIR})

# *****************************************************************************
# IPO / LTO
# *****************************************************************************
if (OPTIONS_ENABLE_IPO)
    include(CheckIPOSupported)
    check_ipo_supported(
        RESULT CMAKE_INTERPROCEDURAL_OPTIMIZATION
        OUTPUT INTERPROCEDURAL_OPTIMIZATION_ERROR
        LANGUAGES CXX
    )
    if (CMAKE_INTERPROCEDURAL_OPTIMIZATION)
        log_option_enabled("IPO/LTO")
    else ()
        log_option_disabled("IPO/LTO: ${INTERPROCEDURAL_OPTIMIZATION_ERROR}")
    endif ()
else ()
    log_option_disabled("IPO/LTO")
endif ()

# *****************************************************************************
# Add project
# *****************************************************************************
add_subdirectory(src)
