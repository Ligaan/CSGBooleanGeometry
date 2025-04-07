#pragma once
#include "glad/glad.h"
#include "glm.hpp"

struct Mesh {
    GLuint VAO;
    GLuint VBO;
    GLuint EBO;
    GLsizei indexCount;
};

class Shapes
{
public:
    static Mesh CreateSphere(float radius, unsigned int sectorCount, unsigned int stackCount, glm::vec3 color);
};

