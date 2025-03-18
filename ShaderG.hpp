//
//  Shader.hpp
//  Lab3
//
//  Created by CGIS on 05/10/2016.
//  Copyright © 2016 CGIS. All rights reserved.
//

#ifndef ShaderG_hpp
#define ShaderG_hpp

#if defined (__APPLE__)
#define GL_SILENCE_DEPRECATION
    #include <OpenGL/gl3.h>
#else
#define GLEW_STATIC
#include <GL/glew.h>
#endif

#include <fstream>
#include <sstream>
#include <iostream>


namespace gps {

    class ShaderG {

    public:
        GLuint shaderProgram;
        void loadShader(std::string vertexShaderFileName, std::string fragmentShaderFileName);
        void useShaderProgram();

    private:
        std::string readShaderFile(std::string fileName);
        void shaderCompileLog(GLuint shaderId);
        void shaderLinkLog(GLuint shaderProgramId);
    };

}

#endif /* Shader_hpp */
