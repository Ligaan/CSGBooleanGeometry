#pragma once
#include "glad/glad.h"
#include "glm.hpp"
#include <vector>

struct Mesh {
    GLuint VAO;
    GLuint VBO;
    GLuint EBO;
    GLsizei indexCount;
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
};

class Shapes
{
public:
    static Mesh CreateSphere(float radius, unsigned int sectorCount, unsigned int stackCount, glm::vec3 color);
    static Mesh CreateBox(float width, float height, float length, glm::vec3 color);
    static Mesh CreateCylinder(float radius, float height, unsigned int sectorCount, glm::vec3 color);
    static Mesh OpenGLDataInitialize(std::vector<float>& vertices, std::vector<unsigned int>& indices);
    static void ProjectOntoAxis(
        const std::vector<float>& vertices,
        const glm::vec3& axis,
        const glm::mat4& modelMatrix,
        float& min,
        float& max
    );
    static bool AreMeshesIntersectingSAT(
        const Mesh& meshA, const glm::mat4& modelA,
        const Mesh& meshB, const glm::mat4& modelB
    );
    static std::vector<glm::vec3> CalculateFaceNormals(const Mesh& mesh, const glm::mat4& modelMatrix);
    static bool IsPointInsideConvexMesh(const glm::vec3& point, const const Mesh& mesh, const glm::mat4 model);
    static std::vector<unsigned int> GetConnectedVertices(const Mesh& mesh, unsigned int vertexIndex);
    static std::vector<unsigned int> GetVertexesWithinMesh(const Mesh& meshA, const glm::mat4& modelMatrixA, const Mesh& meshB, const glm::mat4& modelMatrixB, bool& firstMeshPoints);
};

