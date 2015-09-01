#include "testlevelscreen.h"
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
#include "leveltwo.h"
#ifdef CUDA
#include "particlesystemmanager.h"
#endif

#include "debugprinting.h"

TestLevelScreen::TestLevelScreen(Application *parent)
    : ScreenH(parent),
      m_world(NULL),
      m_oh(NULL),
      m_resetIndex(0),
      m_psmIndex(-1)
{
    m_parentApp->setMouseDecoupled(true);
    m_parentApp->setLeapRightClick(GRAB);
    m_parentApp->setLeapLeftClick(PINCH);

    m_oh = new ObjectHandler();

    resetWorld(glm::vec3(0, 10, 0));
    m_parentApp->playMusic("darkness.wav");
}


TestLevelScreen::~TestLevelScreen()
{
    delete m_world;
    delete m_oh; // m_level is deleted here
}


OBJ* TestLevelScreen::addIsland(const QString& path, GLuint shader, const glm::vec3& offset, glm::vec4 color, ParticleSystemManager *psm){
    QList<Triangle *> tris;
    OBJ *island = m_oh->getObject(path, shader, &tris, offset, color);
    m_world->addObject(island);
    m_world->addToMesh(tris);
    //m_world->m_islands+=island;
    m_resetHalves.append(island->top);

#ifdef CUDA
    psm->addTriangles(&tris, island->top->getCenterPoint(), island->top->getRadius());
#endif
    return island;
}

void TestLevelScreen::addMarker(const QString& objPath, GLuint shader, const glm::vec3& offset, const QString& signPath, glm::vec4 color){

    QList<Triangle*> tris;
    OBJ* objMarker = m_oh->getObject(objPath, shader, &tris, offset, color);
    Marker* marker = new Marker(objMarker, glm::vec2(0.f, 0.f), glm::vec2(1.2,1.2), signPath);
    m_markers.append(marker);

    m_world->addObject(objMarker);
    m_world->addToMesh(tris);
}

QList<Light*> TestLevelScreen::makeLights(){

    QList<Light*> lights;

    int counter = 0;

    //hardcoded lights
    Light* light;
    light = new Light();
    light->id = counter++;
    light->type = POINTLIGHT;
    light->color = glm::vec3(.750, .750, 1.5f);  // rgb color
    light->posDir = glm::vec3(0,6,0);

    light->radius = 80.f;
    light->function = glm::vec3(1.0, .1, .01);
    light->animFunc = Light::CIRCLE;
    light->animationPeriod = 8;
    light->center = light->posDir;

    lights.append(light);
    //end


    // Custom lights
    QList<glm::vec3> positions;
    positions.append(glm::vec3(0,3,0));
    positions.append(glm::vec3(15,2,2));
//    positions.append(glm::vec3(-10,5,0));
    positions.append(glm::vec3(-25,-15,0));
    positions.append(glm::vec3(-50,5,0));
    positions.append(glm::vec3(0,45,0));
    positions.append(glm::vec3(-70,0,-40));         //island bottom of stairs
    positions.append(glm::vec3(-130,29,-40));       //island top of stairs

    positions.append(glm::vec3(-170,29,-40));       //furthest last island

    positions.append(glm::vec3(-150,7.,-42.5));   //bell
    positions.append(glm::vec3(-150,20.,-42.5));


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

    positions.clear();

    positions.append(glm::vec3(-9,10,-39));       //on stairs
    positions.append(glm::vec3(-92,2,-39));
    positions.append(glm::vec3(-114,18,-39));
    positions.append(glm::vec3(-97,6,-39));
    positions.append(glm::vec3(-107,14,-39));
    positions.append(glm::vec3(-119,22,-39));
    positions.append(glm::vec3(-87,-1,-39));
    positions.append(glm::vec3(-82,-4,-39));

    for(int i=0; i<positions.size(); i++){

        Light* light;
        light = new Light();
        light->id = counter++;
        light->type = POINTLIGHT;
        light->color = glm::vec3(.750, .750, 1.5f);  // rgb color
        light->posDir = positions.at(i);

        light->radius = 80.f;
        light->function = glm::vec3(1.0, .1, .01);
        light->animFunc = Light::ZSINE;
        light->animationPeriod = 5.0;
        light->center = light->posDir;

        lights.append(light);
    }


    return lights;

}

void TestLevelScreen::resetWorld(glm::vec3 playerPos)
{
    if (m_world)
    {
        delete m_world;
        m_world = NULL;
    }
    m_resetHalves.clear();

    GLuint shader = m_parentApp->getShader(GEOMETRY);
//    GLuint shader = m_parentApp->getShader(DEFAULT);

    // make an object handler for the lights and parse them in from an obj
    // save into a list of lights and send to the world
    LightParser lightParser;
    QList<Light*> lights = makeLights();

    ActionCamera *cam;
    cam = new ActionCamera();
    cam->setCenter(playerPos);

    GamePlayer *player = new GamePlayer(cam, playerPos);
    player->useSound(m_parentApp->getAudioObject());

    GeometricCollisionManager *gcm = new GeometricCollisionManager();
    vm = new VerletManager(cam);

    m_world = new GameWorld();
    m_world->setLights(lights);
    m_world->addManager(gcm);
    m_world->addManager(vm);
    m_world->setPlayer(player);

    //MARKER OBJECTS:
    addMarker(":/objects/LargeStone.obj", shader, glm::vec3(0), "basicsign.png", glm::vec4(.5,.5,.5,0));
    addMarker(":/objects/MediumStone.obj", shader, glm::vec3(-47,-.2,1), "freezesign.png", glm::vec4(.5,.5,.5,0));
    addMarker(":/objects/MediumStone.obj", shader, glm::vec3(-67,-4.8,-52), "windsign.png", glm::vec4(.5,.5,.5,0));
    addMarker(":/objects/MediumStone.obj", shader, glm::vec3(-130,24.8,-40), "tearsign.png", glm::vec4(.5,.5,.5,0));

    // create end of level marker
    QList<Triangle*> tris;
    OBJ* objMarker = m_oh->getObject(":/objects/Bell.obj", shader, &tris, glm::vec3(-150,5.1,-42.8), glm::vec4(1,1,0,.6));
    m_levelChanger = new Marker(objMarker, glm::vec2(0.f, 0.f), glm::vec2(1.2,1.2), "");
    m_world->addObject(objMarker);
    m_world->addToMesh(tris);

    m_world->setGravity(glm::vec3(0,-10,0));
    vm->wind = glm::vec3(0);
    setCamera(cam);
    player->setMaxOffset(50); //zoom


#ifdef CUDA
    ParticleSystemManager *psm = new ParticleSystemManager(playerPos, GEOMETRY, shader);
    m_psmIndex = m_world->addManager(psm);
    vm->setParams(psm->getParams());

    //Add all islands
    OBJ* island1 = addIsland(":/objects/LargeIsland.obj",shader,glm::vec3(0), glm::vec4(.5,.5,.5,0), psm);
    addIsland(":/objects/MediumIsland.obj", shader, glm::vec3(-48,0,0), glm::vec4(.5,.5,.5,0), psm);
    addIsland(":/objects/MediumIsland.obj", shader, glm::vec3(-70,-5,-40), glm::vec4(.5,.5,.5,0), psm);
    addIsland(":/objects/MediumIsland.obj", shader, glm::vec3(-130,25,-40), glm::vec4(.5,.5,.5,0), psm);
    addIsland(":/objects/MediumIsland.obj", shader, glm::vec3(-170,25,-40), glm::vec4(.5,.5,.5,0), psm);
    addIsland(":/objects/testsmall.obj", shader, glm::vec3(-150,5,-40), glm::vec4(.5,.5,.5,0), psm);
#else
    OBJ* island1 = addIsland(":/objects/LargeIsland.obj",shader,glm::vec3(0), glm::vec4(.5,.5,.5,0));
    addIsland(":/objects/MediumIsland.obj", shader, glm::vec3(-48,0,0), glm::vec4(.5,.5,.5,0));
    addIsland(":/objects/MediumIsland.obj", shader, glm::vec3(-70,-5,-40), glm::vec4(.5,.5,.5,0));
    addIsland(":/objects/MediumIsland.obj", shader, glm::vec3(-130,25,-40), glm::vec4(.5,.5,.5,0));
    addIsland(":/objects/MediumIsland.obj", shader, glm::vec3(-170,25,-40), glm::vec4(.5,.5,.5,0));
    addIsland(":/objects/testsmall.obj", shader, glm::vec3(-150,5,-40), glm::vec4(.5,.5,.5,0));
#endif

    //Add all verlet entities

    //ribbons
    vm->addVerlet(new TriangleMesh(glm::vec2(2,14), 1.4, glm::vec3(-2,15,0), vm, shader,60,Y,TOP_EDGE));
    vm->addVerlet(new TriangleMesh(glm::vec2(2,14), 1.4, glm::vec3(-1,15,1), vm, shader,30,Y,TOP_EDGE));
    vm->addVerlet(new TriangleMesh(glm::vec2(2,14), 1.4, glm::vec3(2,15,0), vm, shader,300,Y,TOP_EDGE));
    vm->addVerlet(new TriangleMesh(glm::vec2(2,14), 1.4, glm::vec3(1,15,1), vm, shader,330,Y,TOP_EDGE));

    //vm->addVerlet(new TriangleMesh(glm::vec2(2,12), 1.5, glm::vec3(-2,10,8), vm, shader,270,Y,TOP_EDGE));


    vm->addVerlet(new TriangleMesh(glm::vec2(8,58), .6, glm::vec3(-11,2.2,1.2), vm, shader,90,Y,HORIZONTAL_EDGE));
    vm->addVerlet(new TriangleMesh(glm::vec2(6,40), .6, glm::vec3(-53,1.2,-8), vm, shader,150));

    // stairs
    vm->addVerlet(new TriangleMesh(glm::vec2(6,20), .6, glm::vec3(-82,-5,-45), vm, shader,Y));
    vm->addVerlet(new TriangleMesh(glm::vec2(6,20), .6, glm::vec3(-87,-1,-45), vm, shader,Y));
    vm->addVerlet(new TriangleMesh(glm::vec2(6,20), .6, glm::vec3(-92,3,-45), vm, shader,Y));
    vm->addVerlet(new TriangleMesh(glm::vec2(6,20), .6, glm::vec3(-97,7,-45), vm, shader,Y));
    vm->addVerlet(new TriangleMesh(glm::vec2(6,20), .6, glm::vec3(-102,11,-45), vm, shader,Y));
    vm->addVerlet(new TriangleMesh(glm::vec2(6,20), .6, glm::vec3(-107,15,-45), vm, shader,Y));
    vm->addVerlet(new TriangleMesh(glm::vec2(6,20), .6, glm::vec3(-112,19,-45), vm, shader,Y));
    vm->addVerlet(new TriangleMesh(glm::vec2(6,20), .6, glm::vec3(-117,23,-45), vm, shader,Y));

    vm->addVerlet(new TriangleMesh(glm::vec2(14,25), 1.2, glm::vec3(-137,27,-32), vm, shader,90,Y, ALL_CORNERS));

    Grass* grass = new Grass(vm, shader);
    grass->createPatch(glm::vec2(0,5),5,island1);
    vm->addVerlet(grass);

    m_cursor = glm::scale(glm::mat4(), glm::vec3(.02f / cam->getAspectRatio(), .02f, .02f));
    m_cursor[3][2] = -.999f;

    m_drawCursor = true;
    m_deltas = glm::vec2(0);

    onResize(m_parentApp->getWidth(), m_parentApp->getHeight());

}

// update and render
void TestLevelScreen::onTick(float secs)
{
    if(m_levelChanger->isInRange(m_world->getPlayer()->getPosition(), 2.0))
    {
        if (m_psmIndex > -1)
        {
            Manager *m = m_world->getManager(m_psmIndex);
            delete m;
            m_psmIndex = -1;
        }
        m_parentApp->addScreen(new LevelTwo(m_parentApp));
        return;
    }

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

    glm::vec4 camPos = m_world->getPlayer()->getCamEye();
    vm->handleFrustumCulling(glm::vec3(camPos.x,camPos.y,camPos.z));

}

void TestLevelScreen::onRender(Graphics *g)
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

void TestLevelScreen::render2D(Graphics *g)
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

void TestLevelScreen::onMouseMoved(QMouseEvent *e, float deltaX, float deltaY, glm::vec3 pos)
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

void TestLevelScreen::onMouseDragged(QMouseEvent *e, float deltaX, float deltaY, glm::vec3 pos)
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

void TestLevelScreen::onKeyPressed(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Backspace)
    {
        m_parentApp->popScreens(1);
        return;
    }
    m_world->onKeyPressed(e);
}

void TestLevelScreen::onKeyReleased(QKeyEvent *e )
{
    if (e->key() == Qt::Key_P)
        m_parentApp->useLeapMotion(!m_parentApp->isUsingLeapMotion());

    m_world->onKeyReleased(e);

}

void TestLevelScreen::onResize(int w, int h)
{
    ScreenH::onResize(w, h);

    float inverseAspect = h * 1.f / w;

    glm::vec4 pos = m_cursor[3];
    m_cursor = glm::scale(glm::mat4(), glm::vec3(.02f * inverseAspect, .02f, 1));
    m_cursor[3] = pos;
}

void TestLevelScreen::onMousePressed(QMouseEvent *e)
{
    if (e->button() == Qt::MidButton)   //Qt::RightButton
        m_parentApp->setMouseDecoupled(false);

    m_world->onMousePressed(e);
}

void TestLevelScreen::onMouseReleased(QMouseEvent *e)
{
    if (e->button() == Qt::MidButton)  //Qt::RightButton
        m_parentApp->setMouseDecoupled(true);

    m_world->onMouseReleased(e);
}

// unused in game
void TestLevelScreen::onMouseWheel(QWheelEvent *e) {
    m_world->onMouseWheel(e);
}
