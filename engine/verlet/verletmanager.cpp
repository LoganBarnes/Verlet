#include "verletmanager.h"
#include <iostream>
//#include "engine/common/graphic.h"
#include "engine/common/raytracer.h"
//#include "engine/common/entity.h"
//#include "engine/common/ellipsoid.h"
#include "rope.h"
#include "trianglemesh.h"
#include "net.h"
#include "verletcube.h"

VerletManager::VerletManager(GLuint shader)
    : Manager(DEFAULT)
{
    //initial curtain
    /*
    Net* n = new Net(glm::vec2(25,25), glm::vec3(2,15,2),
                     glm::vec3(.7,0,0), glm::vec3(0,.7,0));
    for(int i=0;i<5;i++)
        n->createPin(i*5);
    addVerlet(n);
    */

    // test

    Net* n = new Net(glm::vec2(5,5), glm::vec3(-2.5f,7,-5),
                     glm::vec3(1,0,0), glm::vec3(0,1,0), this, shader);
    for(int i=0;i<5;i+=2)
        n->createPin(i);
    addVerlet(n);


    TriangleMesh* tri2 = new TriangleMesh(glm::vec2(12,15), .3, glm::vec3(6,7,0), this, shader);
    tri2->createPin(0);
    tri2->createPin(11);
    addVerlet(tri2);

//    Net* n1 = new Net(glm::vec2(25,25), glm::vec3(2,20,2),
//                     glm::vec3(0,0,.3), glm::vec3(0,-.3,0));
//    for(int i=0;i<5;i++)
//        n1->createPin(i*5);
//    addVerlet(n1);


//    Net* n2 = new Net(glm::vec2(50,50), glm::vec3(0,21,0),
//                     glm::vec3(0,0,.3), glm::vec3(.3,0,0), shader);
//    for(int i=0;i<10;i++)
//        n2->createPin(i*5);
//    for(int i=0;i<10;i++)
//        n2->createPin((49*50)+i*5);
//    addVerlet(n2);


    //huge draping net
    /*
    Net* n2 = new Net(glm::vec2(150,150), glm::vec3(2,2,2),
                     glm::vec3(0,0,.5), glm::vec3(.5,0,0));
    for(int i=0;i<30;i++)
        n2->createPin(i*5);
    for(int i=0;i<30;i++)
        n2->createPin((149*150)+i*5);
    addVerlet(n2);
    */

    VerletCube* c2 = new VerletCube(glm::vec3(0,20,0), glm::vec3(1,21,1), this);
//    addVerlet(c2);

}

VerletManager::~VerletManager()
{
    for (std::vector<Verlet*>::iterator it = verlets.begin() ;
         it != verlets.end(); ++it)
        delete (*it);
    verlets.clear();
}


bool VerletManager::rayTrace(RayTracer* ray, HitTest &result){
    bool h = false;
    for(int i = 0; i<verlets.size(); i++){
        Verlet* v = verlets[i];
        HitTest temp;
        bool hit = ray->hitVerlet(v,temp);
        if(hit&&(temp.t<result.t)){
            h = true;
            result = temp;
        }
    }
    return h;
}

void VerletManager::addVerlet(Verlet* v){
    verlets.push_back(v);
}

void VerletManager::updateBuffer(){
    for(int i=0; i<verlets.size(); i++)
        verlets.at(i)->updateBuffer();
}

void VerletManager::verlet(float seconds){
    for(int i=0; i<verlets.size(); i++)
        verlets.at(i)->verlet(seconds);
}

void VerletManager::accumulateForces(){
    glm::vec3 totalForce = gravity + wind;
    for(int i=0; i<verlets.size(); i++)
        verlets.at(i)->applyForce(totalForce);
}

void VerletManager::resetForces(){
    for(int i=0; i<verlets.size(); i++)
        verlets.at(i)->resetForce();
}

void VerletManager::constraints(){
    for(int i=0; i<verlets.size(); i++) {
        Verlet* v = verlets.at(i);
//        v->boxConstraint(_boxMin, _boxMax);
        v->linkConstraint();
        v->pinConstraint();
    }
}

void VerletManager::manage(World *, float onTickSecs)
{
    if(solve){
        accumulateForces();
        verlet(onTickSecs);
        for(int i=0; i<_numSolves; i++)
            constraints();
        resetForces();
        for(int i=0; i<verlets.size(); i++)
            verlets.at(i)->onTick(onTickSecs);
        updateBuffer();
    }
    else
        resetForces();
}

void VerletManager::onDraw(Graphics *g){

    g->setTexture("");
    for(int i=0; i<verlets.size(); i++)
        verlets.at(i)->onDraw(g);
//    g->setColor(glm::vec3(1,1,1));
    g->drawLineSeg(_boxMin,glm::vec3(_boxMin.x,_boxMin.y,_boxMax.z), .3f);
    g->drawLineSeg(_boxMin,glm::vec3(_boxMin.x,_boxMax.y,_boxMin.z), .3f);
    g->drawLineSeg(_boxMin,glm::vec3(_boxMax.x,_boxMin.y,_boxMin.z), .3f);

}

glm::vec3 VerletManager::collideTerrain(Entity* e){
    /*
    for(int i=0; i<verlets.size(); i++)
        verlets.at(i)->collide(e);
        */
}

