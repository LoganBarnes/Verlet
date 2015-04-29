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


TestLevelScreen::TestLevelScreen(Application *parent)
    : Screen(parent)
{
    m_parentApp->setMouseDecoupled(true);

    GLuint shader = m_parentApp->getShader(DEFAULT);
    QList<Triangle *> tris;

    m_oh = new ObjectHandler();
    m_level = m_oh->getObject(":/objects/flat.obj", shader, &tris);
    //m_level = m_oh->getObject(":/objects/01.obj", shader, &tris);
    m_level->setTexture("level.png");
    //m_level->setTexture("grass.png");


    ActionCamera *cam;
    cam = new ActionCamera();
    glm::vec3 playerPos = glm::vec3(0, 3, 0);
//    glm::vec3 playerPos = glm::vec3(-7, 12, 18);
    cam->setCenter(playerPos);

    GamePlayer *player = new GamePlayer(cam, playerPos);
    player->useSound(m_parentApp->getAudioObject());

    GeometricCollisionManager *gcm = new GeometricCollisionManager();
    VerletManager *vm = new VerletManager(cam, m_parentApp->getShader(DEFAULT));

    Grass* grass = new Grass(vm, shader);
    grass->createPatch(glm::vec2(0,0),6,m_level);
    vm->addVerlet(grass);

    m_world = new GameWorld();
    m_world->addManager(gcm);
    m_world->setPlayer(player);
    m_world->addObject(m_level);
    m_world->addToMesh(tris);
    m_world->addManager(vm);
    m_world->setGravity(glm::vec3(0,-10,0));

    // uncomment to play sound at the origin
//    SoundTester *st = new SoundTester(glm::vec3());
//    st->setSound(m_parentApp->getAudioObject(), "dreams_of_home.wav", true);
//    st->playSound();
//    m_world->addMovableEntity(st);

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

    Light light;
    light.id = 0;
    light.type = DIRECTIONAL;
    light.posDir = glm::vec3(-1);
    light.color = glm::vec3(1);
    g->addLight(light);

    light.id = 1;
    light.type = DIRECTIONAL;
    light.posDir = glm::vec3(1,-1,1);
    light.color = glm::vec3(.2f);
    g->addLight(light);

//    g->setTexture("grass.png", 5.f, 5.f);

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
    if (e->key() == Qt::Key_L)
        m_parentApp->useLeapMotion(!m_parentApp->isUsingLeapMotion());

    m_world->onKeyReleased(e);

}

void TestLevelScreen::onResize(int w, int h)
{
    Screen::onResize(w, h);

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
