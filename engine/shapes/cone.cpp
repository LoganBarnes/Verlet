#include "cone.h"

Cone::Cone(int complexity)
    : Shape(complexity)
{
}


Cone::~Cone()
{
}


void Cone::calcVerts()
{
    m_numVerts = ((m_p2 * ((m_p1 + 1) * 4)) - 2);
    int size = m_numVerts * 8;
    m_vertexData = new GLfloat[size];

    // calc slope of normals
    float normSlope = m_halfWidth / (m_halfWidth * 2.f);

    // for rotations
    float angleSpacing = 2.f * M_PI / m_p2;
    float cosine = cos(angleSpacing);
    float sine = sin(angleSpacing);

    float x = m_halfWidth;
    float z = 0;
    float temp;

    glm::vec2 prev = glm::vec2(x, z);
    glm::vec2 curr = glm::vec2(0, 0);

    float prevU = 1;
    float currU;

    // rotation matrix
    temp = x;
    x = cosine * x - sine * z;
    z = sine * temp + cosine * z;

    int index = 0;

    // iterate through each slice
    for (int i = 0; i < m_p2; i++) {
        curr[0] = x;
        curr[1] = z;

        currU = 1.f - ((i + 1.f) / m_p2);

        make3DSlice(&index, curr, prev, normSlope, currU, prevU);

        // repeat the last point of this slice and the first point of the next
        // slice so the renderer won't connect the two points
        if (i != m_p2 - 1) {
            glm::vec3 nl = glm::normalize(glm::vec3(curr.x, m_halfWidth * normSlope, curr.y));
            glm::vec3 nr = glm::normalize(glm::vec3(prev.x, m_halfWidth * normSlope, prev.y));
            addVertex(&index, glm::vec3(0, -m_halfWidth, 0), glm::vec3(0, -1, 0));
            addVertex(&index, glm::vec3(0, m_halfWidth, 0), glm::normalize(nl + nr));
        }

        // rotation matrix
        temp = x;
        x = cosine * x - sine * z;
        z = sine * temp + cosine * z;

        prev[0] = curr[0];
        prev[1] = curr[1];

        prevU = currU;
    }
}



void Cone::make3DSlice(int *index, glm::vec2 left, glm::vec2 right, float normSlope, float leftU, float rightU)
{
    // get the slice angles and normals
    glm::vec3 spineL = glm::vec3(left.x, m_halfWidth * -2.f, left.y) * (1.f / m_p1);
    glm::vec3 spineR = glm::vec3(right.x, m_halfWidth * -2.f, right.y) * (1.f / m_p1);

    glm::vec3 nl = glm::normalize(glm::vec3(left.x, m_halfWidth * normSlope, left.y));
    glm::vec3 nr = glm::normalize(glm::vec3(right.x, m_halfWidth * normSlope, right.y));

    glm::vec2 texl = glm::vec2(leftU, 0);
    glm::vec2 texr = glm::vec2(rightU, 0);

    // point of cone (starting point)
    glm::vec3 point = glm::vec3(0, m_halfWidth, 0);
    addVertexT(index, point, glm::normalize(nl + nr), texr);

    // build the wall
    for (int i = 1; i <= m_p1; i++) {

        texl.y = i * 1.f / m_p1;
        texr.y = i * 1.f / m_p1;

        addVertexT(index, point + spineL * (1.f * i), nl, texl);
        addVertexT(index, point + spineR * (1.f * i), nr, texr);
    }

    // make the bottom circle slice
    makeBottomSlice(index, glm::vec3(left.x, -m_halfWidth, left.y), glm::vec3(right.x, -m_halfWidth, right.y));

    // ending point
    addVertexT(index, glm::vec3(0, -m_halfWidth, 0), glm::vec3(0, -1, 0), glm::vec2(0.5f, 0.5f));


}


void Cone::makeBottomSlice(int *index, glm::vec3 left, glm::vec3 right)
{
    glm::vec3 n = glm::vec3(0, -1, 0);
    glm::vec3 vl = glm::vec3(0, left.y, 0);
    glm::vec3 vr = glm::vec3(0, right.y, 0);

    float scale;

    // build the slice from the outside in
    for (int i = m_p1; i > 0; i--) {
        scale = i * m_halfWidth * 2.f / m_p1;
        vl.x = left.x * scale;
        vl.z = left.z * scale;
        vr.x = right.x * scale;
        vr.z = right.z * scale;

        addVertexT(index, vl, n, glm::vec2(vl.x + 0.5f, 0.5f - vl.z));
        addVertexT(index, vr, n, glm::vec2(vr.x + 0.5f, 0.5f - vr.z));
    }
}

