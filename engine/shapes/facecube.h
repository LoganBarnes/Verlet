#ifndef MCCUBE_H
#define MCCUBE_H

#include "shape.h"

class FaceCube : public Shape
{
public:
    FaceCube();
    virtual ~FaceCube();

    virtual void calcVerts();
    virtual void updateGL(GLuint shader);

//    virtual void transformAndRender(GLuint shader, glm::mat4 trans, int faces);

protected:
    void addFace(int *index, glm::vec3 v1, glm::vec3 v2, glm::vec3 v3, glm::vec3 v4);
    void addVertexSparse(int *i, glm::vec3 v);
    void addVertexSparseT(int *i, glm::vec3 v, glm::vec2 tex);

};

#endif // MCCUBE_H
