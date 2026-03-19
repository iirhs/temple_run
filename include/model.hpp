#pragma once
#include "mesh.hpp"
#include "texture.hpp"
#include "material.hpp"
#include "transform.hpp"

struct Model {
    void init(Primitive primitive) {
        _meshes.emplace_back().init(primitive);
        _materials.emplace_back()._texture_contribution = 0.0f;
    }
    void init(Primitive primitive, const std::string& texture_path) {
        _meshes.emplace_back().init(primitive);
        _materials.emplace_back()._texture_contribution = 1.0f;
        _textures.emplace_back().init(texture_path);
    }
    void init(const std::string& model_path) {
        // create a temporary model importer
        Assimp::Importer importer;

        // flags that allow some automatic post processing of model
        unsigned int flags = 0; // https://the-asset-importer-lib-documentation.readthedocs.io/en/latest/usage/postprocessing.html
        flags |= aiProcess_PreTransformVertices; // simplifies model load
        flags |= aiProcess_Triangulate; // triangulate all faces if not already triangulated
        flags |= aiProcess_GenNormals; // generate normals if they dont exist
        flags |= aiProcess_FlipUVs; // OpenGL prefers flipped y axis

        // prepare the full path
        std::string base_path = SDL_GetBasePath();
        std::string full_path = base_path + "assets/models/" + model_path;

        // load the entire "scene" (may be multiple meshes, hence scene)
        std::print("Loading {}... ", model_path);
        std::fflush(stdout);
        const aiScene* scene_p = importer.ReadFile(full_path, flags);
        if (scene_p == nullptr) {
            std::println("{}", importer.GetErrorString());
            std::exit(1);
        }
        std::println("done. Contains {} meshes, {} materials and {} textures", scene_p->mNumMeshes, scene_p->mNumMaterials, scene_p->mNumTextures);

        // figure out path to the model root for stuff like .obj, which puts its assets into sub-folders
        std::string model_root = full_path.substr(0, full_path.find_last_of('/') + 1);

        // set containers to the correct sizes
        _meshes.resize(scene_p->mNumMeshes);
        _textures.resize(scene_p->mNumMaterials);
        _materials.resize(scene_p->mNumMaterials);
        
        // create meshes from loaded aiMeshes
        for (uint32_t i = 0; i < scene_p->mNumMeshes; i++) {
            aiMesh* mesh_p = scene_p->mMeshes[i];
            _meshes[i].init(mesh_p);
        }

        // create materials and textures
        for (uint32_t i = 0; i < scene_p->mNumMaterials; i++) {
            aiMaterial& ai_material = *scene_p->mMaterials[i];
            Material& material = _materials[i];

            // load basic material properties
            ai_material.Get(AI_MATKEY_SHININESS_STRENGTH, material._specular);
            ai_material.Get(AI_MATKEY_SHININESS, material._specular_shininess);

            // see if this should use a diffuse texture
            if (ai_material.GetTextureCount(aiTextureType_DIFFUSE) > 0) {
                material._texture_contribution = 1.0;
                // load the texture
                aiString texture_path;
                ai_material.Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0), texture_path);
                // texture folder is in same directory as .obj
                _textures[i].init(texture_path.C_Str(), model_root);
            }
            else material._texture_contribution = 0.0f;
        }
    }
    void destroy() {
        for (auto& mesh: _meshes) {
            mesh.destroy();
        }
        for (auto& texture: _textures) {
            texture.destroy();
        }
    }
    void draw(bool shadow_only = false) {
        // same transform for all sub-meshes
        _transform.bind(shadow_only);

        // bind and draw sets of mesh/material/texture
        for (uint32_t i = 0; i < _meshes.size(); i++) {
            uint32_t material_index = _meshes[i]._material_index;

            // rendering shadow depth maps doesnt need color things
            if (!shadow_only) {
                // bind requested material
                _materials[material_index].bind();

                // only bind the texture when applicable
                if (_materials[material_index]._texture_contribution > 0.0f) {
                    _textures[material_index].bind();
                }
            }

            // finally, draw this mesh
            _meshes[i].bind();
            _meshes[i].draw();
        }
    }

    Transform _transform;
    std::vector<Mesh> _meshes;
    std::vector<Texture> _textures; // TODO: could move textures into material
    std::vector<Material> _materials;
};
