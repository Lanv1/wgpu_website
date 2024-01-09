#pragma once

#include <cassert>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp> 
#include <glm/matrix.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/scalar_constants.hpp>


struct Face {
    uint32_t vertexIndex[3];
    uint32_t normalIndex[3];
};

struct Mesh
{
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<Face> faces;

    std::vector<uint32_t> indices;

    Mesh();
    Mesh
    (
        const std::vector<glm::vec3>& _vertices,
        const std::vector<Face>& _faces,
        const std::vector<glm::vec3>& _normals = std::vector<glm::vec3>(0)
    );

    /*
    * Store vertices and triangles from .obj file.
    */
    Mesh(const std::filesystem::path& filePath);

    void dumpInfo();
};
