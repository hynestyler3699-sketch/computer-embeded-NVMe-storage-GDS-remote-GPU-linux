cmake_minimum_required(VERSION 3.10)
project(ComputerVisionEmbedded 
    VERSION 0.1.0 
    LANGUAGES C CXX
    DESCRIPTION "GPU-Direct Storage for Computer Vision on NVMe"
)

# C/C++ standards
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build options
option(ENABLE_CUDA "Enable CUDA support" OFF)
option(ENABLE_GDS "Enable GPU-Direct Storage support" OFF)
option(BUILD_TESTS "Build test suite" ON)
option(BUILD_EXAMPLES "Build example programs" ON)
option(BUILD_BENCHMARKS "Build benchmark programs" OFF)

# Compiler flags
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(-O3)
    endif()
endif()

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Find packages
find_package(Threads REQUIRED)

if(ENABLE_CUDA)
    find_package(CUDA)
    if(CUDA_FOUND)
        message(STATUS "CUDA found: ${CUDA_VERSION}")
        include_directories(${CUDA_INCLUDE_DIRS})
        add_definitions(-DHAVE_CUDA)
    else()
        message(WARNING "CUDA not found, disabling CUDA support")
        set(ENABLE_CUDA OFF)
    endif()
endif()

if(ENABLE_GDS)
    # Look for cuFile library
    find_library(CUFILE_LIBRARY cufile PATHS /usr/local/cuda/lib64)
    if(CUFILE_LIBRARY)
        message(STATUS "cuFile found: ${CUFILE_LIBRARY}")
    else()
        message(WARNING "cuFile not found, disabling GDS support")
        set(ENABLE_GDS OFF)
    endif()
endif()

# Source files
set(CV_SOURCES
    src/cv_algorithms/edge_detection.c
    src/cv_algorithms/feature_extraction.c
    src/cv_algorithms/image_processing.c
)

# Create library
add_library(cv_algorithms STATIC ${CV_SOURCES})
target_link_libraries(cv_algorithms PRIVATE m)

if(ENABLE_GDS)
    add_library(gds_runtime
        src/gds/gds_real.c
    )
    target_compile_definitions(gds_runtime PUBLIC ENABLE_GDS HAVE_GDS)
    target_link_libraries(gds_runtime PUBLIC ${CUDA_LIBRARIES} ${CUFILE_LIBRARY})
else()
    add_library(gds_runtime
        src/gds/gds_stub.c
    )
endif()
target_include_directories(gds_runtime PUBLIC include)

# Examples
if(BUILD_EXAMPLES)
    add_executable(basic_usage examples/basic_usage.c)
    target_link_libraries(basic_usage PRIVATE cv_algorithms m)
    
    if(ENABLE_GDS)
        add_executable(gds_integration examples/gds_integration.c)
        target_link_libraries(gds_integration
            PRIVATE
                gds_runtime
                Threads::Threads
        )
    endif()
endif()

# Tests
if(BUILD_TESTS)
    enable_testing()
    
    add_executable(cv_tests tests/unit_tests/sample_test.c)
    target_link_libraries(cv_tests PRIVATE cv_algorithms m)
    
    add_test(NAME cv_unit_tests COMMAND cv_tests)
endif()

# Benchmarks
if(BUILD_BENCHMARKS)
    # Placeholder for benchmark targets
    message(STATUS "Benchmark build enabled")
endif()

# Installation
install(TARGETS cv_algorithms
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)

install(DIRECTORY include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

install(DIRECTORY docs/
    DESTINATION share/doc/${PROJECT_NAME}
)

# Print configuration summary
message(STATUS "")
message(STATUS "=== Configuration Summary ===")
message(STATUS "  Version:       ${PROJECT_VERSION}")
message(STATUS "  CUDA:          ${ENABLE_CUDA}")
message(STATUS "  GDS:           ${ENABLE_GDS}")
message(STATUS "  Tests:         ${BUILD_TESTS}")
message(STATUS "  Examples:      ${BUILD_EXAMPLES}")
message(STATUS "  Benchmarks:    ${BUILD_BENCHMARKS}")
message(STATUS "")
