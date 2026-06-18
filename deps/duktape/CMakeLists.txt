cmake_minimum_required(VERSION 3.12)
project(duktape-static C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Duktape source files
set(DUKTAPE_SOURCES
    duktape.c
)

# Create static library
add_library(duktape-static STATIC ${DUKTAPE_SOURCES})

# Compile with position independent code for static linking
set_target_properties(duktape-static PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    C_STANDARD 11
)

# Optimization flags
target_compile_options(duktape-static PRIVATE
    -O2
    -fomit-frame-pointer
    -fno-exceptions
    -fno-rtti
)

# Duktape defines
target_compile_definitions(duktape-static PRIVATE
    DUK_OPT_NO_FILE_IO
    DUK_OPT_NO_JSONX
    DUK_OPT_NO_SECTION_B
    DUK_OPT_PACKED_TVAL
    DUK_OPT_FASTINT
    DUK_OPT_HAVE_CUSTOM_H
)