
#include "Shapes.h"
#include <array>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <unordered_set>
#include <limits>

#include <string>
#define GLM_ENABLE_EXPERIMENTAL
#include "gtx/string_cast.hpp"
#include <iostream>
#include <algorithm>
#include <cmath>


glm::vec3 CalculateCentroid(const std::vector<glm::vec3>& points) {
    glm::vec3 centroid(0.0f);
    for (const auto& point : points) {
        centroid += point;
    }
    return centroid / static_cast<float>(points.size());
}

// Function to calculate the angle between the point and the centroid
float AngleBetweenPoints(const glm::vec3& point, const glm::vec3& centroid) {
    glm::vec2 diff = glm::vec2(point.x - centroid.x, point.y - centroid.y);
    return std::atan2(diff.y, diff.x); // Use atan2 to get angle in 2D
}

// Sort the points by angle relative to the centroid
void SortPointsByAngle(std::vector<glm::vec3>& points) {
    glm::vec3 centroid = CalculateCentroid(points);

    // Sort the points based on their angle relative to the centroid
    std::sort(points.begin(), points.end(), [&centroid](const glm::vec3& a, const glm::vec3& b) {
        return AngleBetweenPoints(a, centroid) < AngleBetweenPoints(b, centroid);
        });
}

////////////////////////////////
void Shapes::ExtractUniquePositionsAndIndices(const Mesh& mesh, std::vector<glm::vec3>& outPositions, std::vector<unsigned int>& outIndices)
{
    std::unordered_map<glm::vec3, unsigned int, Vec3Hash, Vec3Equal> positionToIndex;
    outPositions.clear();
    outIndices.clear();

    for (size_t i = 0; i < mesh.indices.size(); ++i) {
        unsigned int originalIndex = mesh.indices[i];
        glm::vec3 position(
            mesh.vertices[originalIndex * 9 + 0],
            mesh.vertices[originalIndex * 9 + 1],
            mesh.vertices[originalIndex * 9 + 2]
        );

        if (positionToIndex.count(position) == 0) {
            // New unique position
            unsigned int newIndex = static_cast<unsigned int>(outPositions.size());
            outPositions.push_back(position);
            positionToIndex[position] = newIndex;
        }

        outIndices.push_back(positionToIndex[position]);
    }
}

void Shapes::ExtractUniquePositionsAndIndicesWorld(const Mesh& mesh, std::vector<glm::vec3>& outPositions, std::vector<unsigned int>& outIndices, const glm::mat4& model)
{
    std::unordered_map<glm::vec3, unsigned int, Vec3Hash, Vec3Equal> positionToIndex;
    outPositions.clear();
    outIndices.clear();

    for (size_t i = 0; i < mesh.indices.size(); ++i) {
        unsigned int originalIndex = mesh.indices[i];
        glm::vec3 position(
            mesh.vertices[originalIndex * 9 + 0],
            mesh.vertices[originalIndex * 9 + 1],
            mesh.vertices[originalIndex * 9 + 2]
        );

        if (positionToIndex.count(position) == 0) {
            // New unique position
            unsigned int newIndex = static_cast<unsigned int>(outPositions.size());
            position = glm::vec3(model * glm::vec4(position, 1.0f));
            outPositions.push_back(position);
            positionToIndex[position] = newIndex;
        }

        outIndices.push_back(positionToIndex[position]);
    }
}


Mesh Shapes::CreateSphere(float radius, unsigned int sectorCount, unsigned int stackCount, glm::vec3 color) {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    for (unsigned int i = 0; i <= stackCount; ++i) {
        float stackAngle = glm::pi<float>() / 2 - i * glm::pi<float>() / stackCount; // from pi/2 to -pi/2
        float xy = radius * cosf(stackAngle); // r * cos(phi)
        float z = radius * sinf(stackAngle);  // r * sin(phi)

        for (unsigned int j = 0; j <= sectorCount; ++j) {
            float sectorAngle = j * 2 * glm::pi<float>() / sectorCount; // from 0 to 2pi
            float x = xy * cosf(sectorAngle);
            float y = xy * sinf(sectorAngle);

            // Vertex position
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);

            // Normalized normal (for a sphere centered at origin, normal = position normalized)
            glm::vec3 normal = glm::normalize(glm::vec3(x, y, z));
            vertices.push_back(normal.x);
            vertices.push_back(normal.y);
            vertices.push_back(normal.z);

            // Vertex color
            vertices.push_back(color.r);
            vertices.push_back(color.g);
            vertices.push_back(color.b);
        }
    }

    // Indices
    for (unsigned int i = 0; i < stackCount; ++i) {
        unsigned int k1 = i * (sectorCount + 1);
        unsigned int k2 = k1 + sectorCount + 1;

        for (unsigned int j = 0; j < sectorCount; ++j, ++k1, ++k2) {
            if (i != 0) {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }

            if (i != (stackCount - 1)) {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }

    return OpenGLDataInitialize(vertices, indices);
}

Mesh Shapes::CreateBox(float width, float height, float length, glm::vec3 color) {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    float w = width / 2.0f;
    float h = height / 2.0f;
    float l = length / 2.0f;

    struct Face {
        glm::vec3 normal;
        glm::vec3 v0, v1, v2, v3;
    };

    std::vector<Face> faces = {
        // Front face
        {{0, 0, 1},  {-w, -h,  l},  { w, -h,  l},  { w,  h,  l},  {-w,  h,  l}},
        // Back face
        {{0, 0, -1}, {-w, -h, -l},  {-w,  h, -l},  { w,  h, -l},  { w, -h, -l}},
        // Left face
        {{-1, 0, 0}, {-w, -h, -l},  {-w, -h,  l},  {-w,  h,  l},  {-w,  h, -l}},
        // Right face
        {{1, 0, 0},  { w, -h, -l},  { w,  h, -l},  { w,  h,  l},  { w, -h,  l}},
        // Top face
        {{0, 1, 0},  {-w,  h, -l},  {-w,  h,  l},  { w,  h,  l},  { w,  h, -l}},
        // Bottom face
        {{0, -1, 0}, {-w, -h, -l},  { w, -h, -l},  { w, -h,  l},  {-w, -h,  l}},
    };

    unsigned int index = 0;
    for (const auto& face : faces) {
        std::array<unsigned int, 6> faceIndices = {
            index, index + 1, index + 2,
            index, index + 2, index + 3
        };

        for (int i = 0; i < 6; ++i)
            indices.push_back(faceIndices[i]);

        std::vector<glm::vec3> corners = { face.v0, face.v1, face.v2, face.v3 };
        for (const auto& v : corners) {
            // Position
            vertices.push_back(v.x);
            vertices.push_back(v.y);
            vertices.push_back(v.z);

            // Normal
            vertices.push_back(face.normal.x);
            vertices.push_back(face.normal.y);
            vertices.push_back(face.normal.z);

            // Color
            vertices.push_back(color.r);
            vertices.push_back(color.g);
            vertices.push_back(color.b);
        }

        index += 4;
    }

    return OpenGLDataInitialize(vertices, indices);
}

Mesh Shapes::CreateCylinder(float radius, float height, unsigned int sectorCount, glm::vec3 color) {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    float halfHeight = height / 2.0f;
    float sectorStep = 2 * glm::pi<float>() / sectorCount;

    // Side surface
    for (unsigned int i = 0; i <= sectorCount; ++i) {
        float angle = i * sectorStep;
        float x = cos(angle);
        float y = sin(angle);
        glm::vec3 normal = glm::normalize(glm::vec3(x, y, 0.0f));

        // Bottom vertex
        vertices.insert(vertices.end(), {
            radius * x, radius * y, -halfHeight,
            normal.x, normal.y, normal.z,
            color.r, color.g, color.b
            });

        // Top vertex
        vertices.insert(vertices.end(), {
            radius * x, radius * y, halfHeight,
            normal.x, normal.y, normal.z,
            color.r, color.g, color.b
            });
    }

    // Side indices (CCW winding from outside view)
    for (unsigned int i = 0; i < sectorCount; ++i) {
        unsigned int k1 = i * 2;
        unsigned int k2 = k1 + 2;

        indices.push_back(k1);
        indices.push_back(k2);
        indices.push_back(k1 + 1);

        indices.push_back(k1 + 1);
        indices.push_back(k2);
        indices.push_back(k2 + 1);
    }

    // Add center vertices for caps
    unsigned int baseIndex = static_cast<unsigned int>(vertices.size() / 9); // each vertex = 9 floats
    unsigned int bottomCenterIndex = baseIndex;
    unsigned int topCenterIndex = baseIndex + 1;

    glm::vec3 bottomNormal(0, 0, -1), topNormal(0, 0, 1);

    vertices.insert(vertices.end(), {
        0.0f, 0.0f, -halfHeight, bottomNormal.x, bottomNormal.y, bottomNormal.z, color.r, color.g, color.b,
        0.0f, 0.0f,  halfHeight, topNormal.x,    topNormal.y,    topNormal.z,    color.r, color.g, color.b
        });

    // Bottom + top caps
    for (unsigned int i = 0; i < sectorCount; ++i) {
        float angle = i * sectorStep;
        float nextAngle = (i + 1) * sectorStep;

        float x0 = cos(angle), y0 = sin(angle);
        float x1 = cos(nextAngle), y1 = sin(nextAngle);

        // Bottom triangle (CCW from bottom view)
        unsigned int i0 = static_cast<unsigned int>(vertices.size() / 9);
        vertices.insert(vertices.end(), {
            radius * x1, radius * y1, -halfHeight, bottomNormal.x, bottomNormal.y, bottomNormal.z, color.r, color.g, color.b,
            radius * x0, radius * y0, -halfHeight, bottomNormal.x, bottomNormal.y, bottomNormal.z, color.r, color.g, color.b
            });

        indices.insert(indices.end(), {
            bottomCenterIndex, i0, i0 + 1
            });

        // Top triangle (CCW from top view)
        unsigned int i1 = static_cast<unsigned int>(vertices.size() / 9);
        vertices.insert(vertices.end(), {
            radius * x0, radius * y0, halfHeight, topNormal.x, topNormal.y, topNormal.z, color.r, color.g, color.b,
            radius * x1, radius * y1, halfHeight, topNormal.x, topNormal.y, topNormal.z, color.r, color.g, color.b
            });

        indices.insert(indices.end(), {
            topCenterIndex, i1, i1 + 1
            });
    }


    return OpenGLDataInitialize(vertices, indices);
}

Mesh Shapes::OpenGLDataInitialize(std::vector<float>& vertices, std::vector<unsigned int>& indices)
{
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // OpenGL buffer setup
    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    // VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);

    // EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_DYNAMIC_DRAW);

    // Position: location = 0
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Normal: location = 1
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Color: location = 2
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0); // Unbind

    return { VAO, VBO, EBO, static_cast<GLsizei>(indices.size()),vertices,indices };
}

void Shapes::ProjectOntoAxis(
    const std::vector<float>& vertices,
    const glm::vec3& axis,
    const glm::mat4& modelMatrix,
    float& min,
    float& max
) {
    min = std::numeric_limits<float>::infinity();
    max = -std::numeric_limits<float>::infinity();
    for (int i = 0;i < vertices.size() / 9; i++) {
        glm::vec4 localPos(vertices[i * 9], vertices[i * 9 + 1], vertices[i * 9 + 2], 1.0f);
        glm::vec3 worldPos = glm::vec3(modelMatrix * localPos);
        float projection = glm::dot(worldPos, axis);

        min = std::min(min, projection);
        max = std::max(max, projection);
    }
}

bool Shapes::AreMeshesIntersectingSAT(
    const Mesh& meshA, const glm::mat4& modelA,
    const Mesh& meshB, const glm::mat4& modelB
) {
    const std::vector<glm::vec3>& faceNormalsA = CalculateFaceNormals(meshA, modelA);
    const std::vector<glm::vec3>& faceNormalsB = CalculateFaceNormals(meshB, modelB);

    std::vector<glm::vec3> axes = faceNormalsA;
    axes.insert(axes.end(), faceNormalsB.begin(), faceNormalsB.end());


    // Optional: Add edge cross-products if using polyhedra like cylinders and boxes

    for (const glm::vec3& axis : axes) {
        if (glm::length(axis) < 1e-6f) continue; // skip tiny vectors

        float minA, maxA, minB, maxB;
        ProjectOntoAxis(meshA.vertices,  axis, modelA, minA, maxA);
        ProjectOntoAxis(meshB.vertices,  axis, modelB, minB, maxB);

        if (maxA < minB || maxB < minA) {
            return false; // Separating axis found
        }
    }

    return true; // No separating axis => Intersection
}

std::vector<glm::vec3> Shapes::CalculateFaceNormals(const Mesh& mesh, const glm::mat4& modelMatrix) {
    std::vector<glm::vec3> normals;

    for (size_t i = 0; i < mesh.indices.size(); i += 3) {
        unsigned int idx0 = mesh.indices[i];
        unsigned int idx1 = mesh.indices[i + 1];
        unsigned int idx2 = mesh.indices[i + 2];

        glm::vec4 p0(mesh.vertices[idx0 * 9], mesh.vertices[idx0 * 9 + 1], mesh.vertices[idx0 * 9 + 2], 1.0f);
        glm::vec4 p1(mesh.vertices[idx1 * 9], mesh.vertices[idx1 * 9 + 1], mesh.vertices[idx1 * 9 + 2], 1.0f);
        glm::vec4 p2(mesh.vertices[idx2 * 9], mesh.vertices[idx2 * 9 + 1], mesh.vertices[idx2 * 9 + 2], 1.0f);

        glm::vec3 v0 = glm::vec3(modelMatrix * p0);
        glm::vec3 v1 = glm::vec3(modelMatrix * p1);
        glm::vec3 v2 = glm::vec3(modelMatrix * p2);

        glm::vec3 edge1 = v1 - v0;
        glm::vec3 edge2 = v2 - v0;
        glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));

        normals.push_back(normal);
    }

    return normals;
}

bool Shapes::IsPointInsideConvexMesh(const glm::vec3& point,
    const std::vector<glm::vec3>& vertexPositions,
    const std::vector<unsigned int>& indices)
{
    size_t triangleCount = indices.size() / 3;

    for (size_t i = 0; i < triangleCount; ++i) {
        unsigned int idx0 = indices[i * 3];
        unsigned int idx1 = indices[i * 3 + 1];
        unsigned int idx2 = indices[i * 3 + 2];

        // Get transformed vertices
        glm::vec3 v0 = vertexPositions[idx0];
        glm::vec3 v1 = vertexPositions[idx1];
        glm::vec3 v2 = vertexPositions[idx2];

        // Compute face normal
        glm::vec3 normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));

        // Check point against the face plane
        float dotProduct = glm::dot(normal, point - v0);

        if (dotProduct > 0.0f) {
            return false; // Point is outside this face
        }
    }

    return true; // Point is inside all face planes
}

std::vector<unsigned int> Shapes::GetConnectedVertices(const std::vector<unsigned int>& Indices,
    unsigned int vertexIndex) {
    std::unordered_set<unsigned int> connectedVertices;

    for (size_t i = 0; i < Indices.size(); i += 3) {
        unsigned int idx1 = Indices[i];
        unsigned int idx2 = Indices[i + 1];
        unsigned int idx3 = Indices[i + 2];

        if (idx1 == vertexIndex || idx2 == vertexIndex || idx3 == vertexIndex) {
            if (idx1 != vertexIndex) connectedVertices.insert(idx1);
            if (idx2 != vertexIndex) connectedVertices.insert(idx2);
            if (idx3 != vertexIndex) connectedVertices.insert(idx3);
        }
    }

    return std::vector<unsigned int>(connectedVertices.begin(), connectedVertices.end());
}

std::vector<unsigned int> Shapes::GetVertexesWithinMesh(const Mesh& meshA, const glm::mat4& modelMatrixA, const Mesh& meshB, const glm::mat4& modelMatrixB)
{
    std::vector<unsigned int> pointsWithin;
    std::vector<glm::vec3> vertexPositionA;
    std::vector<glm::vec3> vertexPositionB;
    std::vector<unsigned int> IndicesA;
    std::vector<unsigned int> IndicesB;

    ExtractUniquePositionsAndIndices(meshA, vertexPositionA, IndicesA);
    ExtractUniquePositionsAndIndices(meshB, vertexPositionB, IndicesB);

    //firstMeshPoints = true;
    for (int i = 0;i < vertexPositionA.size();i++) {
        glm::vec3 worldV0 = glm::vec3(modelMatrixA * glm::vec4(vertexPositionA[i], 1.0f));
        if (IsPointInsideConvexMesh(worldV0, vertexPositionB, IndicesB)) {
            pointsWithin.push_back(i);
        }
    }
    //if (pointsWithin.empty()) {
    //    //firstMeshPoints = false;
    //    for (int i = 0;i < vertexPositionB.size();i++) {
    //        glm::vec3 worldV0 = glm::vec3(modelMatrixB * glm::vec4(vertexPositionB[i], 1.0f));
    //        if (IsPointInsideConvexMesh(worldV0, vertexPositionA, IndicesA, modelMatrixA)) {
    //            pointsWithin.push_back(i);
    //        }
    //    }
    //}

    /*std::vector<unsigned int> edges;
    if (firstMeshPoints) {
        for (auto point : pointsWithin) {
            edges = GetConnectedVertices(IndicesA, point);
        }
    }
    else {
        for (auto point : pointsWithin) {
            edges = GetConnectedVertices(IndicesB, point);
        }
    }*/

    return pointsWithin;
}

std::vector<glm::vec3> Shapes::GetVertexesWithinMesh2(const std::vector<glm::vec3>& vertexPositionA,
    const std::vector<glm::vec3>& vertexPositionB,
    const std::vector<unsigned int>& IndicesA,
    const std::vector<unsigned int> IndicesB)
{
    std::vector<glm::vec3> pointsWithin;

    for (int i = 0;i < vertexPositionA.size();i++) {
        if (IsPointInsideConvexMesh(vertexPositionA[i], vertexPositionB, IndicesB)) {
            pointsWithin.push_back(vertexPositionA[i]);
        }
    }

    return pointsWithin;
}

std::vector<glm::vec3> Shapes::GetIntersectionPoints(const Mesh& meshA, const glm::mat4& modelMatrixA, const Mesh& meshB, const glm::mat4& modelMatrixB, bool firstMeshPoints)
{
    std::vector<glm::vec3> intersectionPoints;
    std::vector<unsigned int> pointsWithinB = GetVertexesWithinMesh(meshA, modelMatrixA, meshB, modelMatrixB);
    std::vector<unsigned int> pointsWithinA = GetVertexesWithinMesh(meshB, modelMatrixB, meshA, modelMatrixA);
    std::vector<glm::vec3> vertexPositionA;
    std::vector<glm::vec3> vertexPositionB;
    std::vector<unsigned int> IndicesA;
    std::vector<unsigned int> IndicesB;

    ExtractUniquePositionsAndIndices(meshA, vertexPositionA, IndicesA);
    ExtractUniquePositionsAndIndices(meshB, vertexPositionB, IndicesB);

    const float tolerance = 0.001f;  // Define a small tolerance for duplicate points
    std::vector<unsigned int> edges;
        for (auto point : pointsWithinB) {
            intersectionPoints.push_back(glm::vec3(modelMatrixA * glm::vec4(vertexPositionA[point], 1.0f)));
            edges = GetConnectedVertices(IndicesA, point);
            // For each edge, check for intersection with the other mesh
            for (auto edge : edges) {
                // Here you would need to check if the edge intersects with any faces of the other mesh
                glm::vec3 v0 = glm::vec3(modelMatrixA * glm::vec4(vertexPositionA[point], 1.0f));
                glm::vec3 v1 = glm::vec3(modelMatrixA * glm::vec4(vertexPositionA[edge], 1.0f));
                //This needs to be fixed later!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                glm::vec3 intersection = GetEdgeIntersection(v0, v1, vertexPositionB, IndicesB, modelMatrixB)[0];
                if (glm::length(intersection) > 0.0f) {
                    bool add = true;
                    for(auto pointI: intersectionPoints){
                        if (glm::length(intersection - pointI) < tolerance) {
                            add = false;
                            break;
                        }
                    }
                    if (add) {
                        intersectionPoints.push_back(intersection);
                    }
                }
                /*if (intersections == 2)
                    break;*/
            }
        }
        for (auto point : pointsWithinA) {
            intersectionPoints.push_back(glm::vec3(modelMatrixB * glm::vec4(vertexPositionB[point], 1.0f)));
            edges = GetConnectedVertices(IndicesB, point);
            for (auto edge : edges) {
                glm::vec3 v0 = glm::vec3(modelMatrixB * glm::vec4(vertexPositionB[point], 1.0f));
                glm::vec3 v1 = glm::vec3(modelMatrixB * glm::vec4(vertexPositionB[edge], 1.0f));
                //This needs to be fixed later!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                glm::vec3 intersection = GetEdgeIntersection(v0, v1, vertexPositionA, IndicesA, modelMatrixA)[0];
                if (glm::length(intersection) > 0.0f) {
                    bool add = true;
                    for (auto pointI : intersectionPoints) {
                        if (glm::length(intersection - pointI) < tolerance) {
                            add = false;
                            break;
                        }
                    }
                    if (add) {
                        intersectionPoints.push_back(intersection);
                    }
                }
                /*if (intersections == 2)
                    break;*/
            }
        }

    std::vector<glm::vec3> uniquePoints;

    for (const auto& point : intersectionPoints) {
        bool isDuplicate = false;
        for (const auto& uniquePoint : uniquePoints) {
            if (glm::length(point - uniquePoint) < tolerance) {
                isDuplicate = true;
                break;
            }
        }
        if (!isDuplicate) {
            uniquePoints.push_back(point);
        }
    }

    intersectionPoints = uniquePoints;  // Update points with unique ones
    for (auto& point : uniquePoints) {
        std::cout << glm::to_string(point) << "\n";
    }
    return intersectionPoints;
}


std::vector<glm::vec3> Shapes::GetEdgeIntersection(const glm::vec3& v0, const glm::vec3& v1, const std::vector<glm::vec3>& vertices, const std::vector<unsigned int>& indices, const glm::mat4& modelMatrix)
{
    std::vector<glm::vec3> intersections;  // To store the intersection points

    // Iterate over all triangles in the mesh
    for (size_t i = 0; i < indices.size() / 3; ++i) {
        unsigned int idx0 = indices[i * 3];
        unsigned int idx1 = indices[i * 3 + 1];
        unsigned int idx2 = indices[i * 3 + 2];

        glm::vec3 v2 = glm::vec3(modelMatrix * glm::vec4(vertices[idx0], 1.0f));
        glm::vec3 v3 = glm::vec3(modelMatrix * glm::vec4(vertices[idx1], 1.0f));
        glm::vec3 v4 = glm::vec3(modelMatrix * glm::vec4(vertices[idx2], 1.0f));

        // Check for intersection of the line segment [v0, v1] with the triangle [v2, v3, v4]
        glm::vec3 intersection;
        if (LineIntersectsTriangle(v0, v1, v2, v3, v4, intersection)) {
            intersections.push_back(intersection);  // Store the intersection point
            if (intersections.size() == 2)
                break;
        }
    }

    // Return the list of intersection points (may be empty if no intersections were found)
    return intersections;
}



std::vector<Face> Shapes::GeneratePolygonIntersectionFaces(const Mesh& meshA, const glm::mat4& modelMatrixA, const Mesh& meshB, const glm::mat4& modelMatrixB)
{
    std::vector<glm::vec3> vertexPositionA;
    std::vector<glm::vec3> vertexPositionB;
    std::vector<unsigned int> IndicesA;
    std::vector<unsigned int> IndicesB;
    const float tolerance = 0.001f;  // Define a small tolerance for duplicate points
    ExtractUniquePositionsAndIndicesWorld(meshA, vertexPositionA, IndicesA, modelMatrixA);
    ExtractUniquePositionsAndIndicesWorld(meshB, vertexPositionB, IndicesB, modelMatrixB);;
    std::vector<glm::vec3> pointsWithinB = GetVertexesWithinMesh2(vertexPositionA, vertexPositionB, IndicesA, IndicesB);
    std::vector<glm::vec3> pointsWithinA = GetVertexesWithinMesh2(vertexPositionB, vertexPositionA, IndicesB, IndicesA);

    std::vector<Face> faces;

    bool intersect;
    for (int i = 0;i < IndicesA.size();i += 3) {
        Face face;
        glm::vec3 v0 = vertexPositionA[IndicesA[i]];
        glm::vec3 v1 = vertexPositionA[IndicesA[i + 1]];
        glm::vec3 v2 = vertexPositionA[IndicesA[i + 2]];

        glm::vec3 normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));
        face.normal = normal; // You’ll need to add this to your Face struct
        for (int j = 0;j < IndicesB.size();j += 3) {
            glm::vec3 intersection;
            intersect = LineIntersectsTriangle(vertexPositionA[IndicesA[i]], vertexPositionA[IndicesA[i + 1]], vertexPositionB[IndicesB[j]], vertexPositionB[IndicesB[j + 1]], vertexPositionB[IndicesB[j + 2]], intersection);
            if (intersect)
                face.facePoints.push_back(intersection);
            intersect = LineIntersectsTriangle(vertexPositionA[IndicesA[i]], vertexPositionA[IndicesA[i + 2]], vertexPositionB[IndicesB[j]], vertexPositionB[IndicesB[j + 1]], vertexPositionB[IndicesB[j + 2]], intersection);
            if (intersect)
                face.facePoints.push_back(intersection);
            intersect = LineIntersectsTriangle(vertexPositionA[IndicesA[i + 1]], vertexPositionA[IndicesA[i + 2]], vertexPositionB[IndicesB[j]], vertexPositionB[IndicesB[j + 1]], vertexPositionB[IndicesB[j + 2]], intersection);
            if (intersect)
                face.facePoints.push_back(intersection);
        }

        for (auto& point : pointsWithinB) {
            if (IsPointInTriangle(point, vertexPositionA[IndicesA[i]], vertexPositionA[IndicesA[i + 1]], vertexPositionA[IndicesA[i+2]])) {
                face.facePoints.push_back(point);
            }
        }

        for (auto& point : pointsWithinA) {
            if (IsPointInTriangle(point, vertexPositionA[IndicesA[i]], vertexPositionA[IndicesA[i + 1]], vertexPositionA[IndicesA[i + 2]])) {
                face.facePoints.push_back(point);
            }
        }

        std::vector<glm::vec3> uniquePoints;

        for (const auto& point : face.facePoints) {
            bool isDuplicate = false;
            for (const auto& uniquePoint : uniquePoints) {
                if (glm::length(point - uniquePoint) < tolerance) {
                    isDuplicate = true;
                    break;
                }
            }
            if (!isDuplicate) {
                uniquePoints.push_back(point);
            }
        }

        SortPointsByAngle(uniquePoints);
        face.facePoints = uniquePoints;
        face.indeces = TriangulateConvexPolygon(face.facePoints, face.normal);

        if(face.facePoints.size()>0)
        faces.push_back(face);
    }

    std::vector<glm::vec3> uniquePoints;
    for (const auto& face :faces) {
        for (const auto& point : face.facePoints) {
            bool isDuplicate = false;
            for (const auto& uniquePoint : uniquePoints) {
                if (glm::length(point - uniquePoint) < tolerance) {
                    isDuplicate = true;
                    break;
                }
            }
            if (!isDuplicate) {
                uniquePoints.push_back(point);
            }
        }
    }
    for (auto& point : uniquePoints) {
        std::cout << glm::to_string(point)<<"\n";
    }
    std::cout  << "\n";
    return faces;
}

bool Shapes::LineIntersectsTriangle(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, glm::vec3& intersection)
{
    const glm::vec3 dir = p1 - p0;
    const glm::vec3 e1 = v1 - v0;
    const glm::vec3 e2 = v2 - v0;

    const glm::vec3 h = glm::cross(dir, e2);
    const float a = glm::dot(e1, h);

    // Parallel check
    if (fabs(a) < std::numeric_limits<float>::epsilon()) {
        return false;
    }

    const float f = 1.0f / a;
    const glm::vec3 s = p0 - v0;
    const float u = f * glm::dot(s, h);
    if (u < 0.0f || u > 1.0f) {
        return false;
    }

    const glm::vec3 q = glm::cross(s, e1);
    const float v = f * glm::dot(dir, q);
    if (v < 0.0f || u + v > 1.0f) {
        return false;
    }

    const float t = f * glm::dot(e2, q);
    if (t < 0.0f || t > 1.0f) {
        return false; // Outside segment bounds
    }

    intersection = p0 + dir * t;
    return true;
}

bool Shapes::IsPointInTriangle(const glm::vec3& point, const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, float epsilon)
{
    // Compute vectors
    glm::vec3 v0v1 = v1 - v0;
    glm::vec3 v0v2 = v2 - v0;
    glm::vec3 v0p = point - v0;

    // Compute dot products
    float d00 = glm::dot(v0v1, v0v1);
    float d01 = glm::dot(v0v1, v0v2);
    float d11 = glm::dot(v0v2, v0v2);
    float d20 = glm::dot(v0p, v0v1);
    float d21 = glm::dot(v0p, v0v2);

    // Compute barycentric coordinates
    float denom = d00 * d11 - d01 * d01;
    if (fabs(denom) < epsilon)
        return false; // Degenerate triangle

    float v = (d11 * d20 - d01 * d21) / denom;
    float w = (d00 * d21 - d01 * d20) / denom;
    float u = 1.0f - v - w;

    // Check if point is inside or on triangle
    return (u >= -epsilon && v >= -epsilon && w >= -epsilon &&
        u <= 1.0f + epsilon && v <= 1.0f + epsilon && w <= 1.0f + epsilon);
}

std::vector<unsigned int> TriangulateConvexPolygon(const std::vector<glm::vec3>& polygonVertices, const glm::vec3& normal) {
    std::vector<unsigned int> triangleIndices;

    unsigned int n = polygonVertices.size();
    if (n < 3) return triangleIndices;

    glm::vec3 v0 = polygonVertices[0];
    glm::vec3 v1 = polygonVertices[1];
    glm::vec3 v2 = polygonVertices[2];
    glm::vec3 polygonNormal = glm::normalize(glm::cross(v1 - v0, v2 - v0));

    unsigned int anchorIndex = 0;

    for (unsigned int i = 1; i < n - 1; ++i) {
        if (glm::dot(polygonNormal, normal) < 0) {
            triangleIndices.push_back(anchorIndex);
            triangleIndices.push_back(i + 1);       
            triangleIndices.push_back(i);
        }
        else {

            triangleIndices.push_back(anchorIndex);  
            triangleIndices.push_back(i);           
            triangleIndices.push_back(i + 1);
        }
    }

    return triangleIndices;
}