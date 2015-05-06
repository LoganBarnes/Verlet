#include "grass.h"
#include "mesh.h"
#include "graphics.h"
#include "obj.h"
#include "movableentity.h"
#include "verletmanager.h"

#define GLM_FORCE_RADIANS
#include <gtc/type_ptr.hpp>
#include <gtx/norm.hpp>

Grass::Grass(VerletManager* vm, GLuint shader):
    Verlet(vm),
    baseColor(glm::vec3(.5,.8,.7)),
    normalMix(glm::vec3(.05,.1,.05)),
    m_shader(shader)
{
    m_mesh = new Mesh();
    windNoise =.15;

}

Grass::~Grass()
{
    for (std::vector<Tri*>::iterator it = _triangles.begin() ; it != _triangles.end(); ++it)
        delete (*it);
    _triangles.clear();
}

//********************************UPDATING*************************************//
void Grass::onTick(float ){
    foreach(Tri* t, _triangles){
        calculate(t);
        applyWind(t);
    }
}

void Grass::applyWind(Tri* t){
    //wind has full effect to perpendicular cloth, none on parallel cloth
    float r = (.03 * (rand() %100))-1.5f;

    glm::vec3 windDirection = _manager->wind;
    windDirection.x+=r;
    windDirection.z+=r;

    glm::vec3 windForce = windDirection*_manager->windPow; //*windScalar;
    _acc[t->a] += windForce;
    _acc[t->b] += windForce;
    _acc[t->c] += windForce;
}

void Grass::updateBuffer()
{
//    m_mesh->setVerts(getPosArray(), getNormArray());
    m_mesh->setTriangles(_triangles, getPosArray());
}

void Grass::onDraw(Graphics *g)
{
    GLuint shader = g->getShader(CURRENT);
    g->setColor(.3f,.7,.5f,1.f,0.f);
    glUniformMatrix4fv(glGetUniformLocation(shader, "model"),
                       1, GL_FALSE, glm::value_ptr(glm::mat4()));
    m_mesh->onDraw(GL_TRIANGLES);
}

void Grass::collideSurface(OBJ* ){}

//********************************CREATING*************************************//
void Grass::createPatch(const glm::vec2& center, float radius, OBJ* obj){
    //Attributes of the patch - can be changed
    int density = 30;
    int segments = 3;
    float minLength = 2;  //1
    float maxLength = 7;  //4

    //Calculations based on attributes
    int numStrands = radius*radius*density;
    int l1 = minLength*10;
    int l2 = (maxLength-minLength)*10;

    for(int i = 0; i<numStrands; i++){
        float length = .01 * (rand() % l2 + l1);

        float x = .01 * (rand() % (int)(radius*100));
        float z = .01 * (rand() % (int)(radius*100));
        x += radius*-.5 + center.x;
        z += radius*-.5 + center.y;
        float y;
        bool onSurface = obj->findY(glm::vec2(x,z),y);
        if(onSurface) //grass extending over edge of platform isn't created
            createStrand3(glm::vec3(x,y,z), segments, length);
    }
    m_mesh->initTriangles(m_shader, _triangles, getPosArray());
}

//Triangular grass
void Grass::createStrand3(const glm::vec3 &s, int segments, float length){
    glm::vec3 interval = glm::vec3(0,length,0);

    //First strand: bend constraints
    int index1 = numPoints;
    createStrand(s,segments,length);

    //Second strand: no bend constraints
    int index2 = numPoints;
    glm::vec3 start2 = s+glm::vec3(length*.5,0,0);  //-interval
    //Create points
    for(int i = 0; i<segments; i++)
        createPoint(start2+(float)i*interval);
    //Structural links
    for(int i = 0; i<segments-1; i++)
        createLink(index2+i,index2+i+1);
    //Connect the two strands
    for(int i = 0; i<segments; i++){
        int bot1 = index1+i;
        int bot2 = index1+segments+i;
        int top1 = index1+i+1;
        int top2 = index1+segments+i+1;
        createLink(bot1,bot2);
        if(i<segments-1){
            createLink(bot1,top2);
            Tri* tri1 = new Tri(bot1,top1,top2);
            Tri* tri2 = new Tri(bot2,bot1,top2);
            calculate(tri1);
            calculate(tri2);
            _triangles.push_back(tri1);
            _triangles.push_back(tri2);
        }
    }
}

//Only links, no triangles
void Grass::createStrand(const glm::vec3 &s, int segments, float length){
    int index0 = numPoints;
    glm::vec3 interval = glm::vec3(0,length,0);
    glm::vec3 start = s-interval;
    //Create points
    for(int i = 0; i<segments; i++)
        createPoint(start+(float)i*interval);
    //Structural links
    for(int i = 0; i<segments-1; i++)
        createLink(index0+i,index0+i+1);
    //Bend constraints
    for(int i = 0; i<segments-2; i++)
        createLink(index0+i,index0+i+2);
    //Secure the two points: 0 below ground level, 1 at ground level
    createPin(index0);
    createPin(index0+1);
}


glm::vec3 Grass::collide(MovableEntity *e)
{
    glm::vec3 center = e->getPosition();
    float radius = 1.f;
    bool solve = _manager->solve; //determines whether to move points themselves

    for(int i=0; i<numPoints; i++) {
        glm::vec3 dist =_pos[i]-center; //distance between entity + point
        float radiusSquared = radius * radius;
        if(radiusSquared>glm::length2(dist)){  //colliding

            //mtv
            glm::vec3 unit = glm::normalize(dist);
            float factor = glm::length(dist)-radius;
            glm::vec3 extra = unit*factor;

            if(solve)
                _pos[i]=_pos[i]-(extra*sphereInfluence);

        }
    }

    return glm::vec3();
}
