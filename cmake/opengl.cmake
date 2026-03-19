# OpenGL function loader
find_package(glbinding QUIET)
if(NOT glbinding_FOUND)
    # fetch and build it ourselves
    include(FetchContent)
    set(OPTION_BUILD_TOOLS OFF)
    set(OPTION_BUILD_EXAMPLES OFF)
    FetchContent_Declare(glbinding
        GIT_REPOSITORY "https://github.com/cginternals/glbinding.git"
        GIT_TAG "v3.5.0"
        GIT_SHALLOW ON)
    FetchContent_MakeAvailable(glbinding)
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE
    glbinding::glbinding
    glbinding::glbinding-aux)