#pragma once
#include "glad/glad.h"
#include "glm.hpp"
#include "gtc/epsilon.hpp"
#include <vector>
#include <unordered_map>

struct Mesh {
    GLuint VAO;
    GLuint VBO;
    GLuint EBO;
    GLsizei indexCount;
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
};

struct Vec3Hash {
    size_t operator()(const glm::vec3& v) const {
        size_t hx = std::hash<float>()(v.x);
        size_t hy = std::hash<float>()(v.y);
        size_t hz = std::hash<float>()(v.z);
        return hx ^ (hy << 1) ^ (hz << 2);
    }
};

struct Vec3Equal {
    bool operator()(const glm::vec3& a, const glm::vec3& b) const {
        return glm::all(glm::epsilonEqual(a, b, 1e-6f));
    }
};

class Shapes
{
public:
    static void ExtractUniquePositionsAndIndices(
        const Mesh& mesh,
        std::vector<glm::vec3>& outPositions,
        std::vector<unsigned int>& outIndices
    );
    static void ExtractUniquePositionsAndIndicesWorld(const Mesh& mesh, std::vector<glm::vec3>& outPositions, std::vector<unsigned int>& outIndices, const glm::mat4& model);
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
    static bool IsPointInsideConvexMesh(const glm::vec3& point,
        const std::vector<glm::vec3>& vertexPositions,
        const std::vector<unsigned int>& indices);
    static std::vector<unsigned int> GetConnectedVertices(
        const std::vector<unsigned int>& Indices,
        unsigned int vertexIndex);
    static std::vector<unsigned int> GetVertexesWithinMesh(const Mesh& meshA, const glm::mat4& modelMatrixA, const Mesh& meshB, const glm::mat4& modelMatrixB);
    static std::vector<glm::vec3> GetVertexesWithinMesh2(const std::vector<glm::vec3>& vertexPositionA,
    const std::vector<glm::vec3>& vertexPositionB,
    const std::vector<unsigned int>& IndicesA,
    const std::vector<unsigned int> IndicesB);
    static std::vector<glm::vec3> GetIntersectionPoints(const Mesh& meshA, const glm::mat4& modelMatrixA, const Mesh& meshB, const glm::mat4& modelMatrixB, bool firstMeshPoints);
    static bool LineIntersectsTriangle(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, glm::vec3& intersection);
    static std::vector<glm::vec3> GetEdgeIntersection(const glm::vec3& v0, const glm::vec3& v1, const std::vector<glm::vec3>& vertices, const std::vector<unsigned int>& indices, const glm::mat4& modelMatrix);
    static std::vector<glm::vec3> GetMeshTriangleIntersection(const Mesh& meshA, const glm::mat4& modelMatrixA, const Mesh& meshB, const glm::mat4& modelMatrixB);
    static bool IsPointInTriangle(const glm::vec3& point, const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, float epsilon = 0.0001f);
};

