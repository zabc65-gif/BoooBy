cmake_minimum_required(VERSION 3.16)
project(BoooBy VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Find SFML (compatible with version 3.x)
find_package(SFML 3 REQUIRED COMPONENTS Graphics Window System Audio)

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Source files
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "include/*.hpp")

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Link SFML (SFML 3 uses capitalized component names)
target_link_libraries(${PROJECT_NAME}
    SFML::Graphics
    SFML::Window
    SFML::System
    SFML::Audio
)

# Copy assets to build directory (configuration time)
file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

# Copy levels directory at build time (to get latest changes)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/levels
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/levels
    COMMENT "Copying levels directory to build folder"
)

# Platform specific settings
if(APPLE)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE TRUE
    )
endif()

if(WIN32)
    # Copy SFML DLLs to output directory on Windows
    if(BUILD_SHARED_LIBS)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:sfml-graphics>
                $<TARGET_FILE:sfml-window>
                $<TARGET_FILE:sfml-system>
                $<TARGET_FILE:sfml-audio>
                $<TARGET_FILE_DIR:${PROJECT_NAME}>
        )
    endif()
endif()
