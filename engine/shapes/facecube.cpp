#include "facecube.h"

#define GLM_FORCE_RADIANS
#include <glm/gtc/type_ptr.hpp>

FaceCube::FaceCube()
    : Shape(1)
{
}

FaceCube::~FaceCube()
{
}

void FaceCube::calcVerts()
{
    m_numVerts = 6 * 6 - 2;
    int size = m_numVerts * 5;
    m_vertexData = new GLfloat[size];

    float rad = 0.5f;
    glm::vec3 tlf = glm::vec3(-rad, rad, rad);
    glm::vec3 trf = glm::vec3( rad, rad, rad);
    glm::vec3 blf = glm::vec3(-rad,-rad, rad);
    glm::vec3 brf = glm::vec3( rad,-rad, rad);

    glm::vec3 tlb = glm::vec3(-rad, rad, -rad);
    glm::vec3 trb = glm::vec3( rad, rad, -rad);
    glm::vec3 blb = glm::vec3(-rad,-rad, -rad);
    glm::vec3 brb = glm::vec3( rad,-rad, -rad);

    int index = 0;
    // front
    addFace(&index, trf, tlf, brf, blf);
    // rightside
    addFace(&index, trb, trf, brb, brf);
    // back
    addFace(&index, tlb, trb, blb, brb);
    // leftside
    addFace(&index, tlf, tlb, blf, blb);
    // top
    addFace(&index, trb, tlb, trf, tlf);
    // bottom
    addFace(&index, brf, blf, brb, blb);
}


void FaceCube::addFace(int *index, glm::vec3 v1, glm::vec3 v2,
                       glm::vec3 v3, glm::vec3 v4)
{
    addVertexSparseT(index, v1, glm::vec2(1.f, 0.f));
    addVertexSparseT(index, v2, glm::vec2(0.f));
    addVertexSparseT(index, v3, glm::vec2(1.f));
    addVertexSparseT(index, v4, glm::vec2(0.f, 1.f));
}


void FaceCube::updateGL(GLuint shader)
{
    if (m_vaoID)
        glDeleteVertexArrays(1, &m_vaoID);
    if (m_vboID)
        glDeleteBuffers(1, &m_vboID);

    // Initialize the vertex array object.
    glGenVertexArrays(1, &m_vaoID);
    glBindVertexArray(m_vaoID);

    // Initialize the vertex buffer object.
    glGenBuffers(1, &m_vboID);
    glBindBuffer(GL_ARRAY_BUFFER, m_vboID);

    glBufferData(GL_ARRAY_BUFFER, 5 * m_numVerts * sizeof(GLfloat), m_vertexData, GL_STATIC_DRAW);

    GLuint position = glGetAttribLocation(shader, "position");
    GLuint texCoord = glGetAttribLocation(shader, "texCoord");

    glEnableVertexAttribArray(position);
    glVertexAttribPointer(
        position,
        3,                   // Num coordinates per position
        GL_FLOAT,            // Type
        GL_FALSE,            // Normalized
        sizeof(GLfloat) * 5, // Stride
        (void*) 0            // Array buffer offset
    );
    glEnableVertexAttribArray(texCoord);
    glVertexAttribPointer(
        texCoord,
        2,                              // Num coordinates per position
        GL_FLOAT,                       // Type
        GL_TRUE,                        // Normalized
        sizeof(GLfloat) * 5,            // Stride
        (void*) (sizeof(GLfloat) * 3)   // Array buffer offset
    );

    // Unbind buffers.
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void FaceCube::addVertexSparse(int *i, glm::vec3 v)
{
    m_vertexData[(*i)++] = v.x;
    m_vertexData[(*i)++] = v.y;
    m_vertexData[(*i)++] = v.z;
    m_vertexData[(*i)++] = 0;
    m_vertexData[(*i)++] = 0;
}

void FaceCube::addVertexSparseT(int *i, glm::vec3 v, glm::vec2 tex)
{
    m_vertexData[(*i)++] = v.x;
    m_vertexData[(*i)++] = v.y;
    m_vertexData[(*i)++] = v.z;
    m_vertexData[(*i)++] = tex.x;
    m_vertexData[(*i)++] = tex.y;
}


//void FaceCube::transformAndRender(GLuint shader, glm::mat4 trans, int info)
//{
//    glBindVertexArray(m_vaoID);
//    glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(trans));

//    for (int i = 0; i < 6; i++)
//    {
//        if (info & (1 << (13 - i)))
//            glDrawArrays(GL_TRIANGLE_STRIP, (i * 4), 4);

//    }


//    glBindVertexArray(0);
//}









