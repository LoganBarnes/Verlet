#include "application.h"
#include "Leap.h"

Application::Application()
{
    m_currentScreen = NULL;

    // create graphics object
    m_g = new Graphics();
    m_leapController = NULL;

    m_decoupleKey = std::numeric_limits<int>::min();
    m_decoupleMouse = false;
    m_mouseDown = false;
    m_permanentDecouple = false;

    m_mousePos = glm::vec3(0);
}

Application::~Application()
{
    if (m_currentScreen)
        delete m_currentScreen;

    foreach(Screen *s, m_screens)
        delete s;

    delete m_g;

    if (m_leapController)
        delete m_leapController;
}

void Application::init(Screen *initScreen)
{
    m_currentScreen = initScreen;
    m_g->init();
}

void Application::addScreen(Screen *s)
{
    if (m_currentScreen)
        m_screens.append(m_currentScreen);

    m_currentScreen = s;
    m_currentScreen->onResize(m_width, m_height);
}

void Application::popScreens(int num)
{
    while (num-- > 0)
    {
        if (m_currentScreen)
            delete m_currentScreen;

        if (!m_screens.isEmpty())
            m_currentScreen = m_screens.takeLast();
        else
        {
            m_currentScreen = NULL;
            break;
        }
    }
}

// leap motion stuff for personal mac
bool Application::isUsingLeapMotion()
{
    return m_leapController != NULL;
}

void Application::useLeapMotion(bool useLeap)
{
    if (useLeap)
    {
        // already using leap
        if (m_leapController)
            return;

        // not using leap. Create a controller
        m_leapController = new Leap::Controller();
    }
    else // not using leap
    {
        // delete controller to stop the data flow
        if (m_leapController)
        {
            delete m_leapController;
            m_leapController = NULL;
        }
    }
}

Leap::Frame Application::getLeapFrame()
{
    return m_leapController->frame();
}

void Application::onTick(float secs)
{
    m_g->update();

    if (m_currentScreen)
        m_currentScreen->onTick(secs);

    if (!isUsingLeapMotion())
        return;

    handleLeapMouseEvents();
}

void Application::handleLeapMouseEvents()
{
    Leap::Frame frame = getLeapFrame();

    // nothing is detected
    if (frame.hands().count() == 0)
        return;

    Leap::Vector pos = frame.hands().rightmost().palmPosition();

    if (frame.fingers().extended().count() == 0)
    {
        if (!m_mouseDown)
        {
            QMouseEvent qme(QEvent::MouseButtonPress,
                            QPoint(0,0),
                            Qt::LeftButton,
                            Qt::NoButton,
                            Qt::NoModifier);
            this->onMousePressed(&qme);
        }
        m_mouseDown = true;
    }
    else
    {
        if (m_mouseDown)
        {
            QMouseEvent qme(QEvent::MouseButtonRelease,
                            QPoint(0,0),
                            Qt::LeftButton,
                            Qt::NoButton,
                            Qt::NoModifier);
            this->onMouseReleased(&qme);
        }
        m_mouseDown = false;
    }

    float deltaX = 0, deltaY = 0;

    if (m_decoupleMouse || m_permanentDecouple)
    {
        m_mousePos.x = pos.x / 200.f;
        m_mousePos.y = pos.y / 200.f - 1.f;
        m_mousePos = glm::clamp(m_mousePos, -1.f, 1.f);
        m_mousePos.z = 1;
    }
    else
    {
        deltaX = pos.x;
        deltaY = 200.f - pos.y;
        m_mousePos = glm::vec3(0);
    }

    if (m_mouseDown)
        m_currentScreen->onMouseDragged(NULL, deltaX, deltaY, m_mousePos);
    else
        m_currentScreen->onMouseMoved(NULL, deltaX, deltaY, m_mousePos);

//    if (frame.hands().count() > 0 && frame.fingers().count() == 0)
//        std::cout << "CLIIIIIIIIIIIIIIIIIIIIIICKEEEEEEEEEEEEEEEED" << std::endl;

//    if (frame.hands().count() > 0)
//        std::cout << frame.fingers().extended().count() << std::endl;
}

void Application::onRender()
{
    if (m_currentScreen)
    {
        if (m_g->cubeMapIsActive())
            m_g->drawCubeMap(m_currentScreen->getCamera());

        m_g->setGraphicsMode(DEFAULT);
        m_g->setWorldColor(0.f, 0.f, 0.f);
        m_g->setColor(0.f, 0.f, 0.f, 1.f, 0.f);

        m_g->setCamera(m_currentScreen->getCamera(), m_width, m_height);
        m_currentScreen->onRender(m_g);

    }
}

void Application::setUseCubeMap(bool use)
{
    m_g->useCubeMap(use);
}

void Application::onMousePressed(QMouseEvent *e)
{
    m_mouseDown = true;

    if (m_currentScreen)
        m_currentScreen->onMousePressed(e);
}

void Application::onMouseMoved(QMouseEvent *e, float deltaX, float deltaY)
{
    if (m_currentScreen)
    {
        if (m_decoupleMouse || m_permanentDecouple)
        {
            m_mousePos.x += deltaX * 0.005f;
            m_mousePos.y -= deltaY * 0.005f;
            deltaY = 0;
            deltaX = 0;
            m_mousePos = glm::clamp(m_mousePos, -1.f, 1.f);
            m_mousePos.z = 1;
        }
        else
            m_mousePos = glm::vec3(0);

        if (m_mouseDown)
            m_currentScreen->onMouseDragged(e, deltaX, deltaY, m_mousePos);
        else
            m_currentScreen->onMouseMoved(e, deltaX, deltaY, m_mousePos);
    }
}

void Application::onMouseReleased(QMouseEvent *e)
{
    m_mouseDown = false;

    if (m_currentScreen)
        m_currentScreen->onMouseReleased(e);
}

void Application::onMouseDragged(QMouseEvent *e, float deltaX, float deltaY, glm::vec3 pos)
{
    if (m_currentScreen)
        m_currentScreen->onMouseDragged(e, deltaX, deltaY, pos);
}

void Application::onMouseWheel(QWheelEvent *e)
{
    if (m_currentScreen)
        m_currentScreen->onMouseWheel(e);
}

void Application::onKeyPressed(QKeyEvent *e)
{
    if (e->key() == m_decoupleKey)
        m_decoupleMouse = true;

    if (m_currentScreen)
        m_currentScreen->onKeyPressed(e);
}

void Application::onKeyReleased(QKeyEvent *e)
{
    if (e->key() == m_decoupleKey)
        m_decoupleMouse = false;

    if (m_currentScreen)
        m_currentScreen->onKeyReleased(e);
}


void Application::onResize(int w, int h)
{
    if (m_currentScreen)
        m_currentScreen->onResize(w, h);

    m_width = w;
    m_height = h;
}

GLuint Application::getShader(GraphicsMode gm)
{
    return m_g->setGraphicsMode(gm);
}

void Application::setMouseDecoupleKey(int key)
{
    m_decoupleKey = key;
}

int Application::getMouseDecoupleKey()
{
    return m_decoupleKey;
}

void Application::setMouseDecoupled(bool decoupled)
{
    m_permanentDecouple = decoupled;
}

bool Application::isMouseDecoupled()
{
    return m_permanentDecouple;
}


