cmake_minimum_required(VERSION 3.0)
project(MyProject)

# Set C++11 standard
set(CMAKE_CXX_STANDARD 11)

include_directories(include)
# Add source files
set(SOURCES
    src/MaxPoolSE.cpp
    src/reshape.cpp
    src/perceptron.cpp
    src/conv2d.cpp
    src/cnn.cpp
)

# Add test files
set(TEST
    test/main.cpp
    )

# Create executables
add_executable(main ${SOURCES} test/main.cpp)
# Set output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)

