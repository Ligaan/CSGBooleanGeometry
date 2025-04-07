#pragma once
#include "glad/glad.h"
#include "glm.hpp"
#include <vector>

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
    static Mesh CreateBox(float width, float height, float length, glm::vec3 color);
    static Mesh CreateCylinder(float radius, float height, unsigned int sectorCount, glm::vec3 color);
    static Mesh OpenGLDataInitialize(std::vector<float>& vertices, std::vector<unsigned int>& indices);
};

