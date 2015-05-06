#include "verletmanager.h"
#include "rope.h"
#include "trianglemesh.h"
#include "net.h"
#include "verletcube.h"
#include "ray.h"
#include "world.h"
#include "obj.h"
#include "verlet.h"

#define GLM_FORCE_RADIANS
#include <gtc/matrix_transform.hpp>
#include <gtx/norm.hpp>

//#include "debugprinting.h"

VerletManager::VerletManager(Camera *cam)
    : Manager(DEFAULT),
      windPow(0),
      m_dragMode(false),
      m_draggedPoint(0),
      m_draggedVerlet(NULL),
      m_windStart(false),
      m_windMode(false),
      m_windMaxPow(2.5),
      m_windScalar(4),
      m_tearMode(false),
      m_tear_ptA(-1),
      m_tear_ptB(-1),
      m_tear_prevA(-1),
      m_tearVerlet(NULL),
      m_tearLink(NULL),
      m_tearDelay(2),
      m_tearTimer(0)
{
    m_ray = new Ray(cam);
    m_curV = -1;
    m_curI = -1;
}

VerletManager::~VerletManager()
{
    for (std::vector<Verlet*>::iterator it = verlets.begin() ;
         it != verlets.end(); ++it)
        delete (*it);
    verlets.clear();

    delete m_ray;
}


void VerletManager::rayTrace(float x, float y)
{
    m_ray->setRay(x, y);

    float bestT = std::numeric_limits<float>::infinity();
    float t, radius;
    int verlet = -1;
    int index = -1;
    Verlet *v;
    int numVerlets = verlets.size();
    int numVerts;

    for (int i = 0; i < numVerlets; i++)
    {
        v = verlets.at(i);
        numVerts = v->getSize();
        radius = v->rayTraceSize;
        for (int j = 0; j < numVerts; j++)
        {
            t = m_ray->intersectPoint(v->getPoint(j), radius).w;
            if (t < bestT)
            {
                bestT = t;
                verlet = i;
                index = j;
            }
        }
    }
    m_curV = verlet;
    m_curI = index;
}

int VerletManager::rayTrace(float x, float y, std::vector<int> points, Verlet* v)
{
    m_ray->setRay(x, y);

    float bestT = std::numeric_limits<float>::infinity();
    float t;
    int index = -1;
    float radius = v->rayTraceSize;;
    for (uint i = 0; i < points.size(); i++){
        t = m_ray->intersectPoint(v->getPoint(points[i]), radius).w;
        if (t < bestT)
        {
            bestT = t;
            index = points[i];
        }
    }
    return index;
}

void VerletManager::addVerlet(Verlet* v){
    verlets.push_back(v);
}

void VerletManager::updateBuffer(){
    for(unsigned int i=0; i<verlets.size(); i++)
        verlets.at(i)->updateBuffer();
}

void VerletManager::verlet(float seconds){
    for(unsigned int i=0; i<verlets.size(); i++)
        verlets.at(i)->verlet(seconds);
}

void VerletManager::constraints(){
    for(unsigned int i=0; i<verlets.size(); i++) {
        Verlet* v = verlets.at(i);
        v->linkConstraint();
        v->pinConstraint();
    }
}

void VerletManager::manage(World *world, float onTickSecs, float mouseX, float mouseY)
{
    //Wind direction
    if(m_windStart){
        m_windStart = false;
        m_windStartPos = glm::vec2(mouseX,mouseY);

        //find point in world space for drawing visualization
        Camera* cam = world->getPlayer()->getCamera();
        glm::vec3 look = glm::vec3(cam->getLook());
        glm::vec3 direction = -1.0f*look;
        glm::vec3 source = look + world->getPlayer()->getEyePos();
        m_windStartVis = m_ray->getPointonPlane(source,direction);
        m_windEndVis = m_windStartVis;
    }
    if(m_windMode){
        glm::vec2 m_windEndPos = glm::vec2(mouseX,mouseY);
        if(m_windStartPos!=m_windEndPos){ //causes nan -> disappearing verlet
            glm::vec2 wind = m_windEndPos-m_windStartPos;
            //Use length of drawn vector to determine wind strength
            float factor = glm::length2(wind) * m_windScalar;
            windPow = (factor>m_windMaxPow) ? m_windMaxPow : factor;

            //Convert 'wind' (mouse movement in screenspace) to a vector on the xz plane
            Camera* cam = world->getPlayer()->getCamera();
            //Vectors are relative to where the camera is facing
            glm::vec3 right = cam->getRight();
            glm::vec3 forward = (glm::vec3) cam->getLook();
            //Cancel out the y components, to that pitch doesn't effect calculations
            //Since camera looks down at player, vector slopes down when blowing away && up when blowing towards player
            forward.y = 0;
            forward = glm::normalize(forward);
            right.y = 0;
            right = glm::normalize(right);
            //Find the x (right) + z (forward) components and add them
            glm::vec3 forwardComponent = forward*wind.y;
            glm::vec3 rightComponent = right*wind.x;
            glm::vec3 composite = forwardComponent+rightComponent;
            composite = glm::normalize(composite)+this->gravity*-.075f*windPow; //offset gravity
            m_windDirection = composite;

            //Assign wind sign, so that noise can be correctly appleid per triangle in applyWind()
            windSign = findSign(m_windDirection);

            glm::vec3 look = glm::vec3(cam->getLook());
            glm::vec3 direction = -1.0f*look;
            glm::vec3 source = look + world->getPlayer()->getEyePos();
            m_windEndVis = m_ray->getPointonPlane(source,direction);
        }
    }
/*
    float r = (.01 * (rand() %100))-0.5f;
    glm::vec3 test = m_windDirection;
    test.x+=r;
    test.z+=r;
    setWind(test);
*/
    setWind(m_windDirection);

    QList<MovableEntity *> mes = world->getMovableEntities();
    QList<OBJ* > obj = world->getObjs();

    Collision *col = new Collision();
    foreach (MovableEntity *me, mes)
    {
        glm::vec3 prev = me->getPosition();
        col->mtv = collideTerrain(me) * .5f;
        col->t = onTickSecs;
        me->handleCollision(col, !solve);
        //check if player (bottom of sphere) intersects w/ ground - assumes radius of 1

        foreach(OBJ* o, obj){
            glm::vec3 pos = me->getPosition()-glm::vec3(0,1,0);
            o->pointOnTop(pos,prev);
            pos+=glm::vec3(0,1,0);
            me->setPosition(pos);
        }
    }
    delete col;

    rayTrace(mouseX, mouseY);

    if(m_dragMode){
        glm::vec3 point = m_draggedVerlet->getPoint(m_draggedPoint);
        glm::vec4 n = world->getPlayer()->getCamera()->getLook();
        n*=-1;

        float t = m_ray->hitPlane(point,glm::vec3(n));
        m_draggedMouse = m_ray->getPoint(t);

        m_interpolate = m_draggedMouse;
        m_draggedVerlet->setPos(m_draggedPoint, m_interpolate);
    }

    //If setting m_tear_ptB is based on direction the mouse travels, might be better
    if(m_tearMode){
        if(m_tear_ptA<0 && m_curV > -1){
            m_tear_ptA = m_curI;
            m_tearVerlet = getVerlet(m_curV);
        }
        else if(m_tear_ptA>0&&m_tear_ptB<0){
            QList<Link*> proximity = m_tearVerlet->_linkMap[m_tear_ptA].links; //link_map[m_tear_ptA];
            QList<int> p;
            QHash<int,Link*> pairs;
            foreach(Link* l, proximity){
                if(l->pointA==m_tear_ptA){
                    p.push_back(l->pointB);
                    pairs[l->pointB]=l;
                }
                else if(l->pointB==m_tear_ptA){
                    p.push_back(l->pointA);
                    pairs[l->pointA]=l;
                }
            }
            //_ray->hitVerlet(tearVerlet,h);
            if(m_curV > -1 &&getVerlet(m_curV)==m_tearVerlet&&p.contains(m_curI)){
                m_tear_ptB= m_curI;
                m_tearLink = pairs[m_tear_ptB];
            }
            glm::vec3 point = m_tearVerlet->getPoint(m_tear_ptA);
            glm::vec4 n = world->getPlayer()->getCamera()->getLook();
            n*=-1;

            float t = m_ray->hitPlane(point,glm::vec3(n));
            glm::vec3 tearMouse = m_ray->getPoint(t);

            float d = glm::length2(tearMouse-m_tearVerlet->getPoint(m_tear_ptA));
            if(m_tear_ptB<0&&d>.5){
                int near = -1;
                float closest = 1000;
                foreach(int i,p){
                    float dist = glm::length2(tearMouse-m_tearVerlet->getPoint(i));
                    if(dist<closest){
                        closest = dist;
                        near = i;
                    }
                }
                if(near>0){
                    m_tear_ptB=near;
                    m_tearLink = pairs[m_tear_ptB];
                }
            }
            if(m_tear_ptB==m_tear_prevA)
                m_tear_ptB=-1;
        }
        else if(m_tear_ptA>0&&m_tear_ptB>0&&m_tearLink!=NULL){
            m_tearVerlet->tearLink(m_tearLink);
            m_tear_prevA = m_tear_ptA;
            m_tear_ptA = m_tear_ptB;
            m_tear_ptB = -1;
            m_tearLink = NULL;
            m_tearTimer=m_tearDelay;
        }
        else if(m_tearTimer!=0)
            m_tearTimer--;
    }
    // range check first

    if(solve){
        verlet(onTickSecs);
        for(int i=0; i<_numSolves; i++)
            constraints();
        for(unsigned int i=0; i<verlets.size(); i++)
            verlets.at(i)->onTick(onTickSecs);
        updateBuffer();
    }

    //Collide verlet against terrain
    foreach(OBJ* o, obj)
        this->collideSurface(o);
}

void VerletManager::onDraw(Graphics *g){
    g->setTexture("");
    for(unsigned int i=0; i<verlets.size(); i++)
        verlets.at(i)->onDraw(g);

    //for wind
    g->setAllWhite(true);
    if(m_windMode){
        glm::mat4 trans = glm::translate(glm::mat4(), m_windStartVis);
        trans *= glm::scale(glm::mat4(), glm::vec3(.2,.2,.2));
        g->drawSphere(trans);
        g->drawLineSeg(m_windStartVis,m_windEndVis,.1,4);
    }
    g->setAllWhite(false);

    //for dragging
    if(m_dragMode){
        g->setColor(1, 1, 1, 1, 0);
        glm::mat4 trans = glm::translate(glm::mat4(), m_draggedVerlet->getPoint(m_draggedPoint));
        trans *= glm::scale(glm::mat4(), glm::vec3(.2,.2,.2));
        g->drawSphere(trans);
    }
    //for tearing
    if(m_tearMode){
        g->setColor(1, 0, 0, 1, 0);
        if(m_tear_ptA>0&&m_tearVerlet!=NULL){
            //std::cout<<"point a:"<<m_tear_ptA<<std::endl;

            glm::mat4 trans = glm::translate(glm::mat4(), m_tearVerlet->getPoint(m_tear_ptA));
            trans *= glm::scale(glm::mat4(), glm::vec3(.2,.2,.2));
            g->drawSphere(trans);
            /*
            for(unsigned int i=0; i<4; i++){
                for(unsigned int j=0; j<4; j++){
                    std::cout<<trans[i][j]<<"  ";
                }
                std::cout<<std::endl;
            }
            */
        }
    }
}

glm::vec3 VerletManager::collideTerrain(MovableEntity *e)
{
    glm::vec3 mtv = glm::vec3(0);

    for(unsigned int i=0; i<verlets.size(); i++)
        mtv += verlets.at(i)->collide(e);

    return mtv;
}

void VerletManager::collideSurface(OBJ* obj){
    foreach(Verlet* v, verlets)
        v->collideSurface(obj);
}

void VerletManager::onMousePressed(QMouseEvent *e)
{
    //dragging
    if(e->button() == Qt::LeftButton && m_curV>-1){
        m_dragMode = true;
        m_draggedPoint = m_curI;
        m_draggedVerlet = getVerlet(m_curV);
        m_interpolate = m_draggedVerlet->getPoint(m_draggedPoint);
    }
    //freeze
//    if(e->type() == QEvent::MouseButtonDblClick && e->button() == Qt::LeftButton)
//        enableSolve();
    //tear
    if(e->button() == Qt::RightButton)
        m_tearMode=true;
}

void VerletManager::onMouseMoved(QMouseEvent *, float, float)
{
    if(m_dragMode)
        m_draggedVerlet->setPos(m_draggedPoint, m_interpolate);
}

void VerletManager::onMouseReleased(QMouseEvent *e)
{
    if(e->button() == Qt::LeftButton)
        m_dragMode = false;
    if(e->button() == Qt::RightButton){
        m_tearMode = false;
        m_tear_ptA=-1;
        m_tear_ptB=-1;
        m_tearVerlet=NULL;
    }
}

void VerletManager::onMouseDragged(QMouseEvent *, float, float)
{}

void VerletManager::onKeyPressed(QKeyEvent *e)
{
    //wind
    if(e->key() == Qt::Key_Shift){
        m_windStart = true;
        m_windMode = true;
    }
}

void VerletManager::onKeyReleased(QKeyEvent *e)
{
    switch (e->key())
    {
    case Qt::Key_F:
        enableSolve();
        break;
    case Qt::Key_Shift:
        m_windMode = false;
        break;
    case Qt::Key_Down:
        m_windDirection = glm::vec3(1,0,0);
        break;
    case Qt::Key_Up:
        m_windDirection = glm::vec3(-1,0,0);
        break;
    case Qt::Key_Left:
        m_windDirection = glm::vec3(0,0,1);
        break;
    case Qt::Key_Right:
        m_windDirection = glm::vec3(0,0,-1);
        break;
    case Qt::Key_C:
        m_windDirection = glm::vec3();
        break;
    default:
        break;
    }
}


