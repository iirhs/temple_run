#pragma once

// sizeof(Vertex) = sizeof(float) * 4 = 16 bytes
struct Vertex {
    glm::vec3 position; // x, y, z
    glm::vec3 normal; // x, y, z
    glm::vec2 uv; // u, v coordinates for textures
    glm::vec4 color; // r, g, b, a
};
// sizeof(Index) = 4 bytes
using Index = GLuint;

// options for static meshes
enum class Primitive { eCube, eSphere };

struct Mesh {
private:
    auto create_cube() -> std::pair<std::vector<Vertex>, std::vector<Index>> {
        float p = +0.5f; // for readability
        float n = -0.5f; // for readability
        std::vector<Vertex> vertices = {
            Vertex{ glm::vec3{n, n, p}, glm::vec3(0, 0, +1), glm::vec2{0.33, 0.75}, glm::vec4{1, 0, 0, 1} }, // front
            Vertex{ glm::vec3{p, n, p}, glm::vec3(0, 0, +1), glm::vec2{0.66, 0.75}, glm::vec4{1, 0, 0, 1} },
            Vertex{ glm::vec3{n, p, p}, glm::vec3(0, 0, +1), glm::vec2{0.33, 0.50}, glm::vec4{1, 0, 0, 1} },
            Vertex{ glm::vec3{p, p, p}, glm::vec3(0, 0, +1), glm::vec2{0.66, 0.50}, glm::vec4{1, 0, 0, 1} },
            Vertex{ glm::vec3{n, n, n}, glm::vec3(0, 0, -1), glm::vec2{0.33, 0.00}, glm::vec4{1, 0, 0, 1} }, // back
            Vertex{ glm::vec3{p, n, n}, glm::vec3(0, 0, -1), glm::vec2{0.66, 0.00}, glm::vec4{1, 0, 0, 1} },
            Vertex{ glm::vec3{n, p, n}, glm::vec3(0, 0, -1), glm::vec2{0.33, 0.25}, glm::vec4{1, 0, 0, 1} },
            Vertex{ glm::vec3{p, p, n}, glm::vec3(0, 0, -1), glm::vec2{0.66, 0.25}, glm::vec4{1, 0, 0, 1} },
            Vertex{ glm::vec3{n, n, n}, glm::vec3(-1, 0, 0), glm::vec2{0.00, 0.50}, glm::vec4{0, 1, 0, 1} }, // left
            Vertex{ glm::vec3{n, n, p}, glm::vec3(-1, 0, 0), glm::vec2{0.00, 0.25}, glm::vec4{0, 1, 0, 1} },
            Vertex{ glm::vec3{n, p, n}, glm::vec3(-1, 0, 0), glm::vec2{0.33, 0.50}, glm::vec4{0, 1, 0, 1} },
            Vertex{ glm::vec3{n, p, p}, glm::vec3(-1, 0, 0), glm::vec2{0.33, 0.25}, glm::vec4{0, 1, 0, 1} },
            Vertex{ glm::vec3{p, n, n}, glm::vec3(+1, 0, 0), glm::vec2{1.00, 0.50}, glm::vec4{0, 1, 0, 1} }, // right
            Vertex{ glm::vec3{p, n, p}, glm::vec3(+1, 0, 0), glm::vec2{1.00, 0.25}, glm::vec4{0, 1, 0, 1} },
            Vertex{ glm::vec3{p, p, n}, glm::vec3(+1, 0, 0), glm::vec2{0.66, 0.50}, glm::vec4{0, 1, 0, 1} },
            Vertex{ glm::vec3{p, p, p}, glm::vec3(+1, 0, 0), glm::vec2{0.66, 0.25}, glm::vec4{0, 1, 0, 1} },
            Vertex{ glm::vec3{n, p, n}, glm::vec3(0, +1, 0), glm::vec2{0.33, 0.25}, glm::vec4{0, 0, 1, 1} }, // top
            Vertex{ glm::vec3{n, p, p}, glm::vec3(0, +1, 0), glm::vec2{0.33, 0.50}, glm::vec4{0, 0, 1, 1} },
            Vertex{ glm::vec3{p, p, n}, glm::vec3(0, +1, 0), glm::vec2{0.66, 0.25}, glm::vec4{0, 0, 1, 1} },
            Vertex{ glm::vec3{p, p, p}, glm::vec3(0, +1, 0), glm::vec2{0.66, 0.50}, glm::vec4{0, 0, 1, 1} },
            Vertex{ glm::vec3{n, n, n}, glm::vec3(0, -1, 0), glm::vec2{0.33, 0.75}, glm::vec4{0, 0, 1, 1} }, // bottom
            Vertex{ glm::vec3{n, n, p}, glm::vec3(0, -1, 0), glm::vec2{0.33, 1.00}, glm::vec4{0, 0, 1, 1} },
            Vertex{ glm::vec3{p, n, n}, glm::vec3(0, -1, 0), glm::vec2{0.66, 0.75}, glm::vec4{0, 0, 1, 1} },
            Vertex{ glm::vec3{p, n, p}, glm::vec3(0, -1, 0), glm::vec2{0.66, 1.00}, glm::vec4{0, 0, 1, 1} },
        };
        std::vector<Index> indices = {
             0,  1,  3,  3,  2,  0, // front
             5,  4,  7,  7,  4,  6, // back
             8,  9, 11, 11, 10,  8, // left
            13, 12, 15, 15, 12, 14, // right
            16, 17, 19, 19, 18, 16, // top
            23, 21, 20, 23, 20, 22, // bottom
        };
        return { vertices, indices };
    }
    auto create_sphere() -> std::pair<std::vector<Vertex>, std::vector<Index>> {
        // https://www.songho.ca/opengl/gl_sphere.html
        float sector_count = 32;
        float stack_count = 32;
        float pi = 3.14159265358979323846f;
        float radius = 0.5f;
        // precalc expensive operations
        float length_recip = 1.0f / radius;
        float sector_step = 2.0f * pi / static_cast<float>(sector_count);
        float stack_step = pi / static_cast<float>(stack_count);

        // preallocate some space for vertices
        std::vector<Vertex> vertices;
        vertices.reserve((sector_count + 1) * (stack_count + 1));

        // create vertices
        for (uint32_t i = 0; i <= stack_count; i++) {
            float stack_angle = pi / 2.0f - static_cast<float>(i) * stack_step;
            float xy = radius * std::cos(stack_angle);
            float z = radius * std::sin(stack_angle);

            for (uint32_t k = 0; k <= sector_count; k++) {
                Vertex& vertex = vertices.emplace_back();

                // calculate position
                float sector_angle = static_cast<float>(k) * sector_step;
                vertex.position.x = xy * std::cos(sector_angle);
                vertex.position.y = xy * std::sin(sector_angle);
                vertex.position.z = z;

                // calculate normal
                vertex.normal = vertex.position * length_recip;

                // calculate uv/st coordinates
                vertex.color = glm::vec4(1, 1, 1, 1);
            }
        }

        // create indices
        // k1--k1+1
        // |  / |
        // | /  |
        // k2--k2+1
        std::vector<uint32_t> indices;
        for (uint32_t i = 0; i < stack_count; i++) {
            uint32_t k1 = i * (sector_count + 1); // beginning of current stack
            uint32_t k2 = k1 + sector_count + 1;  // beginning of next stack

            for (uint32_t j = 0; j < (uint32_t)sector_count; j++, k1++, k2++) {
                // 2 triangles per sector excluding first and last stacks
                if (i != 0) {
                    indices.insert(indices.end(), {
                        k1, k2, k1 + 1
                    });
                }
                if (i != stack_count - 1) {
                    indices.insert(indices.end(), {
                        k1 + 1, k2, k2 + 1
                    });
                }
            }
        }
        return { vertices, indices };
    }
    void describe_buffers(const std::vector<Vertex>& vertices, const std::vector<Index>& indices) {
        // make sure to update the index count
        _index_count = indices.size();

        // create GPU buffer to store vertices
        glCreateBuffers(1, &_buffer_vertices);
        // upload to GPU
        glNamedBufferStorage(_buffer_vertices,
            vertices.size() * sizeof(Vertex), // number of bytes to upload
            vertices.data(), // raw data to upload
            BufferStorageMask::GL_NONE_BIT);

        // create index buffer the same way
        glCreateBuffers(1, &_buffer_indices);
        // upload to GPU
        glNamedBufferStorage(_buffer_indices,
            indices.size() * sizeof(Index),
            indices.data(),
            BufferStorageMask::GL_NONE_BIT);

        // create vertex array (basically a mesh)
        glCreateVertexArrays(1, &_buffer_mesh);
        // assign vertex and index buffer
        glVertexArrayVertexBuffer(_buffer_mesh, 0, _buffer_vertices, 0, sizeof(Vertex));
        glVertexArrayElementBuffer(_buffer_mesh, _buffer_indices);

        // describe the data inside each Vertex
        // Vertex::position
        glVertexArrayAttribFormat(_buffer_mesh, 0, 3, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribBinding(_buffer_mesh, 0, 0);
        glEnableVertexArrayAttrib(_buffer_mesh, 0);
        // Vertex::normal
        glVertexArrayAttribFormat(_buffer_mesh, 1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex::position));
        glVertexArrayAttribBinding(_buffer_mesh, 1, 0);
        glEnableVertexArrayAttrib(_buffer_mesh, 1);
        // Vertex::uv
        glVertexArrayAttribFormat(_buffer_mesh, 2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex::position) + sizeof(Vertex::normal));
        glVertexArrayAttribBinding(_buffer_mesh, 2, 0);
        glEnableVertexArrayAttrib(_buffer_mesh, 2);
        // Vertex::color
        glVertexArrayAttribFormat(_buffer_mesh, 3, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex::position) + sizeof(Vertex::normal) + sizeof(Vertex::uv));
        glVertexArrayAttribBinding(_buffer_mesh, 3, 0);
        glEnableVertexArrayAttrib(_buffer_mesh, 3);
    }

public:
    void init(Primitive primitive) {
        std::vector<Vertex> vertices;
        std::vector<Index> indices;
        switch (primitive) {
            case Primitive::eCube: {
                std::tie(vertices, indices) = create_cube();
                break;
            }
            case Primitive::eSphere: {
                std::tie(vertices, indices) = create_sphere();
                break;
            }
            default: {
                std::println("Invalid mesh primitive chosen");
                exit(1);
            }
        }

        // describe GPU buffers
        describe_buffers(vertices, indices);
    }
    void init(aiMesh* mesh_p) {
        // make aiMesh easier to use
        aiMesh& mesh = *mesh_p;

        // index into the material vector in model.hpp
        _material_index = mesh.mMaterialIndex;

        // extract vertices from aiMesh
        std::vector<Vertex> vertices;
        vertices.reserve(mesh.mNumVertices);
        for (uint32_t i = 0; i < mesh.mNumVertices; i++) {
            Vertex vertex;
            // extract positions
            vertex.position.x = mesh.mVertices[i].x;
            vertex.position.y = mesh.mVertices[i].y;
            vertex.position.z = mesh.mVertices[i].z;
            // extract normals
            vertex.normal.x = mesh.mNormals[i].x;
            vertex.normal.y = mesh.mNormals[i].y;
            vertex.normal.z = mesh.mNormals[i].z;
            // potentially extract uv/st coords
            if (mesh.HasTextureCoords(0)) {
                vertex.uv.s = mesh.mTextureCoords[0][i].x;
                vertex.uv.t = mesh.mTextureCoords[0][i].y;
            }
            else vertex.uv = {0, 0};
            // potentially extract vertex colors
            if (mesh.HasVertexColors(0)) {
                vertex.color.r = mesh.mColors[0][i].r;
                vertex.color.g = mesh.mColors[0][i].g;
                vertex.color.b = mesh.mColors[0][i].b;
                vertex.color.a = mesh.mColors[0][i].a;
            }
            else vertex.color = {1, 1, 1, 1};
            vertices.push_back(vertex);
        }

        // extract indices from aiMesh
        std::vector<uint32_t> indices;
        indices.reserve(mesh.mNumFaces * 3);
        for (int i = 0; i < mesh.mNumFaces; i++) {
            // each face is one triangle
            aiFace face = mesh.mFaces[i];
            for (int j = 0; j < face.mNumIndices; j++) {
                indices.push_back(face.mIndices[j]);
            }
        }

        // describe GPU buffers
        describe_buffers(vertices, indices);
    }
    void destroy() {
        glDeleteBuffers(1, &_buffer_indices);
        glDeleteBuffers(1, &_buffer_vertices);
        glDeleteVertexArrays(1, &_buffer_mesh);
    }

    void bind() {
        glBindVertexArray(_buffer_mesh);
    }
    void draw() {
        glDrawElements(GL_TRIANGLES, _index_count, GL_UNSIGNED_INT, nullptr);
    }

    GLuint _buffer_vertices;
    GLuint _buffer_indices;
    GLuint _buffer_mesh;
    GLuint _index_count;
    GLuint _material_index = 0;
};
