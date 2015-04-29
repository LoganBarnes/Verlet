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

//#include "debugprinting.h"

VerletManager::VerletManager(Camera *cam, GLuint shader)
    : Manager(DEFAULT),
      m_dragMode(false),
      m_draggedPoint(0),
      m_draggedVerlet(NULL),
      m_tearMode(false),
      m_tear_ptA(-1),
      m_tear_ptB(-1),
      m_tearVerlet(NULL),
      m_tearLink(NULL)
{
    TriangleMesh* tri2 = new TriangleMesh(glm::vec2(12,52), .3, glm::vec3(0,10,0), this, shader);
    tri2->createPin(0);
    tri2->createPin(11);
    addVerlet(tri2);

//    Net* n2 = new Net(glm::vec2(50,50), glm::vec3(-10,10,10),
//                     glm::vec3(0,0,.3), glm::vec3(.3,0,0), this, shader);
//    for(int i=0;i<10;i++)
//        n2->createPin(i*5);
//    for(int i=0;i<10;i++)
//        n2->createPin((49*50)+i*5);
//    addVerlet(n2);

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
        v->pinConstraint();
        v->linkConstraint();
    }
}

void VerletManager::manage(World *world, float onTickSecs, float mouseX, float mouseY)
{
    setWind(m_windDirection);

    QList<MovableEntity *> mes = world->getMovableEntities();
    QList<OBJ* > obj = world->getObjs();

    Collision *col = new Collision();
    foreach (MovableEntity *me, mes)
    {
        col->mtv = collideTerrain(me);
        col->t = onTickSecs;
        me->handleCollision(col);
        //check if player (bottom of sphere) intersects w/ ground - assumes radius of 1
        glm::vec3 pos = me->getPosition()-glm::vec3(0,1,0);
        foreach(OBJ* o, obj){
            o->pointOnTop(pos);
            pos+=glm::vec3(0,1,0);
            me->setPosition(pos);
        }
    }
    delete col;

    //Collide verlet against terrain
//    foreach(OBJ* o, obj)
//        this->collideSurface(o);

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
            QList<Link*> proximity = m_tearVerlet->link_map[m_tear_ptA];
            std::vector<int> p;
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
            int id = rayTrace(mouseX, mouseY, p, m_tearVerlet);
            if(id>-1){
                m_tear_ptB=id;
                m_tearLink = pairs[m_tear_ptB];
            }
            /*
            foreach(Link* l, proximity){
                if(m_curI==l->pointA||m_curI==l->pointB)
                    m_tear_ptB = m_curI;
            }
            */

        }
        else if(m_tear_ptA>0&&m_tear_ptB>0){
            //Link* tearLink = m_tearVerlet->findLink(m_tear_ptA, m_tear_ptB);
            if(m_tearLink!=NULL)
                m_tearVerlet->tearLink(m_tearLink);
            m_tear_ptA = m_tear_ptB;
            m_tear_ptB = -1;
            m_tearLink = NULL;
        }
    }

    if(solve){
        verlet(onTickSecs);
        for(int i=0; i<_numSolves; i++)
            constraints();
        for(unsigned int i=0; i<verlets.size(); i++)
            verlets.at(i)->onTick(onTickSecs);
        updateBuffer();
    }
}

void VerletManager::onDraw(Graphics *g){
    g->setTexture("");
    for(unsigned int i=0; i<verlets.size(); i++)
        verlets.at(i)->onDraw(g);

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
}

void VerletManager::onMouseDragged(QMouseEvent *, float, float)
{
    //if(m_dragMode)
    //    m_draggedVerlet->setPos(m_draggedPoint, m_interpolate);
}

void VerletManager::onKeyPressed(QKeyEvent *e)
{
    if(e->key() == Qt::Key_T)
        m_tearMode=true;
}

void VerletManager::onKeyReleased(QKeyEvent *e)
{
    switch (e->key())
    {
    case Qt::Key_F:
        enableSolve();
        break;
    case Qt::Key_Down:
        m_windDirection = glm::vec3(0,0,1);
        break;
    case Qt::Key_Up:
        m_windDirection = glm::vec3(0,0,-1);
        break;
    case Qt::Key_Left:
        m_windDirection = glm::vec3(-1,0,0);
        break;
    case Qt::Key_Right:
        m_windDirection = glm::vec3(1,0,0);
        break;
    case Qt::Key_T:
        m_tearMode = false;
        m_tear_ptA=-1;
        m_tear_ptB=-1;
        m_tearVerlet=NULL;
        break;
    default:
        break;
    }
}


