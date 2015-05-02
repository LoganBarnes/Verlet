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

#include "debugprinting.h"

TestLevelScreen::TestLevelScreen(Application *parent)
    : ScreenH(parent),
      m_world(NULL),
      m_oh(NULL),
      m_resetIndex(0)
{
    m_parentApp->setMouseDecoupled(true);
    m_parentApp->setLeapRightClick(GRAB);
    m_parentApp->setLeapLeftClick(PINCH);

    m_oh = new ObjectHandler();
    QList<Triangle *> tris;

    resetWorld(glm::vec3(0, 10, 0));
}


TestLevelScreen::~TestLevelScreen()
{
    delete m_world;
    delete m_oh; // m_level is deleted here
}


OBJ* TestLevelScreen::addIsland(const QString& path, GLuint shader, const glm::vec3& offset){
    QList<Triangle *> tris;
    OBJ *island = m_oh->getObject(path, shader, &tris, offset);
    m_world->addObject(island);
    m_world->addToMesh(tris);
    m_resetHalves.append(island->top);
    return island;
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
    QList<Light*> lights = lightParser.getLights(":/objects/island_lights.obj", glm::vec3(0));

//    Light *l = lights.value(0);
//    lights.clear();
//    lights.append(l);

    ActionCamera *cam;
    cam = new ActionCamera();
    cam->setCenter(playerPos);

    GamePlayer *player = new GamePlayer(cam, playerPos);
    player->useSound(m_parentApp->getAudioObject());

    GeometricCollisionManager *gcm = new GeometricCollisionManager();
    VerletManager *vm = new VerletManager(cam);

    QList<Triangle*> tris5;

    //MARKER OBJECTS:
    OBJ* objMarker1 = m_oh->getObject(":/objects/Stone.obj", shader, &tris5, glm::vec3(0));
    Marker* marker1 = new Marker(objMarker1, glm::vec2(0.f, 0.f), glm::vec2(1.2,1.2), "freezeSign.png");
    m_markers.append(marker1);
    //END MARKER OBJECTS

    m_world = new GameWorld();
    m_world->setLights(lights);
    m_world->addManager(gcm);
    m_world->addManager(vm);

    m_world->addObject(objMarker1);
    m_world->setPlayer(player);
    m_world->addToMesh(tris5);

    m_world->setGravity(glm::vec3(0,-10,0));
    setCamera(cam);
    player->setMaxOffset(50); //zoom

    //Add all islands
    OBJ* island1 = addIsland(":/objects/testsmall.obj",shader,glm::vec3(0));
    addIsland(":/objects/testsmall.obj", shader, glm::vec3(-20,0,0));
    addIsland(":/objects/testsmall.obj", shader, glm::vec3(-55,30,0));

    //Add all verlet entities
    vm->addVerlet(new TriangleMesh(glm::vec2(8,18), .6, glm::vec3(-5,0,-2.2), vm, shader,Z,true,HORIZONTAL_EDGE));
    vm->addVerlet(new TriangleMesh(glm::vec2(6,20), .6, glm::vec3(-27,0,4), vm, shader));
    vm->addVerlet(new TriangleMesh(glm::vec2(6,20), .6, glm::vec3(-32,4,4), vm, shader));
    vm->addVerlet(new TriangleMesh(glm::vec2(6,20), .6, glm::vec3(-37,8,4), vm, shader));
    vm->addVerlet(new TriangleMesh(glm::vec2(6,20), .6, glm::vec3(-42,12,4), vm, shader));
    vm->addVerlet(new TriangleMesh(glm::vec2(6,20), .6, glm::vec3(-47,16,4), vm, shader));
    vm->addVerlet(new TriangleMesh(glm::vec2(6,20), .6, glm::vec3(-52,19,4), vm, shader));
    vm->addVerlet(new TriangleMesh(glm::vec2(6,20), .6, glm::vec3(-60,19,4), vm, shader));
    vm->addVerlet(new TriangleMesh(glm::vec2(6,25), .6, glm::vec3(-74,18,-2), vm, shader,Z,true, ALL_CORNERS));
    vm->addVerlet(new TriangleMesh(glm::vec2(8,22), .3, glm::vec3(-88,-8,-2.2), vm, shader,Z,true));

    Grass* grass = new Grass(vm, shader);
    grass->createPatch(glm::vec2(0,0),6,island1);
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
    m_world->onTick(secs, m_cursor[3][0], m_cursor[3][1]);

    if (m_world->getPlayer()->getPosition().y < -40)
    {
        Half *h = m_resetHalves.value(m_resetIndex);
        glm::vec2 c = h->getCenter();
        resetWorld(glm::vec3(c.x, h->getYLimits().y + 5.f, c.y));
    }

    glm::vec3 pos = m_world->getPlayer()->getPosition();
    QList<OBJ*> objs = m_world->getObjs();
    for (int i = 0; i < objs.size(); i++){
        OBJ* o = objs[i];
        if(o->top->inHitBox(pos)&&i>m_resetIndex){
            m_resetIndex = i;
            break;
        }
    }
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
