#include "cylinder.h"

Cylinder::Cylinder(int complexity)
    : Shape(complexity)
{}


Cylinder::~Cylinder() {}


void Cylinder::calcVerts()
{
    // 3 verts per triangle * slices * (horizSquares * 2 +
    // (top and bottom squares * 2 - 1) * 2) * 2 for normals
    m_numVerts = ((m_p2 * ((m_p1 * 2 * 3) + 6)) - 2);
    int size = m_numVerts * 8;
    m_vertexData = new GLfloat[size];

    // for rotation
    float angleSpacing = 2.f * M_PI / m_p2;
    float cosine = cos(angleSpacing);
    float sine = sin(angleSpacing);

    float x = m_halfWidth;
    float z = 0;
    float temp;

    glm::vec2 prev = glm::vec2(x, z);
    glm::vec2 curr = glm::vec2(0, 0);
    float prevU = 1.f;
    float currU;

    // rotation matrix
    temp = x;
    x = cosine * x - sine * z;
    z = sine * temp + cosine * z;

    int index = 0;

    // iterate through the slices
    for (int i = 0; i < m_p2; i++) {
        curr[0] = x;
        curr[1] = z;

        currU = 1.f - ((i + 1.f) / m_p2);

        make3DSlice(&index, curr, prev, currU, prevU);

        // repeat the last point of this slice and the first point of the next
        // slice so the renderer won't connect the two points
        if (i != m_p2 - 1) {
            addVertex(&index, glm::vec3(0, -m_halfWidth, 0), glm::vec3(0, -1, 0));
            addVertex(&index, glm::vec3(0, m_halfWidth, 0), glm::vec3(0, 1, 0));
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



void Cylinder::make3DSlice(int *index, glm::vec2 left, glm::vec2 right, float leftU, float rightU)
{
    // starting point
    glm::vec2 tex = glm::vec2(0.5f, 0.5f);
    addVertexT(index, glm::vec3(0, m_halfWidth, 0), glm::vec3(0, 1, 0), tex);

    // top sliver
    makeTopSlice(index, glm::vec3(left.x, m_halfWidth, left.y), glm::vec3(right.x, m_halfWidth, right.y));

    // make wall
    glm::vec3 vl = glm::vec3(left.x, 0, left.y);
    glm::vec3 vr = glm::vec3(right.x, 0, right.y);

    glm::vec3 nl = glm::normalize(vl);
    glm::vec3 nr = glm::normalize(vr);

    glm::vec2 texl = glm::vec2(leftU, 0);
    glm::vec2 texr = glm::vec2(rightU, 0);

    for (int i = m_p1; i >= 0; i--) {
        float y = (i * m_halfWidth * 2.f / m_p1) - m_halfWidth;
        vl.y = y;
        vr.y = y;

        texl.y = 1.f - (i * 1.f / m_p1);
        texr.y = 1.f - (i * 1.f / m_p1);

        addVertexT(index, vl, nl, texl);
        addVertexT(index, vr, nr, texr);
    }

    // bottom sliver
    makeBottomSlice(index, glm::vec3(left.x, -m_halfWidth, left.y), glm::vec3(right.x, -m_halfWidth, right.y));

    // ending point
    tex = glm::vec2(0.5f, 0.5f);
    addVertexT(index, glm::vec3(0, -m_halfWidth, 0), glm::vec3(0, -1, 0), tex);
}


void Cylinder::makeTopSlice(int *index, glm::vec3 left, glm::vec3 right)
{
    glm::vec3 n = glm::vec3(0, 1, 0);
    glm::vec3 vl = glm::vec3(0, left.y, 0);
    glm::vec3 vr = glm::vec3(0, right.y, 0);

    float scale;

    // make top slice from center outward
    for (int i = 1; i <= m_p1; i++) {
        scale = i * m_halfWidth * 2.f / m_p1;
        vl.x = left.x * scale;
        vl.z = left.z * scale;
        vr.x = right.x * scale;
        vr.z = right.z * scale;

        addVertexT(index, vl, n, glm::vec2(vl.x + 0.5f, vl.z + 0.5));
        addVertexT(index, vr, n, glm::vec2(vr.x + 0.5f, vr.z + 0.5));
    }
}


void Cylinder::makeBottomSlice(int *index, glm::vec3 left, glm::vec3 right)
{
    glm::vec3 n = glm::vec3(0, -1, 0);
    glm::vec3 vl = glm::vec3(0, left.y, 0);
    glm::vec3 vr = glm::vec3(0, right.y, 0);

    float scale;

    // make bottom slice moving inward toward the center
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


