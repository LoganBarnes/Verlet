#include "verlet.h"
#include "verletmanager.h"
#include "graphics.h"
#include "movableentity.h"
#include "obj.h"
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

void Verlet::removeLink(Link* l){
    //remove link from 'link_map' of a and b
    link_map[l->pointA].removeOne(l);
    link_map[l->pointB].removeOne(l);
    //remove link from 'links'
    links.erase(std::remove(links.begin(), links.end(), l), links.end());
    delete l;
}

void Verlet::replaceLink(Link* key, Link* oldLink, Link* newLink,
                               QHash<Link*, QList<Link*> >& hash){
    hash[key].removeOne(oldLink);
    hash[key]+=newLink;
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
    float nearest = 100000000;
    int closest = id;
    for(uint i = 0; i<indices.size(); i++){
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
         //update positions- .99 so system stablizes faster
        pos += .99f*(pos-prevPos)+acc*seconds*seconds;
        prevPos = temp;
        //reset
        _acc[i]=glm::vec3(0,0,0);
        _normal[i]=glm::vec3(0,0,0);
    }
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

//***************************Updating triangles*****************************//

void Verlet::applyWind(Tri* t){
    //wind has full effect to perpendicular cloth, none on parallel cloth
    glm::vec3 windDirection = _manager->wind;
    float windScalar =  glm::dot(windDirection, t->normal);

    if(windScalar<0)
        windScalar*=-1;
    t->windForce = windScalar;

    glm::vec3 windForce = windDirection*windScalar*_manager->windPow;

    _acc[t->a] += windForce;
    _acc[t->b] += windForce;
    _acc[t->c] += windForce;
}


void Verlet::calculate(Tri* t){
    t->vertices[0]=_pos[t->a];
    t->vertices[1]=_pos[t->b];
    t->vertices[2]=_pos[t->c];

    t->normal =  glm::cross((t->vertices[1] - t->vertices[0]), (t->vertices[2] - t->vertices[0]));
    t->normal = glm::normalize(t->normal);

    //Uncomment for per-vertex normals- not currently being used
    /*
    _normal[t->a]+=t->normal*_scalar[t->a];
    _normal[t->b]+=t->normal*_scalar[t->b];
    _normal[t->c]+=t->normal*_scalar[t->c];
    */
}
//***************************Collisions*****************************//

void Verlet::collideSurface(OBJ* obj){
    for(int i = 0; i < numPoints; i++){
        glm::vec3 flatPoint = _pos[i];
        glm::vec3 prev = _prevPos[i];
        float difference = glm::length2(prev-flatPoint); //only perform check if velocity is high
        if(difference>.000001){
            obj->pointOnSurface(_pos[i]);
        }
    }
}

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
