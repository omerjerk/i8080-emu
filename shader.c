#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "shader.h"

/* copy a shader from a plain text file into a character array */
static bool parseFileIntoStr(const char *fileName, char **shader_str) {
    FILE *file = fopen(fileName, "r" );
    if (!file) {
        printf( "ERROR: opening file for reading: %s\n", fileName );
        return false;
    }
    fseek(file, 0L, SEEK_END);
    int fsize = ftell(file);
    fseek(file, 0L, SEEK_SET);
    *shader_str = malloc(fsize);
    size_t cnt = fread(*shader_str, fsize, 1, file);
    if (ferror(file)) {
        printf("ERROR: reading shader file %s\n", fileName);
        fclose(file);
        return false;
    }
    // append \0 to end of file string
    (*shader_str)[fsize] = '\0';
    fclose(file);
    return true;
}

GLuint loadShaders(const char* vertexFilePath, const char* fragmentFilePath) {

    // Create the shaders
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    // Read the Vertex Shader code from the file
    const char* vertexShaderCode;
    char* tempVShader;
    if (!parseFileIntoStr(vertexFilePath, &tempVShader)) {
        return 0;
    }
    vertexShaderCode = tempVShader;

    // Read the Fragment Shader code from the file
    const char* fragmentShaderCode;
    char* tempFShader;
    if(!parseFileIntoStr(fragmentFilePath, &tempFShader)){
        return 0;
    }
    fragmentShaderCode = tempFShader;

    GLint Result = GL_FALSE;
    int InfoLogLength;


    // Compile Vertex Shader
    printf("Compiling shader : %s\n", vertexFilePath);
    glShaderSource(VertexShaderID, 1, &vertexShaderCode , NULL);
    glCompileShader(VertexShaderID);

    // Check Vertex Shader
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0){
        char VertexShaderErrorMessage[InfoLogLength+1];
        glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, VertexShaderErrorMessage);
        printf("%s\n", &VertexShaderErrorMessage[0]);
    }

    // Compile Fragment Shader
    printf("Compiling shader : %s\n", fragmentFilePath);
    glShaderSource(FragmentShaderID, 1, &fragmentShaderCode , NULL);
    glCompileShader(FragmentShaderID);

    // Check Fragment Shader
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0 ) {
        char fragmentShaderErrorMessage[InfoLogLength+1];
        glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, fragmentShaderErrorMessage);
        printf("%s\n", fragmentShaderErrorMessage);
    }

    // Link the program
    printf("Linking program\n");
    GLuint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);

    // Check the program
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0) {
        char programErrorMessage[InfoLogLength+1];
        glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, programErrorMessage);
        printf("%s\n", &programErrorMessage[0]);
    }

    glDetachShader(ProgramID, VertexShaderID);
    glDetachShader(ProgramID, FragmentShaderID);
    
    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);

    return ProgramID;
}