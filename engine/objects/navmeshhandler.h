#ifndef NAVMESHHANDLER_H
#define NAVMESHHANDLER_H

#include <QList>
#include <QHash>
#include <QPoint>
#include "triangle.h"
#include "obj.h"

class Graphics;

struct PathNode
{
    int id;
    int next;
    int right;
    int left;

    PathNode(int id = -1, int next = -1, int right = -1, int left = -1)
    {
        this->id = id;
        this->next = next;
        this->right = right;
        this->left = left;
    }
};

class NavMeshHandler
{
public:
    NavMeshHandler();
    virtual ~NavMeshHandler();

    void setObject(OBJ *obj);
    bool hasObject();
    QList<Triangle*> *getTriangleList();

    void createVBO();

    void draw(glm::mat4 trans);
    void drawLines(glm::mat4 trans);
    void drawPath(glm::mat4 trans, Graphics *g, glm::vec4 color);
    void drawStart(glm::mat4 trans);
    void drawEnd(glm::mat4 trans);

    void setStart(glm::vec3 pos);
    void setEnd(glm::vec3 pos);
    void findPath();

    QList<glm::vec3> getPath();

private:
    void buildGraph();
    void fillVertex(int *index, GLfloat *data, glm::vec3 v);
    int findTriangle(glm::vec3 pos);
    void simpleStupidFunnel();

    QList<Triangle *> m_mesh;
    OBJ *m_obj;

    QHash<int, int* > m_graph;
    QHash<QPair<int, int>, int> m_g;

    int m_start, m_end;
    glm::vec3 m_startPos, m_endPos;
    QList<PathNode> m_pathNodes;
    QList<glm::vec3> m_pathActual;

    GLuint m_shader;
    GLuint m_vaoID;
    GLuint m_vboID;
    int m_numVerts;

};

#endif // NAVMESHHANDLER_H
