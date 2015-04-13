#include "verlet.h"
#include "graphics.h"
//#include "engine/common/graphic.h"
//#include "engine/common/entity.h"
//#include "engine/common/ellipsoid.h"

#define GLM_FORCE_RADIANS
#include <glm/gtx/norm.hpp>

#include <iostream>

#include "debugprinting.h"

Verlet::Verlet()
{

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
    cout << glm::to_string(_pos[index]) << endl;
}

void Verlet::createLink(int a, int b){
    float length = (_pos[b]-_pos[a]).length();
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
            _acc[i] = force;
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

glm::vec3 Verlet::collide(Entity *e){
//    Ellipsoid* el = e->getEllipsoid();
//    glm::vec3 toMove = e->getMove();
//    glm::vec3 center = el->getPos()+toMove;

    for(int i=0; i<numPoints; i++) {
        glm::vec3 dist =_pos[i];//-center; //distance between entity + point

        //Assumes entity is a sphere of radius 1
        if(1>glm::length2(dist)){  //colliding
            //mtv
            glm::vec3 unit = dist;
            unit = glm::normalize(unit);
            float factor = dist.length()-1;
            glm::vec3 extra = unit*factor;

            _pos[i]=_pos[i]-(extra * 0.5f);

            //note: approximated.
            //Need to have counter of points hit + divide by that
            //And a glm::vec3 tracking sums, to add toMove after all collisions per verlet?
//            toMove += (extra / 12.f);
//            e->setMove(toMove);
        }
     }
}
