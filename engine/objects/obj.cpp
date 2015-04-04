#include "obj.h"
#include <qgl.h>
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include "triangle.h"

#define GLM_FORCE_RADIANS
#include <glm/gtc/type_ptr.hpp>

OBJ::OBJ(GLuint shader)
{
    m_shader = shader;
    m_vaoID = 0;
    m_vboID = 0;
}

OBJ::~OBJ()
{
    if (m_vaoID)
        glDeleteVertexArrays(1, &m_vaoID);
    if (m_vboID)
        glDeleteBuffers(1, &m_vboID);
}

GLuint OBJ::getShader()
{
    return m_shader;
}

void OBJ::draw(glm::mat4 trans) const
{
    glBindVertexArray(m_vaoID);
    glUniformMatrix4fv(glGetUniformLocation(m_shader, "model"), 1, GL_FALSE, glm::value_ptr(trans));
    glDrawArrays(GL_TRIANGLES, 0, m_numVerts);
    glBindVertexArray(0);
}

void OBJ::drawTriangle(int start, glm::mat4 trans) const
{
    glBindVertexArray(m_vaoID);
    glUniformMatrix4fv(glGetUniformLocation(m_shader, "model"), 1, GL_FALSE, glm::value_ptr(trans));
    glDrawArrays(GL_TRIANGLES, start*3, 3);
    glBindVertexArray(0);
}

static bool inBounds(int i, int size)
{
    return (i >= 0 && i < size);
}

bool OBJ::read(const QString &path, QList<Triangle *> *tris)
{
    // Open the file
    QFile file(path);
    if (!file.open(QFile::ReadOnly | QFile::Text)) return false;
    QTextStream f(&file);
    QString line;

    tris->clear();

    // Read the file
    QRegExp spaces("\\s+");
    do {
        line = f.readLine().trimmed();
        QStringList parts = line.split(spaces);
        if (parts.isEmpty()) continue;

        if (parts[0] == "v" && parts.count() >= 4) {
            vertices += glm::vec3(parts[1].toFloat(), parts[2].toFloat(), parts[3].toFloat());
        } else if (parts[0] == "vt" && parts.count() >= 3) {
            coords += glm::vec2(parts[1].toFloat(), parts[2].toFloat());
        } else if (parts[0] == "vn" && parts.count() >= 4) {
            normals += glm::vec3(parts[1].toFloat(), parts[2].toFloat(), parts[3].toFloat());
        } else if (parts[0] == "f" && parts.count() >= 4) {
            // Convert polygons into triangle fans
            Index a = getIndex(parts[1]);
            Index b = getIndex(parts[2]);
            for (int i = 3; i < parts.count(); i++) {
                Index c = getIndex(parts[i]);
                triangles += Tri(a, b, c);
                tris->append(new Triangle(vertices[a.vertex], vertices[b.vertex], vertices[c.vertex]));
                b = c;
            }
        }
    } while (!line.isNull());

    createVBO();

    return true;
}

void OBJ::createVBO()
{
    // delete old array and buffer
    if (m_vaoID)
        glDeleteVertexArrays(1, &m_vaoID);
    if (m_vboID)
        glDeleteBuffers(1, &m_vboID);

    // build new data array
    m_numVerts = triangles.size() * 3;
    int size = m_numVerts * 8;
    GLfloat vertexData[size];

    int index = 0;
    foreach (Tri t, triangles)
    {
        fillVertex(&index, vertexData, t.a);
        fillVertex(&index, vertexData, t.b);
        fillVertex(&index, vertexData, t.c);
    }

    // Initialize the vertex array object.
    glGenVertexArrays(1, &m_vaoID);
    glBindVertexArray(m_vaoID);

    // Initialize the vertex buffer object.
    glGenBuffers(1, &m_vboID);
    glBindBuffer(GL_ARRAY_BUFFER, m_vboID);

    glBufferData(GL_ARRAY_BUFFER, 8 * m_numVerts * sizeof(GLfloat), vertexData, GL_STATIC_DRAW);

    GLuint position = glGetAttribLocation(m_shader, "position");
    GLuint normal = glGetAttribLocation(m_shader, "normal");
    GLuint texCoord = glGetAttribLocation(m_shader, "texCoord");

    glEnableVertexAttribArray(position);
    glVertexAttribPointer(
        position,
        3,                   // Num coordinates per position
        GL_FLOAT,            // Type
        GL_FALSE,            // Normalized
        sizeof(GLfloat) * 8, // Stride
        (void*) 0            // Array buffer offset
    );
    glEnableVertexAttribArray(normal);
    glVertexAttribPointer(
        normal,
        3,                              // Num coordinates per normal
        GL_FLOAT,                       // Type
        GL_TRUE,                        // Normalized
        sizeof(GLfloat) * 8,            // Stride
        (void*) (sizeof(GLfloat) * 3)   // Array buffer offset
    );
    glEnableVertexAttribArray(texCoord);
    glVertexAttribPointer(
        texCoord,
        2,                              // Num coordinates per position
        GL_FLOAT,                       // Type
        GL_TRUE,                        // Normalized
        sizeof(GLfloat) * 8,            // Stride
        (void*) (sizeof(GLfloat) * 6)   // Array buffer offset
    );

    // Unbind buffers.
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void OBJ::fillVertex(int *i, GLfloat *data, Index index)
{
    glm::vec3 v = vertices.value(index.vertex);
    glm::vec3 n = (inBounds(index.normal, normals.size()) ? normals.value(index.normal) : glm::vec3());
    glm::vec2 c = coords.value(index.coord);

    data[(*i)++] = v.x;
    data[(*i)++] = v.y;
    data[(*i)++] = v.z;
    data[(*i)++] = n.x;
    data[(*i)++] = n.y;
    data[(*i)++] = n.z;
    data[(*i)++] = c.x;
    data[(*i)++] = c.y;
}


static QString str(const glm::vec2 &v) { return QString("%1 %2").arg(v.x).arg(v.y); }
static QString str(const glm::vec3 &v) { return QString("%1 %2 %3").arg(v.x).arg(v.y).arg(v.z); }

static QString str(const OBJ::Index &i)
{
    if (i.normal >= 0) {
        if (i.coord >= 0) return QString("%1/%2/%3").arg(i.vertex + 1).arg(i.coord + 1).arg(i.normal + 1);
        return QString("%1//%2").arg(i.vertex + 1).arg(i.normal + 1);
    } else {
        if (i.coord >= 0) return QString("%1/%2").arg(i.vertex + 1).arg(i.coord + 1);
        return QString("%1").arg(i.vertex + 1);
    }
}

bool OBJ::write(const QString &path) const
{
    // Open the file
    QFile file(path);
    if (!file.open(QFile::WriteOnly | QFile::Text)) return false;
    QTextStream f(&file);

    // Write the file
    foreach (const glm::vec3 &vertex, vertices) f << "v " << str(vertex) << '\n';
    foreach (const glm::vec2 &coord, coords) f << "vt " << str(coord) << '\n';
    foreach (const glm::vec3 &normal, normals) f << "vn " << str(normal) << '\n';
    foreach (const Tri &tri, triangles) f << "f " << str(tri.a) << ' ' << str(tri.b) << ' ' << str(tri.c) << '\n';

    return true;
}

inline int relativeIndex(int index, int count)
{
    return index >= 0 ? index - 1 : count + index;
}

OBJ::Index OBJ::getIndex(const QString &str) const
{
    QStringList parts = str.split('/');
    int vertex = parts.count() > 0 ? relativeIndex(parts[0].toInt(), vertices.size()) : -1;
    int coord = parts.count() > 1 ? relativeIndex(parts[1].toInt(), coords.size()) : -1;
    int normal = parts.count() > 2 ? relativeIndex(parts[2].toInt(), normals.size()) : -1;
    return Index(vertex, coord, normal);
}
