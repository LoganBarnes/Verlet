#ifndef MESHBUFFER_H
#define MESHBUFFER_H

#include <GL/glew.h>
#include <QList>
#include "triangle.h"

class MeshBuffer
{
public:
    MeshBuffer();
    ~MeshBuffer();

    void setBuffer(GLuint shader, QList<Triangle *> tris);
    GLuint getUBO();

private:
    GLuint m_ubo;
    const GLuint m_bindingPoint;

};

#endif // MESHBUFFER_H
