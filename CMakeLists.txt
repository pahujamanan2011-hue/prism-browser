cmake_minimum_required(VERSION 3.12)
project(PrismBrowser C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -O2 -march=native -pipe")

# Find required packages
find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(CURL REQUIRED)
find_package(OpenSSL REQUIRED)

# Configure config.h
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

# Include directories
include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_TTF_INCLUDE_DIRS}
    ${CURL_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/duktape
)

# Add subdirectories
add_subdirectory(deps/duktape)

# Source files
set(SOURCES
    src/main.c
    src/window.c
    src/renderer.c
    src/parser.c
    src/network.c
    src/blackhole.c
    src/js_runtime.c
    src/image_loader.c
    src/keybinds.c
)

# Create executable
add_executable(prism-browser ${SOURCES})

# Link libraries
target_link_libraries(prism-browser
    ${SDL2_LIBRARIES}
    ${SDL2_TTF_LIBRARIES}
    ${CURL_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    duktape-static
    pthread
    m
)

# Copy config.h to build directory
add_custom_command(TARGET prism-browser POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_BINARY_DIR}/config.h
        ${CMAKE_CURRENT_BINARY_DIR}/config.h
    COMMENT "Copying config.h to build directory"
)
