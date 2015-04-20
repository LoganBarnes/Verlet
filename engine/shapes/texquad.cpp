#include "texquad.h"

#include <iostream>

// Class to generate and render a full screen quad from the final pass fbo
TexQuad::TexQuad(){}

TexQuad::TexQuad(const GLuint vertexLocation, const GLuint normalLocation)
{
    init(vertexLocation, normalLocation);
}

void TexQuad::init(const GLuint vertexLocation, const GLuint texLocation)
{
    m_posLocation = vertexLocation;
    m_texLocation = texLocation;

    // Generate a full screen quad
    GLfloat rad = 1.0f;    // -1 to +1 in clip space
    const GLuint dataSize = ( 3 + 2 ) * 4;
    GLfloat data[dataSize] = { -rad, -rad, 0,    0, 0,        // BL
                                rad, -rad, 0,    1, 0,        // BR
                               -rad,  rad, 0,    0, 1,        // TL
                                rad,  rad, 0,    1, 1};       // TR


    size_t stride = sizeof( GLfloat ) * 3 + sizeof( GLfloat ) * 2;
    glGenVertexArrays( 1, &m_vaoID );
    glBindVertexArray( m_vaoID );

    GLuint vertexBuffer;
    glGenBuffers( 1, &vertexBuffer );
    glBindBuffer( GL_ARRAY_BUFFER, vertexBuffer );
    glBufferData( GL_ARRAY_BUFFER, sizeof(GLfloat) * dataSize, &data[0], GL_STATIC_DRAW);


    glEnableVertexAttribArray( m_posLocation );
    glVertexAttribPointer( m_posLocation, 3, GL_FLOAT, GL_FALSE, stride, 0); //Position

    glEnableVertexAttribArray( m_texLocation );
    glVertexAttribPointer( m_texLocation, 2, GL_FLOAT, GL_FALSE, stride, (void*)(3*sizeof(GLfloat)) ); //Texture coordinate

    glBindVertexArray( 0 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );

//    //Clean up -- unbind things
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glBindVertexArray( 0 );
}

void TexQuad::draw(){
    glBindVertexArray(m_vaoID);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}
