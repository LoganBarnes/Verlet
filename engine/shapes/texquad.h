#ifndef TEXQUAD_H
#define TEXQUAD_H

#include "GL/glew.h"

class TexQuad
{
public:
    //Uninitialized quad
    TexQuad();
    //Initialized quad
    TexQuad(const GLuint vertexLocation, const GLuint normalLocation);
    void init(const GLuint vertexLocation, const GLuint normalLocation);
    void draw();

private:
    GLuint m_vaoID;
    GLuint m_posLocation;
    GLuint m_texLocation;
};

#endif // TEXQUAD_H
