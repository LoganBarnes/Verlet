#ifndef GRASS_H
#define GRASS_H

#include "verlet.h"

class Mesh;
class VerletManager;
typedef unsigned int GLuint;

class Grass: public Verlet
{
public:
    Grass(VerletManager *vm, GLuint shader);
    virtual ~Grass();

    void onTick(float seconds); //updates triangle vertices + normal
    void onDraw(Graphics *g);
    virtual void updateBuffer();

    //Grass doesn't collide with the surface, to save on runtimes
    virtual void collideSurface(OBJ* obj);

    // grass doesn't move the entity
    virtual glm::vec3 collide(MovableEntity *e);

    void createPatch(const glm::vec2 &center, float radius, OBJ* obj);
    void setColor(const glm::vec3& base, const glm::vec3& mix){
        baseColor = base;
        normalMix = mix;
    }
private:
    std::vector<Tri*> _triangles;
    //@param start: point on ground level
    //2 strands, 1 w/ bend, and triangular offset
    void createStrand3(const glm::vec3& start,int segments,float length);
    //1 strand, w/ bend
    void createStrand(const glm::vec3& start, int segments, float length);

    glm::vec3 baseColor;
    glm::vec3 normalMix; //normals are multiplied component-wise and added to base

    Mesh *m_mesh;
    GLuint m_shader;
};

#endif // GRASS_H
