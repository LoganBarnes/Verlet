#include "navmeshhandler.h"
#include "graphics.h"
#include "obj.h"
#include <QSet>

//#include <iostream>
//using namespace std;
//#include <glm/ext.hpp>

#define GLM_FORCE_RADIANS
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/vector_angle.hpp>

NavMeshHandler::NavMeshHandler()
{
    m_obj = NULL;
    m_mesh.clear();

    m_shader = 0;
    m_vaoID = 0;
    m_vboID = 0;

    m_start = -1;
    m_end = -1;

    m_graph.clear();
}


NavMeshHandler::~NavMeshHandler()
{
    foreach (int *edges, m_graph) {
        delete[] edges;
    }
}


void NavMeshHandler::setObject(OBJ *obj)
{
    if (m_obj)
        delete m_obj;
    m_obj = obj;
    if (m_obj)
    {
        m_shader = obj->getShader();
        buildGraph();
    }
}

bool NavMeshHandler::hasObject()
{
    return m_obj != NULL;
}

QList<Triangle*> *NavMeshHandler::getTriangleList()
{
    return &m_mesh;
}

void NavMeshHandler::draw(glm::mat4 trans)
{
    m_obj->draw(trans);
}

void NavMeshHandler::drawLines(glm::mat4 trans)
{
    glBindVertexArray(m_vaoID);
    glUniformMatrix4fv(glGetUniformLocation(m_shader, "model"), 1, GL_FALSE, glm::value_ptr(trans));
    glDrawArrays(GL_LINES, 0, m_numVerts);
    glBindVertexArray(0);
}

void NavMeshHandler::drawPath(glm::mat4 trans, Graphics *g, glm::vec4 color)
{
    // lines
    int size = m_pathActual.size();
    glm::vec3 prev = m_pathActual.value(0) + glm::vec3(0, 1, 0);
    glm::vec3 curr;
    g->setAllWhite(true);
    for (int i = 1; i < size; i++)
    {
        curr = m_pathActual.value(i) + glm::vec3(0, 1, 0);
        g->drawLineSeg(prev, curr, .1f);
        prev = curr;
    }
    g->setAllWhite(false);

    // triangles
    g->setColor(color.r, color.g, color.b, color.a, 0);
    foreach (PathNode n, m_pathNodes)
        m_obj->drawTriangle(n.id, trans);
}

void NavMeshHandler::drawStart(glm::mat4 trans)
{
    if (m_start > -1)
        m_obj->drawTriangle(m_start, trans);
}

void NavMeshHandler::drawEnd(glm::mat4 trans)
{
    if (m_end > -1)
        m_obj->drawTriangle(m_end, trans);
}

void NavMeshHandler::createVBO()
{
    // delete old array and buffer
    if (m_vaoID)
        glDeleteVertexArrays(1, &m_vaoID);
    if (m_vboID)
        glDeleteBuffers(1, &m_vboID);

    // build new data array
    m_numVerts = m_mesh.size() * 6;
    int size = m_numVerts * 3;
    GLfloat vertexData[size];

    int index = 0;
    foreach (Triangle *t, m_mesh)
    {
        fillVertex(&index, vertexData, t->vertices[0]);
        fillVertex(&index, vertexData, t->vertices[1]);
        fillVertex(&index, vertexData, t->vertices[1]);
        fillVertex(&index, vertexData, t->vertices[2]);
        fillVertex(&index, vertexData, t->vertices[2]);
        fillVertex(&index, vertexData, t->vertices[0]);
    }

    // Initialize the vertex array object.
    glGenVertexArrays(1, &m_vaoID);
    glBindVertexArray(m_vaoID);

    // Initialize the vertex buffer object.
    glGenBuffers(1, &m_vboID);
    glBindBuffer(GL_ARRAY_BUFFER, m_vboID);

    glBufferData(GL_ARRAY_BUFFER, 3 * m_numVerts * sizeof(GLfloat), vertexData, GL_STATIC_DRAW);

    GLuint position = glGetAttribLocation(m_obj->getShader(), "position");

    glEnableVertexAttribArray(position);
    glVertexAttribPointer(
        position,
        3,                   // Num coordinates per position
        GL_FLOAT,            // Type
        GL_FALSE,            // Normalized
        sizeof(GLfloat) * 3, // Stride
        (void*) 0            // Array buffer offset
    );

    // Unbind buffers.
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void NavMeshHandler::fillVertex(int *index, GLfloat *data, glm::vec3 v)
{
    data[(*index)++] = v.x;
    data[(*index)++] = v.y;
    data[(*index)++] = v.z;
}


void NavMeshHandler::setStart(glm::vec3 pos)
{
    m_start = findTriangle(pos);
    if (m_start <= -1)
    {
        m_pathNodes.clear();
        m_pathActual.clear();
    }
    else
        m_startPos = pos;
}


void NavMeshHandler::setEnd(glm::vec3 pos)
{
    m_end = findTriangle(pos);
    if (m_end <= -1)
    {
        m_pathNodes.clear();
        m_pathActual.clear();
    }
    else
        m_endPos = pos;
}

int NavMeshHandler::findTriangle(glm::vec3 pos)
{
    int index = -1;
    int size = m_mesh.size();
    Triangle *t;
    float bestDist = INFINITY;
    float dist;
    for (int i = 0; i < size; i++)
    {
        t = m_mesh.value(i);
        if (t->containsXZ(pos))
        {
            dist = pos.y - t->getHeight(pos);
            if (dist > 0 && dist < bestDist)
            {
                bestDist = dist;
                index = i;
            }
        }
    }
    return index;
}


QList<glm::vec3> NavMeshHandler::getPath()
{
    return m_pathActual;
}


void NavMeshHandler::findPath()
{
    if (m_start <= -1 || m_end <= -1)
        return;

    m_pathNodes.clear();
    m_pathActual.clear();

    if (m_start == m_end)
    {
        m_pathActual.append(m_startPos + glm::vec3(0, -1, 0));
        m_pathActual.append(m_endPos + glm::vec3(0, -1, 0));
        return;
    }

    const QList<OBJ::Tri> &tris = m_obj->triangles;

    QList<int> queue;
    QSet<int> visited;
    QHash<int,PathNode> prev;

    queue.append(m_start);
    visited.insert(m_start);
    prev[m_start] = PathNode();

    int curr, neighbor;
    while (!queue.isEmpty())
    {
        curr = queue.takeFirst();
        if (curr == m_end)
            break;

        const OBJ::Tri &tri = tris.value(curr);
        neighbor = m_g.value(QPair<int,int>(tri.b.vertex, tri.a.vertex), -1);
        if (!visited.contains(neighbor))
        {
            queue.append(neighbor);
            visited.insert(neighbor);
            prev[neighbor] = PathNode(curr, neighbor, tri.a.vertex, tri.b.vertex);
        }
        neighbor = m_g.value(QPair<int,int>(tri.c.vertex, tri.b.vertex), -1);
        if (!visited.contains(neighbor))
        {
            queue.append(neighbor);
            visited.insert(neighbor);
            prev[neighbor] = PathNode(curr, neighbor, tri.b.vertex, tri.c.vertex);
        }
        neighbor = m_g.value(QPair<int,int>(tri.a.vertex, tri.c.vertex), -1);
        if (!visited.contains(neighbor))
        {
            queue.append(neighbor);
            visited.insert(neighbor);
            prev[neighbor] = PathNode(curr, neighbor, tri.c.vertex, tri.a.vertex);
        }
    }

    if (curr != m_end)
        return;

    PathNode node = PathNode(curr);
    do
    {
        m_pathNodes.prepend(node);
        node = prev[node.id];
    }
    while (node.id != -1);

    // simple stupid funnel
    simpleStupidFunnel();
}


void NavMeshHandler::buildGraph()
{
    const QList<OBJ::Tri> &tris = m_obj->triangles;

    int size = m_mesh.size();
    for (int i = 0; i < size; i++)
    {
        const OBJ::Tri &tri = tris.value(i);
        m_g[QPair<int,int>(tri.a.vertex, tri.b.vertex)] = i;
        m_g[QPair<int,int>(tri.b.vertex, tri.c.vertex)] = i;
        m_g[QPair<int,int>(tri.c.vertex, tri.a.vertex)] = i;
    }
}

static void setVars(glm::vec3 *apex, int i, PathNode *gate, int *leftI, int *rightI, glm::vec3 *left,
                    glm::vec3 *right,  glm::vec3 *vecL, glm::vec3 *vecR, const QList<glm::vec3> *verts)
{
    *leftI = i;
    *rightI = i;
    *left = (*verts)[(*gate).left];
    *right = (*verts)[(*gate).right];

//    glm::vec3 temp = (*left)*.99f + (*right)*.001f;
//    *right = (*right)*.99f + (*left)*.001f;
//    *left = temp;

    *vecL = glm::normalize((*left) - (*apex));
    *vecR = glm::normalize((*right) - (*apex));
}


void NavMeshHandler::simpleStupidFunnel()
{
    const QList<glm::vec3> &verts = m_obj->vertices;

    m_pathActual.append(m_startPos + glm::vec3(0, -1, 0));
    glm::vec3 apex = m_startPos + glm::vec3(0, -1, 0);
    PathNode gate = m_pathNodes[0];

    int leftI, rightI, nextI;
    glm::vec3 left, right, next;
    glm::vec3 vecL, vecR, nextVec;

    setVars(&apex, 0, &gate, &leftI, &rightI, &left, &right, &vecL, &vecR, &verts);

    glm::vec3 nextCross;
    float nextAngle;
    glm::vec3 cross = glm::cross(vecR, vecL);
    float angle = glm::orientedAngle(vecR, vecL, glm::normalize(cross));

    PathNode prevGate = gate;

    int size = m_pathNodes.size();
    for (int i = 1; i < size; i++)
    {
        gate = m_pathNodes[i];
        if (prevGate.left == gate.left)
        {
            // try to move right
            nextI = i;
            next = verts[gate.right];
            nextVec = glm::normalize(next - apex);
            nextCross = glm::cross(nextVec, vecL);
            nextAngle = glm::orientedAngle(nextVec, vecL, glm::normalize(nextCross));
            if (nextI == leftI) // edge length 0
            {
//                m_pathActual.append(left);
//                return;
            }
            else if (nextCross.y < 0)
            {
                m_pathActual.append(left);
                apex = left;
                i = leftI;

                // find next gate that doesn't include apex
                int currLeft = m_pathNodes.value(i, PathNode()).left;
                gate = m_pathNodes.value(++i, PathNode());
                while (gate.id != -1 && gate.left == currLeft)
                {
                    gate = m_pathNodes.value(++i, PathNode());
                }

                // on last triangle
                if (gate.left == -1)
                {
                    m_pathActual.append(m_endPos + glm::vec3(0, -1, 0));
                    return;
                }
                setVars(&apex, i, &gate, &leftI, &rightI, &left, &right, &vecL, &vecR, &verts);
                cross = glm::cross(vecR, vecL);
                angle = glm::orientedAngle(vecR, vecL, glm::normalize(cross));
            }
            else if (nextAngle < angle)
            {
                rightI = nextI;
                right = next;//*.99f + left*.01f;
                vecR = nextVec;
                cross = nextCross;
                angle = nextAngle;
            }
        }
        else if (prevGate.right == gate.right)
        {
            // try to move left
            nextI = i;
            next = verts[gate.left];
            nextVec = glm::normalize(next - apex);
            nextCross = glm::cross(vecR, nextVec);
            nextAngle = glm::orientedAngle(vecR, nextVec, glm::normalize(nextCross));

            if (nextI == rightI) // edge length 0
            {
//                m_pathActual.append(right);
//                return;
            }
            else if (nextCross.y < 0)
            {
                m_pathActual.append(right);
                apex = right;
                i = rightI;

                // find next gate that doesn't include apex
                int currRight = m_pathNodes.value(i, PathNode()).right;
                gate = m_pathNodes.value(++i, PathNode());
                while (gate.id != -1 && gate.right == currRight)
                {
                    gate = m_pathNodes.value(++i, PathNode());
                }

                // on last triangle
                if (gate.right == -1)
                {
                    m_pathActual.append(m_endPos + glm::vec3(0, -1, 0));
                    return;
                }
                setVars(&apex, i, &gate, &leftI, &rightI, &left, &right, &vecL, &vecR, &verts);
                cross = glm::cross(vecR, vecL);
                angle = glm::orientedAngle(vecR, vecL, glm::normalize(cross));
            }
            else if (nextAngle < angle)
            {
                leftI = nextI;
                left = next;//*.99f + right*.01f;
                vecL = nextVec;
                cross = nextCross;
                angle = nextAngle;
            }
        }
        else // end node
        {
            // check left
            next = m_endPos;
            nextVec = glm::normalize(next - apex);
            nextCross = glm::cross(nextVec, vecL);
            if (nextCross.y < 0)
            {
                m_pathActual.append(left);
                apex = left;
                i = leftI;

                // find next gate that doesn't include apex
                int currLeft = m_pathNodes.value(i, PathNode()).left;
                gate = m_pathNodes.value(++i, PathNode());
                while (gate.id != -1 && gate.left == currLeft)
                {
                    gate = m_pathNodes.value(++i, PathNode());
                }

                // on last triangle
                if (gate.left == -1)
                {
                    m_pathActual.append(m_endPos + glm::vec3(0, -1, 0));
                    return;
                }
                setVars(&apex, i, &gate, &leftI, &rightI, &left, &right, &vecL, &vecR, &verts);
                cross = glm::cross(vecR, vecL);
                angle = glm::orientedAngle(vecR, vecL, glm::normalize(cross));
            }
            else
            {

                // check right
                nextCross = glm::cross(vecR, nextVec);
                if (nextCross.y < 0)
                {
                    m_pathActual.append(right);
                    apex = right;
                    i = rightI;

                    // find next gate that doesn't include apex
                    int currRight = m_pathNodes.value(i, PathNode()).right;
                    gate = m_pathNodes.value(++i, PathNode());
                    while (gate.id != -1 && gate.right == currRight)
                    {
                        gate = m_pathNodes.value(++i, PathNode());
                    }

                    // on last triangle
                    if (gate.right == -1)
                    {
                        m_pathActual.append(m_endPos + glm::vec3(0, -1, 0));
                        return;
                    }
                    setVars(&apex, i, &gate, &leftI, &rightI, &left, &right, &vecL, &vecR, &verts);
                    cross = glm::cross(vecR, vecL);
                    angle = glm::orientedAngle(vecR, vecL, glm::normalize(cross));
                }
                else
                {
                    m_pathActual.append(m_endPos + glm::vec3(0, -1, 0));
                    return;
                }
            }
        }
        prevGate = gate;
    }
}




