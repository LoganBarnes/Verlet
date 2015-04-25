#include "verlet.h"
#include "verletmanager.h"
#include "graphics.h"
#include "movableentity.h"

#define GLM_FORCE_RADIANS
#include <gtx/norm.hpp>

Verlet::Verlet(VerletManager* m)
{
    _manager = m;
}

Verlet::~Verlet()
{
    for (std::vector<Link*>::iterator it = links.begin() ; it != links.end(); ++it)
        delete (*it);
    links.clear();
}

//***************************creating*****************************//
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

//***************************editing links*****************************//
Link* Verlet::createLink(int a, int b){
    float length = glm::length(_pos[b]-_pos[a]);
    Link* l = new Link(a, b, length);
    links.push_back(l);
    link_map[a]+=l;
    link_map[b]+=l;
    return l;
}

Link* Verlet::findLink(int a, int b){
    foreach(Link* l, links){
        if(l->pointA==a && l->pointB==b)
            return l;
        if(l->pointB==a && l->pointA==b)
            return l;
    }
    std::cout<<"Link from "<<a<<" to "<<b<<" not found"<<std::endl;
    return links.at(0);
}

void Verlet::removeLink(int id){
    QList<Link*> list = link_map[id];
    //clears list of links index is mapped to in 'link_map'
    QList<Link*> at;
    link_map[id]=at;
    //erase all links in 'links'
    foreach(Link* l, list){
        links.erase(std::remove(links.begin(), links.end(), l), links.end());
        delete l;
    }
}

void Verlet::removeLink(Link* l){
    //remove link from 'link_map' of a and b
    removeFromHash(l->pointA,l,link_map);
    removeFromHash(l->pointB,l,link_map);
    //remove link from 'links'
    links.erase(std::remove(links.begin(), links.end(), l), links.end());
    delete l;
}

void Verlet::replaceLink(Link* key, Link* oldLink, Link* newLink,
                               QHash<Link*, QList<Link*> >& hash){
    removeFromHash(key,oldLink,hash);
    hash[key]+=newLink;
}

/*
void Verlet::replaceLink(int key, Link* oldLink, Link* newLink,
                               QHash<int, QList<Link*> >& hash){
    removeFromHash(key,oldLink,hash);
    hash[key]+=newLink;
}
*/

void Verlet::removeFromHash(int key, Link *toRemove, QHash<int, QList<Link *> > &hash){
    QList<Link*> list = hash[key];
    list.removeOne(toRemove);
    hash[key]=list;
}

void Verlet::removeFromHash(Link* key, Link *toRemove, QHash<Link*, QList<Link *> > &hash){
    QList<Link*> list = hash[key];
    list.removeOne(toRemove);
    hash[key]=list;
}

//***************************for tearing*****************************//
Link* Verlet::closestLink(int id, const glm::vec3& point){
    //Find all points id is connected to
    std::vector<int> indices;
    QList<Link*> list = link_map[id];
    foreach(Link* l, list){
        if(l->pointA!=id)
            indices.push_back(l->pointA);
        else
            indices.push_back(l->pointB);
    }
    //Find which of these points is closest to 'point'
    float nearest = 10000;
    int closest = id;
    for(unsigned int i = 0; i<indices.size(); i++){
        int index = indices.at(i);
        float distance = glm::length2(_pos[index]-point);
        if(distance<nearest){
            closest = index;
            nearest = distance;
        }
    }
    //return the link between id + closest point
    return findLink(id,closest);
}

void Verlet::tearLink(Link* ){}

//***************************for update*****************************//
//Updates positions of all particles w/ velocity + acc
void Verlet::verlet(float seconds){
    for(int i=0; i<numPoints; i++) {
        glm::vec3& pos = _pos[i];
        glm::vec3 temp = pos;
        glm::vec3& prevPos = _prevPos[i];
        //apply gravity
        glm::vec3& acc = _acc[i]+=_manager->gravity;
         //update positions
        pos += (pos-prevPos)+acc*seconds*seconds;
        prevPos = temp;
        //reset
        _acc[i]=glm::vec3(0,0,0);
        _normal[i]=glm::vec3(0,0,0);
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
    for(unsigned int i=0; i<pins.size(); i++) {
        Pin p = pins.at(i);
        _pos[p.index] = p.pinPos;
    }
}

//Uses squareroot approximation
void Verlet::linkConstraint(){
    for(unsigned int i=0; i<links.size(); i++) {
        Link* l = links.at(i);
        glm::vec3& posA = _pos[l->pointA];
        glm::vec3& posB = _pos[l->pointB];

        glm::vec3 delta = posB-posA;
        //the closer delta.dot(delta) is to restLengthSq, the smaller the displacement
        delta*=l->restLengthSq / (glm::dot(delta, delta)+l->restLengthSq) - .5;
        posA -= delta;
        posB += delta;
    }
}

void Verlet::onDraw(Graphics *g){
    for(unsigned int i=0; i<links.size(); i++){
        Link* l = links.at(i);
        g->drawLineSeg(_pos[l->pointA],_pos[l->pointB], .1f);
    }
}

void Verlet::onTick(float ){}

glm::vec3 Verlet::collide(MovableEntity *e)
{
    glm::vec3 center = e->getPosition();
    float radius = 1.f;
    bool solve = _manager->solve; //determines whether to move points themselves

    float count = 0;  //how many points hit
    glm::vec3 translation = glm::vec3(0,0,0); //accumulative mtv

    for(int i=0; i<numPoints; i++) {
        glm::vec3 dist =_pos[i]-center; //distance between entity + point
        float radiusSquared = radius * radius;
        if(radiusSquared>glm::length2(dist)){  //colliding
            count++;

            //mtv
            glm::vec3 unit = glm::normalize(dist);
            float factor = glm::length(dist)-radius;
            glm::vec3 extra = unit*factor;

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
        translation /= count; //divide accumulative mtv by points hit
    }

    return translation;

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
