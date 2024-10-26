cmake_minimum_required(VERSION 3.1)
project(Jerboa)

list(APPEND CMAKE_CXX_SOURCE_FILE_EXTENSIONS shader)

set(CMAKE_CXX_STANDARD 17)
if (RELEASE)
    if (ANDROID)
        message("ANDROID MinSizeRel!")
        # GOOGLE!! (Release is not a thing pos ndk 23 apparently..)
        add_compile_definitions(BUILD_TYPE="MinSizeRel")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-trapping-math -fno-rounding-math -fno-signed-zeros")
    else()
        set(CMAKE_BUILD_TYPE Release)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-trapping-math -fno-rounding-math -fno-signaling-nans -fno-signed-zeros")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -O3 -Wall")
    endif()
else()
    if (ANDROID)
        # GOOGLE!!
        add_compile_definitions(BUILD_TYPE="RelWithDebInfo")
    else()
        set(CMAKE_BUILD_TYPE Debug)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -g -Wall")
    endif()
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")

set(OUTPUT_NAME example)

include_directories(include)

if (WINDOWS)
    add_compile_definitions(WINDOWS)
    if (RELEASE)
        # launch as windows, not console app - so cmd does not open as well
        add_link_options(-mwindows)
    endif ()
elseif(OSX)
    add_compile_definitions(APPLE)
else ()
    add_link_options(-no-pie)
endif()

add_executable(${OUTPUT_NAME}
    "src/example.cpp"
)

find_package(OpenGL REQUIRED)
find_package(X11 REQUIRED)

add_subdirectory(include/glm)

if (NOT ANDROID)

    add_subdirectory(include/glfw)
    # let sources know glew is static
    add_compile_definitions(GLEW_STATIC)
    set(GLEW_SOURCE include/glew/src/glew.c)
    set(GLEW_INCLUDE include/glew/include)

    add_library(glew STATIC ${GLEW_SOURCE} ${GLEW_INCLUDE})
    target_include_directories(glew PUBLIC ${GLEW_INCLUDE} ${OPENGL_INCLUDE_DIRS})

    target_link_libraries(glew
        ${OPENGL_LIBRARY}
    )

endif()

if (ANDROID)
    target_compile_definitions(${OUTPUT_NAME} PUBLIC GLSL_VERSION="300 es")
    target_link_libraries(${OUTPUT_NAME} PUBLIC ${GLES-lib} glm)
elseif(WINDOWS)
    target_compile_definitions(${OUTPUT_NAME} PUBLIC GLSL_VERSION="330")
    target_link_libraries(${OUTPUT_NAME} PUBLIC glew glm ${GLEW_LIBRARIES} ${OPENGL_LIBRARIES} glfw "winmm")
else()
    target_compile_definitions(${OUTPUT_NAME} PUBLIC GLSL_VERSION="330")
    target_link_libraries(${OUTPUT_NAME} PUBLIC glew glm ${GLEW_LIBRARIES} ${OPENGL_LIBRARIES} glfw)
endif()

if (OSX)
    # https://stackoverflow.com/questions/18391487/compiling-with-glfw3-linker-errors-undefined-reference
    target_link_libraries(${OUTPUT_NAME} "-framework Cocoa -framework IOKit -framework CoreVideo")
endif ()

if (WINDOWS)
    target_link_libraries(${OUTPUT_NAME} "winmm")
endif ()

set_target_properties(${OUTPUT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${OUTPUT_NAME}")

if (WINDOWS)
    file(GLOB DLL "${PROJECT_SOURCE_DIR}/common/windows/*.dll")
    file(COPY ${DLL} DESTINATION "${CMAKE_BINARY_DIR}/${OUTPUT_NAME}/")
endif()
