#ifndef CUBEMAP_H
#define CUBEMAP_H

#include <GL/glew.h>
#include <QString>
#include <glm/glm.hpp>

class CubeMap
{
public:
    CubeMap();
    ~CubeMap();

    void init();
    void setCube(float radius);
    void setTextures();

    void bindTexture();
    void render();

private:
    bool loadTexture(GLuint tex, GLenum side, const QString &filename);

    GLuint m_texID;
    GLuint m_vboID;
    GLuint m_vaoID;

};

#endif // CUBEMAP_H
