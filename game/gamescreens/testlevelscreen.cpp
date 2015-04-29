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

#include "debugprinting.h"

TestLevelScreen::TestLevelScreen(Application *parent)
    : ScreenH(parent)
{
    m_parentApp->setMouseDecoupled(true);
    m_parentApp->setLeapRightClick(GRAB);
    m_parentApp->setLeapLeftClick(PINCH);
//    m_parentApp->setMouseDecoupleKey(Qt::Key_Shift);

    GLuint shader = m_parentApp->getShader(GEOMETRY);
//    GLuint shader = m_parentApp->getShader(DEFAULT);
    QList<Triangle *> tris;
    QList<Triangle *> tris2;
    QList<Triangle *> tris3;
    QList<Triangle *> tris4;
    QList<Triangle *> tris5;
    QList<Triangle *> tris6;

    m_oh = new ObjectHandler();

    m_level = m_oh->getObject(":/objects/Level1a.obj", shader, &tris);

    //m_level->setTexture("01.png");
    m_level->setTexture("grass.png");

    // make an object handler for the lights and parse them in from an obj
    // save into a list of lights and send to the world
    LightParser lightParser;
    QList<Light*> lights = lightParser.getLights(":/objects/Level1Lights.obj");

//    cout << "length: " << lights.size() << endl;
//    foreach (Light *l, lights) {
//        cout << l->id << endl;
//    }

    ActionCamera *cam;
    cam = new ActionCamera();
        glm::vec3 playerPos = glm::vec3(0, 10, 0);
    cam->setCenter(playerPos);

    GamePlayer *player = new GamePlayer(cam, playerPos);
    player->useSound(m_parentApp->getAudioObject());

    GeometricCollisionManager *gcm = new GeometricCollisionManager();
    VerletManager *vm = new VerletManager(cam, shader);

    Grass* grass = new Grass(vm, shader);
    grass->createPatch(glm::vec2(0,0),6,m_level);
    vm->addVerlet(grass);

    // stairs

    TriangleMesh* tri2 = new TriangleMesh(glm::vec2(12,75), .3, glm::vec3(-5,1,0), vm, shader);
    tri2->createPin(0);
    tri2->createPin(11);
    vm->addVerlet(tri2);

    TriangleMesh* tri3 = new TriangleMesh(glm::vec2(12,35), .3, glm::vec3(-27,0,4), vm, shader);
    tri3->createPin(0);
    tri3->createPin(11);
    vm->addVerlet(tri3);

    TriangleMesh* tri4 = new TriangleMesh(glm::vec2(12,35), .3, glm::vec3(-32,4,4), vm, shader);
    tri4->createPin(0);
    tri4->createPin(11);
    vm->addVerlet(tri4);

    TriangleMesh* tri5 = new TriangleMesh(glm::vec2(12,35), .3, glm::vec3(-37,8,4), vm, shader);
    tri5->createPin(0);
    tri5->createPin(11);
    vm->addVerlet(tri5);

    TriangleMesh* tri6 = new TriangleMesh(glm::vec2(12,35), .3, glm::vec3(-42,12,4), vm, shader);
    tri6->createPin(0);
    tri6->createPin(11);
    vm->addVerlet(tri6);

    TriangleMesh* tri7 = new TriangleMesh(glm::vec2(12,35), .3, glm::vec3(-47,16,4), vm, shader);
    tri7->createPin(0);
    tri7->createPin(11);
    vm->addVerlet(tri7);

    TriangleMesh* tri8 = new TriangleMesh(glm::vec2(12,35), .3, glm::vec3(-52,19,4), vm, shader);
    tri8->createPin(0);
    tri8->createPin(11);
    vm->addVerlet(tri8);

    TriangleMesh* tri9 = new TriangleMesh(glm::vec2(12,35), .3, glm::vec3(-60,19,4), vm, shader);
    tri9->createPin(0);
    tri9->createPin(11);
    vm->addVerlet(tri9);

    m_world = new GameWorld();
    m_world->setLights(lights);
    m_world->addManager(gcm);
    m_world->addManager(vm);
    m_world->addObject(m_level);
    m_world->addObject(m_oh->getObject(":/objects/Level1b.obj", shader, &tris2));
    m_world->addObject(m_oh->getObject(":/objects/Level1c.obj", shader, &tris3));
    m_world->addObject(m_oh->getObject(":/objects/Level1d.obj", shader, &tris4));
    m_world->addObject(m_oh->getObject(":/objects/Level1e.obj", shader, &tris5));
    m_world->addObject(m_oh->getObject(":/objects/Level1f.obj", shader, &tris6));

    m_world->setPlayer(player);
    m_world->addToMesh(tris);
    m_world->addToMesh(tris2);
    m_world->addToMesh(tris3);
    m_world->addToMesh(tris4);
    m_world->addToMesh(tris5);
    m_world->addToMesh(tris6);

    m_world->setGravity(glm::vec3(0,-10,0));

    // uncomment to play sound at the origin
    SoundTester *st = new SoundTester(glm::vec3());
    st->setSound(m_parentApp->getAudioObject(), "dreams_of_home.wav", true);
    st->playSound();
    m_world->addMovableEntity(st);

    setCamera(cam);

    m_cursor = glm::scale(glm::mat4(), glm::vec3(.02f / cam->getAspectRatio(), .02f, .02f));
    m_cursor[3][2] = -.999f;

    m_drawCursor = true;
    m_deltas = glm::vec2(0);
}


TestLevelScreen::~TestLevelScreen()
{
    delete m_world;
    delete m_oh; // m_level is deleted here
}

// update and render
void TestLevelScreen::onTick(float secs)
{
    m_world->onTick(secs, m_cursor[3][0], m_cursor[3][1]);
}


void TestLevelScreen::onRender(Graphics *g)
{
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
        g->setAllWhite(true);
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
    if (e->button() == Qt::RightButton)
        m_parentApp->setMouseDecoupled(false);

    m_world->onMousePressed(e);
}

void TestLevelScreen::onMouseReleased(QMouseEvent *e)
{
    if (e->button() == Qt::RightButton)
        m_parentApp->setMouseDecoupled(true);

    m_world->onMouseReleased(e);
}

// unused in game
void TestLevelScreen::onMouseWheel(QWheelEvent *) {}
