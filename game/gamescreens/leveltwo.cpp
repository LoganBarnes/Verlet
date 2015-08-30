#include "leveltwo.h"
#include "application.h"
#include "gameworld.h"
#include "objecthandler.h"
#include "obj.h"
#include "actioncamera.h"
#include "gameplayer.h"
#include "geometriccollisionmanager.h"
#include "verletmanager.h"
#include "audio.h"
#include "soundtester.h"
#include "grass.h"
#include "trianglemesh.h"
#include "half.h"
#ifdef CUDA
#include "particlesystemmanager.h"
#endif

#define PI 3.1415926535897932f

#include "debugprinting.h"

LevelTwo::LevelTwo(Application *parent)
    : ScreenH(parent),
      m_world(NULL),
      m_oh(NULL),
      m_resetIndex(0),
      m_spiraling(false),
      m_spiralIndex(0),
      m_spiralDelay(30),
      m_spiralCounter(0)
{
    m_parentApp->setMouseDecoupled(true);
    m_parentApp->setLeapRightClick(GRAB);
    m_parentApp->setLeapLeftClick(PINCH);

    m_oh = new ObjectHandler();

    resetWorld(glm::vec3(0, 10, 0));
    m_parentApp->playMusic("darkness.wav");
}


LevelTwo::~LevelTwo()
{
    delete m_world;
    delete m_oh; // m_level is deleted here
}

OBJ* LevelTwo::addIsland(const QString& path, GLuint shader, const glm::vec3& offset, glm::vec4 color, ParticleSystemManager *psm){

    QList<Triangle *> tris;
    OBJ *island = m_oh->getObject(path, shader, &tris, offset, color);
    m_world->addObject(island);
    m_world->addToMesh(tris);
    m_resetHalves.append(island->top);

#ifdef CUDA
    psm->addTriangles(&tris, island->top->getCenterPoint(), island->top->getRadius());
#endif

    return island;
}

void LevelTwo::addMarker(const QString& objPath, GLuint shader, const glm::vec3& offset, const QString& signPath, glm::vec4 color){

    QList<Triangle*> tris;
    OBJ* objMarker = m_oh->getObject(objPath, shader, &tris, offset, color);
    Marker* marker = new Marker(objMarker, glm::vec2(0.f, 0.f), glm::vec2(1.2,1.2), signPath);
    m_markers.append(marker);

    m_world->addObject(objMarker);
    m_world->addToMesh(tris);
}

QList<Light*> LevelTwo::makeLights(){

    QList<Light*> lights;
    QList<glm::vec3> positions;

    int counter = 0;
    positions.append(glm::vec3(0,5,0));
    positions.append(glm::vec3(-40,5,25));
    positions.append(glm::vec3(-40,50,25));
    positions.append(glm::vec3(-40,50,-10));
    positions.append(glm::vec3(-40,50.,-19));

    for(int i=0; i<positions.size(); i++){

        Light* light;

        light = new Light();
        light->id = counter++;
        light->type = POINTLIGHT;
        light->color = glm::vec3(.750, .750, 1.5f);  // rgb color
        light->posDir = positions.at(i);

        light->radius = 80.f;
        light->function = glm::vec3(1.0, .1, .01);
        light->animFunc = Light::NONE;

        lights.append(light);
    }

    return lights;
}


void LevelTwo::resetWorld(glm::vec3 playerPos)
{

    if (m_world)
    {
        delete m_world;
        m_world = NULL;
    }
    m_resetHalves.clear();

    //reset spiral
    m_spiral.clear();
    m_spiraling=false;
    m_spiralIndex=0;

    GLuint shader = m_parentApp->getShader(GEOMETRY);
//    GLuint shader = m_parentApp->getShader(DEFAULT);

    QList<Light*> lights = makeLights();

    ActionCamera *cam;
    cam = new ActionCamera();
    cam->setCenter(playerPos);

    GamePlayer *player = new GamePlayer(cam, playerPos);
    player->useSound(m_parentApp->getAudioObject());

    GeometricCollisionManager *gcm = new GeometricCollisionManager();
    vm = new VerletManager(cam);
    vm->windPow=0;

    m_world = new GameWorld();
    m_world->addManager(gcm);
    m_world->addManager(vm);
    m_world->setPlayer(player);

    m_world->setGravity(glm::vec3(0,-10,0));
    setCamera(cam);
    player->setMaxOffset(50); //zoom

    //Add all islands


#ifdef CUDA
    ParticleSystemManager *psm = new ParticleSystemManager(playerPos, GEOMETRY, shader);
    m_world->addManager(psm);
    vm->setParams(psm->getParams());

    //Add all islands
    addIsland(":/objects/MediumIsland.obj", shader, glm::vec3(0), glm::vec4(.5,.5,.5,0), psm);

    //Add all verlet entities
    addIsland(":/objects/MediumIsland.obj", shader, glm::vec3(-40,0,25), glm::vec4(.5,.5,.5,0), psm);
    addIsland(":/objects/testsmall.obj", shader, glm::vec3(-40,45,25), glm::vec4(.5,.5,.5,0), psm);
    addIsland(":/objects/MediumIsland.obj", shader, glm::vec3(-40,45,-16), glm::vec4(.5,.5,.5,0), psm);
#else

    //Add all islands
    addIsland(":/objects/MediumIsland.obj", shader, glm::vec3(0), glm::vec4(.5,.5,.5,0), NULL);

    //Add all verlet entities
    addIsland(":/objects/MediumIsland.obj", shader, glm::vec3(-40,0,25), glm::vec4(.5,.5,.5,0), NULL);
    addIsland(":/objects/testsmall.obj", shader, glm::vec3(-40,45,25), glm::vec4(.5,.5,.5,0), NULL);
    addIsland(":/objects/MediumIsland.obj", shader, glm::vec3(-40,45,-16), glm::vec4(.5,.5,.5,0), NULL);

#endif




    int row = 12;
    int column = 60;
    float yOffset = 1;
    float w = .8;

    TriangleMesh* curtain = new TriangleMesh(glm::vec2(column+1,row+1), w, glm::vec3(0,yOffset,0),vm,shader);

    float length = (column-.5)*w;
    float height = ((sqrt(3)/2.0) * w);
    float totalHeight =height*row;
    float radius = length/2.0/3.14;
    float angleIncrement = 360.0/float(column*2-1);

    float angle = 0;
    //top
    for(int i = 0; i<column+1; i++){
        if(i==0||i==column)
            angle = 0;
        else
            angle+=angleIncrement*2;
        float rad = angle*3.14/180.0;
        //top
        curtain->setPos(i, glm::vec3(cos(rad)*radius, 0+yOffset, sin(rad)*radius));
        curtain->createPin(i);
    }

    //bot
    int botOffset = (column+1)*row; //find last row
    angle = 0;
    for(int i = 0; i<column+1; i++){
        if(i==0||i==column)
            angle = 0;
        else if(i==1)
            angle+=angleIncrement;
        else
            angle+=angleIncrement*2;
        float rad = angle*3.14/180.0;
        curtain->setPos(i+botOffset, glm::vec3(cos(rad)*radius, totalHeight+yOffset, sin(rad)*radius));
        curtain->createPin(i+botOffset);
    }

    //sides
    int c = column+1;
    int r = row+1;
    for(int i = 0; i<r; i++){
        float rad = 0*3.14/180.0;
        curtain->setPos(i*c,glm::vec3(cos(rad)*radius, i*height+yOffset, sin(rad)*radius));
        curtain->createPin(i*c);
        curtain->setPos(i*c+c-1,glm::vec3(cos(rad)*radius, i*height+yOffset, sin(rad)*radius));
        curtain->createPin(i*c+c-1);
    }

    vm->addVerlet(curtain);

    //Add all verlet entities
    float rad = 55 * (PI/180.0);
    TriangleMesh* tri = new TriangleMesh(glm::vec2(8,80), .5, glm::vec3(-7*sin(rad),2,7*cos(rad)), vm, shader, 55);
    tri->createPin(79*8);
    tri->createPin(80*8-1);
    vm->addVerlet(tri);

    vm->addVerlet(new TriangleMesh(glm::vec2(6,25), .6, glm::vec3(-40,46,22), vm, shader,0,Y, TOP_CORNERS));    // final bridge

    //stairs
    int counter = lights.size();
    int numStairs = 10;
    int y = 5;
    for(int i=0; i<720; i+= 360/numStairs, y+=2){
        float rad = i * (PI/180.0);
        glm::vec3 pos = glm::vec3(-5*sin(rad),y,5*cos(rad))+glm::vec3(-40,0,25);
        TriangleMesh* t = new TriangleMesh(glm::vec2(8,30), .5, pos, vm, shader, i);
        float rad1 = (i+90) * (PI/180.0);


        // make a light at loc
        Light* light;

        light = new Light();
        light->id = counter++;
        light->type = POINTLIGHT;
        light->color = glm::vec3(.20, .740, .6f);  // rgb color
        light->posDir = pos + glm::vec3(0,2,0);

        light->radius = 40.f;
        light->function = glm::vec3(1.0, .1, .1);
        light->animFunc = Light::NONE;

        lights.append(light);

        // end making light


        t->setWindDirection(rad1);
        vm->addVerlet(t);
        this->m_spiral.push_back(t);
    }

    // wind trigger
    QList<Triangle*> tris;
    OBJ* objMarker = m_oh->getObject(":/objects/Bell.obj", shader, &tris, glm::vec3(-40,0,25), glm::vec4(1,1,0,.6));
    m_spiralSensor = new Marker(objMarker, glm::vec2(0.f, 0.f), glm::vec2(2,2), "");
    m_world->addObject(objMarker);
    m_world->addToMesh(tris);

    // end level bell
    QList<Triangle*> tris2;
    objMarker = m_oh->getObject(":/objects/Bell.obj", shader, &tris2, glm::vec3(-40,45.7,-19), glm::vec4(1,1,0,.6));
    m_levelChanger = new Marker(objMarker, glm::vec2(0.f, 0.f), glm::vec2(1.2,1.2), "");
    m_world->addObject(objMarker);
    m_world->addToMesh(tris2);

    m_world->setLights(lights);
    m_cursor = glm::scale(glm::mat4(), glm::vec3(.02f / cam->getAspectRatio(), .02f, .02f));
    m_cursor[3][2] = -.999f;

    m_drawCursor = true;
    m_deltas = glm::vec2(0);

    onResize(m_parentApp->getWidth(), m_parentApp->getHeight());

}

// update and render
void LevelTwo::onTick(float secs)
{

    m_world->onTick(secs, m_cursor[3][0], m_cursor[3][1]);

    if (m_world->getPlayer()->getPosition().y < -40)
    {
        Half *h = m_resetHalves.value(m_resetIndex);
        glm::vec2 c = h->getCenter();
        resetWorld(glm::vec3(c.x, h->getYLimits().y + 5.f, c.y));
    }

    glm::vec3 pos = m_world->getPlayer()->getPosition();
    for (int i = 0; i < m_resetHalves.size(); i++){
        Half* h = m_resetHalves[i];
        if(h->inHitBox(pos)&&i>m_resetIndex){
            m_resetIndex = i;
            break;
        }
    }
    if(this->m_spiralSensor->isInRange(pos,2.0))
        m_spiraling = true;
    if(m_spiraling){
        if(m_spiralCounter==0){
            m_spiralCounter = m_spiralDelay;
            if(m_spiralIndex<m_spiral.size())
                m_spiral[m_spiralIndex]->controlWind=true;
            m_spiralIndex++;
        }
        m_spiralCounter--;
    }

    glm::vec4 camPos = m_world->getPlayer()->getCamEye();
    vm->handleFrustumCulling(glm::vec3(camPos.x,camPos.y,camPos.z));
}


void LevelTwo::onRender(Graphics *g)
{
    // draw markers first
    foreach(Marker* marker, m_markers){
        if(marker->isInRange(m_world->getPlayer()->getPosition(), 1.8))
            marker->displayTexture(g);
    }

    g->setWorldColor(.2f,.2f,.2f);
    g->setColor(1,1,1,1,0);

    g->setTexture("grass.png", 5.f, 5.f);

    m_world->onDraw(g);

    g->setAllWhite(true);
    g->drawLine(glm::vec3(0, 0, -5), glm::vec3(0, 15, -5));
    g->setAllWhite(false);

    render2D(g);
}

void LevelTwo::render2D(Graphics *g)
{
    g->setGraphicsMode(DRAW2D);

    if (m_drawCursor)
    {
        g->setTexture("");
        g->setColor(1, 0, 0, 1, 0);
//        g->setAllWhite(true);
        g->drawSphere(m_cursor);
        g->setAllWhite(false);
    }
}

void LevelTwo::onMouseMoved(QMouseEvent *e, float deltaX, float deltaY, glm::vec3 pos)
{
    if (m_parentApp->isUsingLeapMotion())
    {
        deltaX *= 1.5f;
        deltaY *= 1.5f;
    }

    m_world->onMouseMoved(e, deltaX * 1000.f, deltaY * 1000.f);

    m_cursor[3][0] = pos.x;
    m_cursor[3][1] = pos.y;
}

void LevelTwo::onMouseDragged(QMouseEvent *e, float deltaX, float deltaY, glm::vec3 pos)
{
    if (m_parentApp->isUsingLeapMotion())
    {
        deltaX *= 1.5f;
        deltaY *= 1.5f;
    }

    m_world->onMouseMoved(e, deltaX * 1000.f, deltaY * 1000.f);

    m_cursor[3][0] = pos.x;
    m_cursor[3][1] = pos.y;
}

void LevelTwo::onKeyPressed(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Backspace)
    {
        m_parentApp->popScreens(1);
        return;
    }
    m_world->onKeyPressed(e);
}

void LevelTwo::onKeyReleased(QKeyEvent *e )
{
    if (e->key() == Qt::Key_P)
        m_parentApp->useLeapMotion(!m_parentApp->isUsingLeapMotion());

    m_world->onKeyReleased(e);

}

void LevelTwo::onResize(int w, int h)
{
    ScreenH::onResize(w, h);

    float inverseAspect = h * 1.f / w;

    glm::vec4 pos = m_cursor[3];
    m_cursor = glm::scale(glm::mat4(), glm::vec3(.02f * inverseAspect, .02f, 1));
    m_cursor[3] = pos;
}

void LevelTwo::onMousePressed(QMouseEvent *e)
{
    if (e->button() == Qt::MidButton)   //Qt::RightButton
        m_parentApp->setMouseDecoupled(false);

    m_world->onMousePressed(e);
}

void LevelTwo::onMouseReleased(QMouseEvent *e)
{
    if (e->button() == Qt::MidButton)  //Qt::RightButton
        m_parentApp->setMouseDecoupled(true);

    m_world->onMouseReleased(e);
}

// unused in game
void LevelTwo::onMouseWheel(QWheelEvent *e) {
    m_world->onMouseWheel(e);
}

