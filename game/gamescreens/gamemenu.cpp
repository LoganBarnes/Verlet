#include "gamemenu.h"
#include "application.h"
#include "gamescreen.h"

#define GLM_FORCE_RADIANS
#include <glm/gtx/transform.hpp>

#include "debugprinting.h"

GameMenu::GameMenu(Application *parent)
    : Screen(parent)
{
//    m_parentApp->useLeapMotion(true);

    float aspect = m_camera->getAspectRatio();

    m_startButton = new Button();
    m_startButton->setCenter(0.f, 0.f);
    m_startButton->setSize(.3, aspect * .3);
    m_startButton->setImage("play.png");

    m_cursor = glm::scale(glm::mat4(), glm::vec3(.05, .05, .05));
    m_cursor[3][2] = -.999f;
}

GameMenu::~GameMenu()
{
    delete m_startButton;
}

// update and render
void GameMenu::onTick(float)
{
    ///////////////commented leap motion stuff//////////////////////

//    if (!m_parentApp->isUsingLeapMotion())
//        return;

//    Leap::Frame frame = m_parentApp->getLeapFrame();
//    Leap::Vector pos = frame.hands().rightmost().palmPosition();

//    float newX = pos.x / 200.f;
//    if (newX < -1.f)
//        newX = -1.f;
//    else if (newX > 1.f)
//        newX = 1.f;
//    m_cursor[3][0] = newX;

//    float newY = pos.y / 200.f - 1.f;
//    if (newY < -1.f)
//        newY = -1.f;
//    else if (newY > 1.f)
//        newY = 1.f;
//    m_cursor[3][1] = newY;

//    float c = .5f;
//    m_startButton->setColor(c, c, c);
//    if (m_startButton->contains(newX, newY))
//    {
//        m_level = 1;
//        m_startButton->setColor(1, 1, 1);
//    }

//    if (frame.hands().count() > 0 && frame.fingers().count() == 0)
//        cout << "CLIIIIIIIIIIIIIIIIIIIIIICKEEEEEEEEEEEEEEEED" << endl;

//    if (frame.hands().count() > 0)
//        cout << frame.fingers().extended().count() << endl;
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

void GameMenu::onMouseMoved(QMouseEvent *, float deltaX, float deltaY)
{
    float newX = m_cursor[3][0] + deltaX * 0.005f;
    float newY = m_cursor[3][1] - deltaY * 0.005f;
    m_cursor[3][0] = glm::clamp(newX, -1.f, 1.f);
    m_cursor[3][1] = glm::clamp(newY, -1.f, 1.f);

    float c = .5f;
    m_startButton->setColor(c, c, c);
    if (m_startButton->contains(newX, newY))
        m_startButton->setColor(1, 1, 1);
}

void GameMenu::onMousePressed(QMouseEvent *e)
{
//    if (e->button() == Qt::LeftButton && m_startButton->contains(m_cursor[3][0], m_cursor[3][1]))
        m_parentApp->addScreen(new GameScreen(m_parentApp));
}


void GameMenu::onResize(int w, int h)
{
    Screen::onResize(w, h);

    float aspect = w * 1.f / h;

    m_startButton->setSize(.3, aspect * .3);

    glm::vec4 pos = m_cursor[3];
    m_cursor = glm::scale(glm::mat4(), glm::vec3(.03f / aspect, .03f, 1));
    m_cursor[3] = pos;
}


// unused in menu
void GameMenu::onMouseReleased(QMouseEvent *) {}
void GameMenu::onMouseDragged(QMouseEvent *, float, float) {}
void GameMenu::onMouseWheel(QWheelEvent *) {}
void GameMenu::onKeyPressed(QKeyEvent *) {}
void GameMenu::onKeyReleased(QKeyEvent *) {}
