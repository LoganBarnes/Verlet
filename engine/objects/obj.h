#ifndef OBJ_H
#define OBJ_H

#include <GL/glew.h>
#include <glm.hpp>
#include <QList>
#include <QString>

class Triangle;
class Graphics;
class Half;

// A simple parser that reads and writes Wavefront .obj files
class OBJ
{
public:
    struct Index
    {
        int vertex;
        int coord;
        int normal;

        Index() : vertex(-1), coord(-1), normal(-1) {}
        Index(int vertex, int coord, int normal) : vertex(vertex), coord(coord), normal(normal) {}
    };

    struct Tri
    {
        Index a, b, c;

        Tri() {}
        Tri(const Index &a, const Index &b, const Index &c) : a(a), b(b), c(c) {}
    };

    OBJ(GLuint shader,glm::vec4 color);
    virtual ~OBJ();

    QList<glm::vec3> vertices;
    QList<glm::vec2> coords;
    QList<glm::vec3> normals;
    QList<Tri> triangles;

    void draw(glm::mat4 trans, GLuint shader, Graphics *g) const;
    void drawTriangle(int start, glm::mat4 trans) const;
    bool read(const QString &path, QList<Triangle *> *tris, glm::vec3 offset);
    bool write(const QString &path) const;

    void makeTriList(QList<Triangle *> *tris);

    void setTexture(QString tex) { m_texture = tex; }
    void setColor(glm::vec4 color) { m_color = color; }

    GLuint getShader();

    //VERLET COLLISION
    Half* top;
    Half* bot;
    //Adjust a point within a Half's hitbox to be outside of the mesh
    bool pointOnTop(glm::vec3 &surfacePt, const glm::vec3 &prevPt);
    bool pointOnSurface(glm::vec3 &surfacePt, const glm::vec3 &prevPt);
    //Returns y on mesh corresponding w/ given point on x-z plane. Defaults to top half
    bool findY(const glm::vec2& coor, float& y, bool surface = true);
    //Checks if pos is within specified range of top+bot's cylinderical hitboxes
    bool inRange(const glm::vec3& pos, float range);
protected:
    virtual void createVBO();
    GLuint m_shader;

private:
    Index getIndex(const QString &str) const;
    void fillVertex(int *i, GLfloat *data, Index index);

    GLuint m_vaoID;
    GLuint m_vboID;
    int m_numVerts;

    QString m_texture;
    glm::vec4 m_color;

};

#endif // OBJ_H
