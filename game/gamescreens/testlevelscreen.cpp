#include "testlevelscreen.h"
#include "application.h"
#include "gameworld.h"
#include "objecthandler.h"
#include "obj.h"
#include "actioncamera.h"
#include "gameplayer.h"
#include "geometriccollisionmanager.h"
#include "verletmanager.h"
#include "ray.h"
#include "audio.h"
#include "soundtester.h"


#include "debugprinting.h"

TestLevelScreen::TestLevelScreen(Application *parent)
    : Screen(parent)
{
    m_parentApp->setMouseDecoupled(true);
//    m_parentApp->setMouseDecoupleKey(Qt::Key_Shift);

    GLuint shader = m_parentApp->getShader(GEOMETRY);
    QList<Triangle *> tris;

    m_oh = new ObjectHandler();
    m_level = m_oh->getObject(":/objects/testsmall.obj", shader, &tris);

    ActionCamera *cam;
    cam = new ActionCamera();
    glm::vec3 playerPos = glm::vec3(0, 10, 0);
//    glm::vec3 playerPos = glm::vec3(-7, 12, 18);
    cam->setCenter(playerPos);

    GamePlayer *player = new GamePlayer(cam, playerPos);
    player->useSound(m_parentApp->getAudioObject());

    GeometricCollisionManager *gcm = new GeometricCollisionManager();
    vm = new VerletManager(cam, m_parentApp->getShader(GEOMETRY));

    m_world = new GameWorld();
    m_world->addManager(gcm);
    m_world->addManager(vm);
    m_world->setPlayer(player);
    m_world->addToMesh(tris);
    m_world->setGravity(glm::vec3(0,-10,0));

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
    vm->setWind(windDirection);

    m_world->onTick(secs, m_cursor[3][0], m_cursor[3][1]);
//    vm->rayTrace(m_cursor[3][0], m_cursor[3][1]);

    if(dragMode){
        glm::vec3 point = draggedVerlet->getPoint(draggedPoint);
        glm::vec4 n = this->getCamera()->getLook();
        n*=-1;

        float t = vm->m_ray->hitPlane(point,glm::vec3(n));
        draggedMouse = vm->m_ray->getPoint(t);

        interpolate = draggedMouse;
        //interpolate = Vector3::lerp(interpolate, draggedMouse, 1 - powf(0.01, seconds));
    }
}


void TestLevelScreen::onRender(Graphics *g)
{
    g->setWorldColor(.2f,.2f,.2f);
    g->setColor(1,1,1,1,0);

    g->setTexture("grass.png", 5.f, 5.f);

//    m_level->draw(glm::mat4());
    m_world->onDraw(g, m_level, vm);
//    vm->onDraw(g);

    //for dragging
    if(dragMode){
        g->setColor(1, 1, 1, 1, 0);
        glm::mat4 trans = glm::translate(glm::mat4(), draggedVerlet->getPoint(draggedPoint));
        trans *= glm::scale(glm::mat4(), glm::vec3(.2,.2,.2));
        g->drawSphere(trans);
    }

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
    if(dragMode)
        draggedVerlet->setPos(draggedPoint,interpolate);

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
    if(dragMode)
        draggedVerlet->setPos(draggedPoint,interpolate);

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

    //testing verlet functions
    if(e->key() == Qt::Key_F) vm->enableSolve();
    if(e->key() == Qt::Key_Down) windDirection = glm::vec3(0,0,1);
    if(e->key() == Qt::Key_Up) windDirection = glm::vec3(0,0,-1);
    if(e->key() == Qt::Key_Left) windDirection = glm::vec3(-1,0,0);
    if(e->key() == Qt::Key_Right) windDirection = glm::vec3(1,0,0);

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
    //dragging
    if(e->button() == Qt::LeftButton&& vm->m_curV>-1){
        dragMode = true;
        draggedPoint = vm->m_curI;
        draggedVerlet = vm->getVerlet(vm->m_curV);
        interpolate = draggedVerlet->getPoint(draggedPoint);
    }

    if (e->button() == Qt::RightButton)
        m_parentApp->setMouseDecoupled(false);
}

void TestLevelScreen::onMouseReleased(QMouseEvent *e)
{
    if(e->button() == Qt::LeftButton)
        dragMode = false;

    if (e->button() == Qt::RightButton)
        m_parentApp->setMouseDecoupled(true);
}

// unused in game
void TestLevelScreen::onMouseWheel(QWheelEvent *) {}
