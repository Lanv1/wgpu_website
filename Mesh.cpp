#include "Mesh.hpp"

using namespace glm;

void parseObjFile(const std::string& filename) {
   
}


Mesh::Mesh()
{
    vertices = std::vector<vec3>(0);
    normals = std::vector<vec3>(0);
    faces = std::vector<Face>(0);
}

Mesh::Mesh
(
    const std::vector<glm::vec3>& _vertices,
    const std::vector<Face>& _faces,
    const std::vector<glm::vec3>& _normals
): vertices(_vertices), faces(_faces), normals(_normals)
{

}

Mesh::Mesh(const std::filesystem::path& filePath)
{
    vertices = std::vector<vec3>(0);
    normals = std::vector<vec3>(0);
    faces = std::vector<Face>(0);

    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open file " << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string token;
        iss >> token;

        if (token == "v") 
        {
            vec3 vertex;
            iss >> vertex.x >> vertex.y >> vertex.z;
            vertices.push_back(vertex);
        } 
        else if (token == "f") 
        {
            Face face;
            for (int i = 0; i < 3; ++i) {

                iss >> face.vertexIndex[i];
                face.vertexIndex[i] --;
                if (iss.peek() == '/') {
                    iss.ignore(); // Skip '/'

                    /*
                    * Enable if using texture coordinates
                    */
                    // if (iss.peek() != '/') {
                    //     iss >> face.textureIndex[i];
                    // }

                    if (iss.peek() == '/') {
                        iss.ignore(); // Skip '/'
                        iss >> face.normalIndex[i];
                        face.normalIndex[i] --;
                    }
                }
            }
            faces.push_back(face);
        }
        else if(token == "vn")
        {
            vec3 normal;
            iss >> normal.x >> normal.y >> normal.z;
            normals.push_back(normal);
        }
    }

    file.close();

    for(const Face face : faces) {
        indices.push_back(face.vertexIndex[0]);
        indices.push_back(face.vertexIndex[1]);
        indices.push_back(face.vertexIndex[2]);
    }


}

void Mesh::dumpInfo()
{
    std::cout<<"Mesh has "<<vertices.size()<<" vertices,"<<faces.size()<<" faces and "<< normals.size()<<" normals."<<std::endl;

}