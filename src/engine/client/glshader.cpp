//#include "GLShader.hpp"
#include <GL/glew.h>
#include "shaders.h"

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

#include <engine/console.h>

std::string readFile(const char *filePath) {
    std::string content;
    std::ifstream fileStream(filePath, std::ios::in);

    if(!fileStream.is_open()) {
        //std::cerr << "Could not read file " << filePath << ". File does not exist." << std::endl;
		dbg_msg("gfx", "couldn't load the shader file");
		
        return "";
    }

    std::string line = "";
    while(!fileStream.eof()) {
        std::getline(fileStream, line);
        content.append(line + "\n");
    }

    fileStream.close();
    return content;
}


GLuint LoadShader(const char *vertex_path, const char *fragment_path) {
    GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);

    // Read shaders
    std::string vertShaderStr = readFile(vertex_path);
    std::string fragShaderStr = readFile(fragment_path);
    const char *vertShaderSrc = vertShaderStr.c_str();
    const char *fragShaderSrc = fragShaderStr.c_str();

    GLint result = GL_FALSE;
    int logLength;

    // Compile vertex shader
    //std::cout << "Compiling vertex shader." << std::endl;
    glShaderSource(vertShader, 1, &vertShaderSrc, NULL);
    glCompileShader(vertShader);

    // Check vertex shader
    glGetShaderiv(vertShader, GL_COMPILE_STATUS, &result);
    glGetShaderiv(vertShader, GL_INFO_LOG_LENGTH, &logLength);
    if (!result)
    {
        dbg_msg("gfx", "Could not compile vertex shader %s", vertex_path);
        char *infoLog = (char *) malloc ( sizeof ( char ) * logLength );
        glGetShaderInfoLog(vertShader, logLength, NULL, infoLog);
        dbg_msg("gfx", "%s", infoLog);
        free ( infoLog );
        dbg_msg("gfx", "%s", vertShaderSrc);
        return 0;
    }

    // Compile fragment shader
    //std::cout << "Compiling fragment shader." << std::endl;
    glShaderSource(fragShader, 1, &fragShaderSrc, NULL);
    glCompileShader(fragShader);

    // Check fragment shader
    glGetShaderiv(fragShader, GL_COMPILE_STATUS, &result);
    glGetShaderiv(fragShader, GL_INFO_LOG_LENGTH, &logLength);
    if (!result)
    {
        dbg_msg("gfx", "Could not compile fragment shader %s", fragment_path);
        char *infoLog = (char *) malloc ( sizeof ( char ) * logLength );
        glGetShaderInfoLog(fragShader, logLength, NULL, infoLog);
        dbg_msg("gfx", "%s", infoLog);
        free ( infoLog );
        dbg_msg("gfx", "%s", fragShaderSrc);
        return 0;
    }

    //std::cout << "Linking program" << std::endl;
    GLuint program = glCreateProgram();
    glAttachShader(program, vertShader);
    glAttachShader(program, fragShader);
    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &result);
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
    if (!result)
    {
        dbg_msg("gfx", "Could not link shaders %s %s", vertex_path, fragment_path);
        char *infoLog = (char *) malloc ( sizeof ( char ) * logLength );
        glGetShaderInfoLog(program, logLength, NULL, infoLog);
        dbg_msg("gfx", "%s", infoLog);
        free ( infoLog );
        return 0;
    }

    glDeleteShader(vertShader);
    glDeleteShader(fragShader);

    return program;
}
