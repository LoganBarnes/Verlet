#include "gamemenu.h"
#include "application.h"
//#include "gamescreen.h"
#include "testlevelscreen.h"
#include "leveltwo.h"
#include "button.h"
#include "soundtester.h"

#define GLM_FORCE_RADIANS
#include <gtx/transform.hpp>

GameMenu::GameMenu(Application *parent)
    : ScreenH(parent),
      m_st(NULL)
{
    float aspect = m_camera->getAspectRatio();

    m_startButton = new Button();
    m_startButton->setCenter(0.f, 0.f);
    m_startButton->setSize(.3, aspect * .3);
    m_startButton->setImage("play.png");

    m_cursor = glm::scale(glm::mat4(), glm::vec3(.05, .05, .05));
    m_cursor[3][2] = -.999f;

    // uncomment to play sound at the origin
    m_st = new SoundTester(glm::vec3());
    m_st->setSound(m_parentApp->getAudioObject(), "dreams_of_home.wav", true);
    m_st->playSound();
}

GameMenu::~GameMenu()
{
    delete m_startButton;
    if (m_st)
        delete m_st;
}

// update and render
void GameMenu::onTick(float)
{
    m_parentApp->setMouseDecoupled(true);
}

void GameMenu::onRender(Graphics *g)
{
    m_parentApp->setUseCubeMap(false);
    g->setWorldColor(1, 1, 1);
    g->setColor(1, 1, 1, 1, 0);

    g->setGraphicsMode(DRAW2D);
    m_startButton->onDraw(g);

    g->setTexture("");
    g->setColor(1, 0, 0, 1, 0);
    g->drawSphere(m_cursor);

}

void GameMenu::onMouseMoved(QMouseEvent *, float, float, glm::vec3 pos)
{
    m_cursor[3][0] = pos.x;
    m_cursor[3][1] = pos.y;

    float c = .5f;
    m_startButton->setColor(c, c, c);
    if (m_startButton->contains(pos.x, pos.y))
        m_startButton->setColor(1, 1, 1);
}

void GameMenu::onMousePressed(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton && m_startButton->contains(m_cursor[3][0], m_cursor[3][1]))
    {
        m_parentApp->setMouseDecoupled(false);
//        m_parentApp->addScreen(new TestLevelScreen(m_parentApp));
        m_parentApp->addScreen(new LevelTwo(m_parentApp));
    }
}


void GameMenu::onResize(int w, int h)
{
    ScreenH::onResize(w, h);

    float aspect = w * 1.f / h;

    m_startButton->setSize(.3, aspect * .3);

    glm::vec4 pos = m_cursor[3];
    m_cursor = glm::scale(glm::mat4(), glm::vec3(.03f / aspect, .03f, 1));
    m_cursor[3] = pos;
}

void GameMenu::onKeyReleased(QKeyEvent *e)
{
    if (e->key() == Qt::Key_P)
    {
        m_parentApp->useLeapMotion(!m_parentApp->isUsingLeapMotion());
        m_parentApp->leapEnableKeyTapGesture();
    }
}

void GameMenu::onLeapKeyTap(glm::vec3)
{
    if (m_startButton->contains(m_cursor[3][0], m_cursor[3][1]))
    {
        m_parentApp->setMouseDecoupled(false);
//        m_parentApp->addScreen(new TestLevelScreen(m_parentApp));
        m_parentApp->addScreen(new LevelTwo(m_parentApp));
    }
}

// unused in menu
void GameMenu::onMouseReleased(QMouseEvent *) {}
void GameMenu::onMouseDragged(QMouseEvent *, float, float, glm::vec3) {}
void GameMenu::onMouseWheel(QWheelEvent *) {}
void GameMenu::onKeyPressed(QKeyEvent *) {}
