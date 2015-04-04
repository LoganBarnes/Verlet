#include "sphere.h"

Sphere::Sphere(int complexity)
    : Shape(complexity)
{
}


Sphere::~Sphere()
{
}


void Sphere::calcVerts()
{
    // ((slices * verts per slice) - first and last vert) * 2 for normals
    m_numVerts = ((m_p2 * (m_p1 + 1) * 2) - 2);
    int size = m_numVerts * 8; // 3 points per vertex
    m_vertexData = new GLfloat[size];

    float prev = 0;
    float curr;

    int index = 0;

    // iterate through the slices
    for (int i = 1; i <= m_p2; i++) {
        curr = i * M_PI * 2.f / m_p2;

        make3Dslice(&index, curr, prev);

        // repeat the last point of this slice and the first point of the next
        // slice so the renderer won't connect the two points
        if (i != m_p2) {
            addVertex(&index, glm::vec3(0, -m_halfWidth, 0), glm::vec3(0, -1, 0));
            addVertex(&index, glm::vec3(0, m_halfWidth, 0), glm::vec3(0, 1, 0));
        }

        prev = curr;
    }
}


void Sphere::make3Dslice(int *index, float thetaL, float thetaR)
{
    double spacing = M_PI / m_p1;
    float phi;

    // top point
    glm::vec2 tex = glm::vec2(1.f - (thetaR / (2 * M_PI)), 0.f);
    addVertexT(index, glm::vec3(0, m_halfWidth, 0), glm::vec3(0, 1, 0), tex);

    // iterate through sub blocks of slice
    for (int i = 1; i < m_p1; i++) {
        phi = i * spacing;
        calcSliceSeg(index, thetaL, thetaR, phi);
    }
    // bottom point
    tex.y = 1.f;
    addVertexT(index, glm::vec3(0, -m_halfWidth, 0), glm::vec3(0, -1, 0), tex);
}

void Sphere::calcSliceSeg(int *index, float thetaL, float thetaR, float phi)
{
    // parametric sphere equations
    glm::vec3 vl = glm::vec3(m_halfWidth * sin(phi) * cos(thetaL),
                        m_halfWidth * cos(phi),
                        m_halfWidth * sin(phi) * sin(thetaL));
    glm::vec3 vr = glm::vec3(m_halfWidth * sin(phi) * cos(thetaR),
                        m_halfWidth * cos(phi),
                        m_halfWidth * sin(phi) * sin(thetaR));

    glm::vec2 texl = glm::vec2(1.f - thetaL / (2 * M_PI), phi / M_PI);
    glm::vec2 texr = glm::vec2(1.f - thetaR / (2 * M_PI), phi / M_PI);

    addVertexT(index, vl, glm::normalize(vl), texl);
    addVertexT(index, vr, glm::normalize(vr), texr);
}


