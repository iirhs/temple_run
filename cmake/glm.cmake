# OpenGL function loader
find_package(glm QUIET)
if(NOT glm_FOUND)
    # fetch and build it ourselves
    include(FetchContent)
    FetchContent_Declare(glm
        GIT_REPOSITORY "https://github.com/g-truc/glm.git"
        GIT_TAG "1.0.2"
        GIT_SHALLOW ON)
    FetchContent_MakeAvailable(glm)
endif()
target_link_libraries(${PROJECT_NAME} PRIVATE glm::glm-header-only)