#include <print>
#include <string>
#include <vector>
#include <fstream>
#include <iterator>
#include <chrono>

#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h> // main header
#include <SDL3/SDL_main.h> // entry point
#include <SDL3_image/SDL_image.h> // image loading

#include <glbinding/gl/gl.h> // opengl headers
#include <glbinding/glbinding.h> // main header
#include <glbinding-aux/debug.h> // utils
using namespace gl;

#include <glm/glm.hpp> // main header
#include <glm/ext/matrix_transform.hpp> // translation, rotation, etc
#include <glm/gtc/type_ptr.hpp> // obtain pointers

#include <assimp/scene.h> // ability to import "scenes" (multiple objects in one)
#include <assimp/material.h> // material properties
#include <assimp/Importer.hpp> // main importer for model files
#include <assimp/postprocess.h> // post-processing of model files