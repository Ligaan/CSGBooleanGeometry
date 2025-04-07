#include "Shapes.h"
#include <array>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

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

    return { VAO, VBO, EBO, static_cast<GLsizei>(indices.size()) };
}
