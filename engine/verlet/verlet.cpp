#include "verlet.h"
#include "verletmanager.h"
#include "graphics.h"
//#include "engine/common/graphic.h"
//#include "engine/common/entity.h"
//#include "engine/common/ellipsoid.h"

#define GLM_FORCE_RADIANS
#include <glm/gtx/norm.hpp>

//#include <iostream>

//#include "debugprinting.h"

Verlet::Verlet(VerletManager* m)
{
    _manager = m;
}

Verlet::~Verlet()
{

}

void Verlet::setPos(int index, const glm::vec3& pos){
    _pos[index]=pos;
}

void Verlet::createPoint(const glm::vec3& pos){
    _pos[numPoints] = pos;
    _prevPos[numPoints] = pos;
    numPoints++;
}

void Verlet::createPin(int index){
    Pin p = {index, _pos[index]};
    pins.push_back(p);
}

void Verlet::createLink(int a, int b){
    float length = glm::length(_pos[b]-_pos[a]);
    Link l = Link(a, b, length);
    links.push_back(l);
    link_map[a]+=l;
    //link_map[b]+=l;
}

void Verlet::removeLink(int id){
    QList<Link> list = link_map[id];
    //clears list of links index is mapped to in 'link_map'
    QList<Link> at;
    link_map[id]=at;
    //erase all links in 'links'
    foreach(Link l, list)
        links.erase(std::remove(links.begin(), links.end(), l), links.end());
}

void Verlet::removeLink(Link l){
    int a = l.pointA;
    QList<Link> list = link_map[a];
    //remove link from 'link_map'
    list.removeOne(l);
    link_map[a]=list;
    //remove link from 'links'
    links.erase(std::remove(links.begin(), links.end(), l), links.end());
}

//Updates positions of all particles w/ velocity + acc
void Verlet::verlet(float seconds){
    for(int i=0; i<numPoints; i++) {
        glm::vec3& pos = _pos[i];
        glm::vec3 temp = pos;
        glm::vec3& prevPos = _prevPos[i];
        glm::vec3& acc = _acc[i];
        pos += (pos-prevPos)+acc*seconds*seconds;
        prevPos = temp;
    }
}

void Verlet::applyForce(const glm::vec3& force){
    for(int i=0; i<numPoints; i++)
            _acc[i] += force;
}

void Verlet::resetForce(){
    for(int i=0; i<numPoints; i++)
            _acc[i] = glm::vec3(0,0,0);
}

void Verlet::boxConstraint(const glm::vec3& _boxMin,
                                  const glm::vec3& _boxMax){
    for(int i=0; i<numPoints; i++) {
        glm::vec3& pos = _pos[i];
        pos = glm::max(pos,_boxMin);
        pos = glm::min(pos,_boxMax);
    }
}

void Verlet::pinConstraint(){
    for(int i=0; i<pins.size(); i++) {
        Pin p = pins.at(i);
        _pos[p.index] = p.pinPos;
    }
}

//Uses squareroot approximation
void Verlet::linkConstraint(){
    for(int i=0; i<links.size(); i++) {
        Link l = links.at(i);
        glm::vec3& posA = _pos[l.pointA];
        glm::vec3& posB = _pos[l.pointB];

        glm::vec3 delta = posB-posA;
        //the closer delta.dot(delta) is to restLengthSq, the smaller the displacement
        delta*=l.restLengthSq / (glm::dot(delta, delta)+l.restLengthSq) - .5;
        posA -= delta;
        posB += delta;
    }
}

void Verlet::onDraw(Graphics *g){
//    g->setLineWidth(1);
    for(int i=0; i<links.size(); i++){
        Link l = links.at(i);
        g->drawLineSeg(_pos[l.pointA],_pos[l.pointB], .1f);
    }
}

void Verlet::onTick(float seconds){}

glm::vec3 Verlet::collide(Entity *e){
    /*
    Ellipsoid* el = e->getEllipsoid();
    Vector3 toMove = e->getMove();
    Vector3 center = el->getPos()+toMove;
    float radius = e->getDim().x;
    bool solve = _manager->solve; //determines whether to move points themselves

    float count = 0;  //how many points hit
    Vector3 translation = Vector3(0,0,0); //accumulative mtv

    for(int i=0; i<numPoints; i++) {
        Vector3 dist =_pos[i]-center; //distance between entity + point
        float radiusSquared = radius * radius;
        if(radiusSquared>dist.lengthSquared()){  //colliding
            count++;

            //mtv
            Vector3 unit = dist;
            unit.normalize();
            float factor = dist.length()-radius;
            Vector3 extra = unit*factor;

            if(solve)
                _pos[i]=_pos[i]-(extra*sphereInfluence);

            translation+=extra;
            //note: approximated original method
            //toMove += (extra/12.0);
            //e->setMove(toMove);
        }
     }
    if(count>0){
        //lower = jittery, higher = doesn't compensate for collisions
        count *= .5;
        translation/=count; //divide accumulative mtv by points hit
    }
    toMove+=translation;
    e->setMove(toMove);
    */
}
