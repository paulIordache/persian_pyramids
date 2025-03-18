#ifndef Model3D_hpp
#define Model3D_hpp

#include "Mesh.hpp"

#include "tiny_obj_loader.h"
#include "stb_image.h"
#include "glm/glm.hpp"//core glm functionality
#include "glm/gtc/matrix_transform.hpp"//glm extension for generating common transformation matrices
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_inverse.hpp"

#include <iostream>
#include <string>
#include <vector>

namespace gps {

    class Model3D {

    public:
        ~Model3D();

		void LoadModel(std::string fileName, gps::Shader shader);

		void LoadModel(std::string fileName, std::string basePath, gps::Shader shader);

		void Draw(gps::Shader shaderProgram);

        GLuint ReadTextureFromFile(const char* file_name);


    private:
		// Component meshes - group of objects
        std::vector<gps::Mesh> meshes;
		// Associated textures
        std::vector<gps::Texture> loadedTextures;

		// Does the parsing of the .obj file and fills in the data structure
		void ReadOBJ(std::string fileName, std::string basePath, gps::Shader shader);

		// Retrieves a texture associated with the object - by its name and type
		gps::Texture LoadTexture(std::string path, std::string type);

		// Reads the pixel data from an image file and loads it into the video memory
    };
}

#endif /* Model3D_hpp */
