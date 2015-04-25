#include "application.h"
#include "audio.h"

Application::Application()
{

#ifdef LEAP
    m_leapController = NULL;
#endif

    m_currentScreen = NULL;

    // create graphics object
    m_g = new Graphics();
    m_a = new Audio();

    m_decoupleKey = std::numeric_limits<int>::min();
    m_decoupleMouse = false;
    m_mouseDown = false;
    m_permanentDecouple = false;

    m_mousePos = glm::vec3(0);
    m_prevPos = glm::vec3(0);
}

Application::~Application()
{
    if (m_currentScreen)
        delete m_currentScreen;

    foreach(Screen *s, m_screens)
        delete s;

    delete m_g;
    delete m_a;

#ifdef LEAP
    if (m_leapController)
        delete m_leapController;
#endif

}

void Application::init(Screen *initScreen)
{
    m_currentScreen = initScreen;
    m_g->init();
    m_a->initAudio();
}

// Have the graphics object initialize fbos for lighting
void Application::resetFBOs(int width, int height){
    m_g->loadDeferredLightFBOs(width, height);
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
#ifdef LEAP
    return m_leapController != NULL;
#else
    return false;
#endif

}

void Application::useLeapMotion(bool useLeap)
{
    if (useLeap)
    {
#ifdef LEAP
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
#endif
    }
}

void Application::leapEnableKeyTapGesture()
{
#ifdef LEAP
    if (m_leapController)
        m_leapController->enableGesture(Leap::Gesture::TYPE_KEY_TAP);
#endif

}

void Application::onTick(float secs)
{
    // prevent massive updates when the game is slow.
    secs = glm::min(secs, .07f);
    m_g->update();

#ifdef LEAP
    if (isUsingLeapMotion())
        handleLeapMouseEvents();
#endif

    if (m_currentScreen)
        m_currentScreen->onTick(secs);


}

#ifdef LEAP
void Application::handleLeapMouseEvents()
{
    Leap::Frame frame = m_leapController->frame();

    // nothing is detected
    if (frame.hands().count() == 0)
    {
        m_previousLeapFrame = frame;
        return;
    }

    Leap::Vector pos = frame.hands().rightmost().palmPosition();

    if (m_previousLeapFrame.hands().count() == 0)
    {
        m_prevPos = glm::vec3(pos.x, pos.y - 200.f, 200.f) * 0.005f;
        m_prevPos = glm::clamp(m_prevPos, -1.f, 1.f);
    }

//    if (frame.fingers().extended().count() == 0)
//    {
//        if (!m_mouseDown)
//        {
//            QMouseEvent qme(QEvent::MouseButtonPress,
//                            QPoint(0,0),
//                            Qt::LeftButton,
//                            Qt::NoButton,
//                            Qt::NoModifier);
//            this->onMousePressed(&qme);
//        }
//        m_mouseDown = true;
//    }
//    else
//    {
//        if (m_mouseDown)
//        {
//            QMouseEvent qme(QEvent::MouseButtonRelease,
//                            QPoint(0,0),
//                            Qt::LeftButton,
//                            Qt::NoButton,
//                            Qt::NoModifier);
//            this->onMouseReleased(&qme);
//        }
//        m_mouseDown = false;
//    }

    float deltaX = 0, deltaY = 0;

    m_mousePos = glm::vec3(pos.x, pos.y - 200.f, 200.f) * 0.005f;
    m_mousePos = glm::clamp(m_mousePos, -1.f, 1.f);

    if (!m_decoupleMouse && !m_permanentDecouple)
    {
        deltaX = m_mousePos.x - m_prevPos.x;
        deltaY = m_prevPos.y - m_mousePos.y;
        m_prevPos = m_mousePos;
        m_mousePos = glm::vec3(0);
    }
    else
        m_prevPos = m_mousePos;


    foreach (Leap::Gesture gesture, frame.gestures())
    {
        switch (gesture.type()) {
            case Leap::Gesture::TYPE_CIRCLE:
                //Handle circle gestures
                break;
            case Leap::Gesture::TYPE_KEY_TAP:
                m_currentScreen->onLeapKeyTap(m_mousePos);
                break;
            case Leap::Gesture::TYPE_SCREEN_TAP:
                //Handle screen tap gestures
                break;
            case Leap::Gesture::TYPE_SWIPE:
                //Handle swipe gestures
                break;
            default:
                //Handle unrecognized gestures
                break;
        }
    }

    if (m_mouseDown)
        m_currentScreen->onMouseDragged(NULL, deltaX, deltaY, m_mousePos);
    else
        m_currentScreen->onMouseMoved(NULL, deltaX, deltaY, m_mousePos);

    m_previousLeapFrame = frame;
}
#endif

void Application::onRender()
{
    if (m_currentScreen)
    {
        if (m_g->cubeMapIsActive())
            m_g->drawCubeMap(m_currentScreen->getCamera());

//        m_g->setGraphicsMode(DEFAULT);
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
        if (!isUsingLeapMotion() && (m_decoupleMouse || m_permanentDecouple))
        {
            m_mousePos.x += deltaX;
            m_mousePos.y -= deltaY;
            deltaY = 0;
            deltaX = 0;
            m_mousePos = glm::clamp(m_mousePos, -1.f, 1.f);
            m_mousePos.z = 1;
        }
//        else
//            m_mousePos = glm::vec3(0);

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

Audio *Application::getAudioObject()
{
    return m_a;
}


