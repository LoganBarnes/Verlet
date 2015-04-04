#include "cube.h"

Cube::Cube(int complexity) : Shape(complexity)
{
}


Cube::~Cube()
{
}


void Cube::calcVerts()
{
    // 6 sides * 2w verts * p1 strips * 2 for normals
    int w = m_p1 + 1;
    m_numVerts = (6 * ((2 * w + 2) * m_p1) - 2);
    int size = m_numVerts * 8;
    m_vertexData = new GLfloat[size];

    double spacing = m_halfWidth * 2.0 / m_p1;

    int index = 0;
    // draw each of the six sides
    makeSide(&index, glm::vec3(0, 0, 1), spacing, true, false);
    makeSide(&index, glm::vec3(0, 1, 0), spacing, false, false);
    makeSide(&index, glm::vec3(1, 0, 0), spacing, false, false);
    makeSide(&index, glm::vec3(0, -1, 0), spacing, false, false);
    makeSide(&index, glm::vec3(-1, 0, 0), spacing, false, false);
    makeSide(&index, glm::vec3(0, 0, -1), spacing, false, true);
}


void Cube::makeSide(int *index, glm::vec3 norm, double spacing, bool first, bool last)
{
    int tbi; // top and bottom index
    int rli; // right and left index
    int oi;  // other index
    int n;   // norm

    glm::vec2 omin = glm::vec2(0, 0);
    glm::vec2 omax = glm::vec2(m_p1, m_p1);
    glm::vec2 nmin, nmax;

    if (norm.x) {
        tbi = 2;
        rli = 1;
        oi = 0;
        n = norm.x;
        nmin = glm::vec2(1.f, 1.f);
        nmax = glm::vec2(0.f, 0.f);
    }
    else if (norm.y) {
        tbi = 0;
        rli = 2;
        oi = 1;
        n = norm.y;
        if (n > 0) {
            nmin = glm::vec2(0.f, 0.f);
            nmax = glm::vec2(1.f, 1.f);
        } else {
            nmin = glm::vec2(1.f, 1.f);
            nmax = glm::vec2(0.f, 0.f);
        }
    }
    else { // (norm.z)
        tbi = 0;
        rli = 1;
        oi = 2;
        n = -norm.z;
        nmin = glm::vec2(1.f, 1.f);
        nmax = glm::vec2(0.f, 0.f);
    }


    glm::vec3 v1, v2;
    glm::vec2 t1, t2; // texture coords

    // set corner point
    v1[rli] = -m_halfWidth;
    v1[tbi] = -m_halfWidth;
    v1[oi] = norm[oi] * m_halfWidth;

    for (int i = 0; i < m_p1; i++) {

        v1[rli] = (i * spacing - m_halfWidth);
        v1[tbi] = (-m_halfWidth) * n;

        // double the first point if it isn't the start of the cube
        if (i != 0 || !first)
            addVertex(index, v1, norm);

        for (int j = 0; j <= m_p1; j++) {
            v1[tbi] = (j * spacing - m_halfWidth) * n;
            v2 = v1;
            v2[rli] = v1[rli] + spacing;

            t1 = mapPoints(glm::vec2(j, i), omin, omax, nmin, nmax);
            t2 = mapPoints(glm::vec2(j, i+1), omin, omax, nmin, nmax);

            addVertexT(index, v1, norm, t1);
            addVertexT(index, v2, norm, t2);
        }
        // double the last point if it isn't the end of the cube
        if (i != m_p1 - 1 || !last)
            addVertex(index, v2, norm);
    }
}


