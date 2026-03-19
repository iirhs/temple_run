# assimp model loader
find_package(assimp QUIET)
if(NOT assimp_FOUND)
    # fetch and build it ourselves
    include(FetchContent)

    # disable stuff we dont need
    set(ASSIMP_INSTALL OFF)
    set(ASSIMP_NO_EXPORT ON)
    set(ASSIMP_BUILD_TESTS OFF)
    set(ASSIMP_BUILD_ASSIMP_VIEW OFF)
    # enable other importers here via ASSIMP_BUILD_XXX_IMPORTER
    set(ASSIMP_BUILD_OBJ_IMPORTER ON)
    set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF)
    FetchContent_Declare(assimp
        GIT_REPOSITORY "https://github.com/assimp/assimp.git"
        GIT_TAG "v6.0.2"
        GIT_SHALLOW ON)
    FetchContent_MakeAvailable(assimp)
endif()
target_link_libraries(${PROJECT_NAME} PRIVATE assimp::assimp)

# download the sponza model into the assets/models folder
# do not forget to add "models/sponza" to your .gitignore!
include(FetchContent)
FetchContent_Declare(sponza
    GIT_REPOSITORY "https://github.com/jimmiebergmann/Sponza.git"
    GIT_TAG "master"
    GIT_SHALLOW ON
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/assets/models/sponza")
FetchContent_MakeAvailable(sponza)