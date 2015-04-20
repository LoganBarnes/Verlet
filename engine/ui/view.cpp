#include "view.h"
#include <QApplication>
#include <QKeyEvent>
#include "gamemenu.h"

#include <iostream>

View::View(QGLFormat format, QWidget *parent)
    : QGLWidget(format, parent),
      m_fpsInit(false),
      freq(200),
      counter(0),
      totalfps(0.f)
{
    // View needs all mouse move events, not just mouse drag events
    setMouseTracking(true);

    // Hide the cursor since this is a fullscreen app
    setCursor(Qt::BlankCursor);

    // View needs keyboard focus
    setFocusPolicy(Qt::StrongFocus);

    // The game loop is implemented using a timer
    connect(&timer, SIGNAL(timeout()), this, SLOT(tick()));

    // create game application
    m_app = new Application();
}

View::~View()
{
    delete m_app;
}

void View::initializeGL()
{
    // All OpenGL initialization *MUST* be done during or after this
    // method. Before this method is called, there is no active OpenGL
    // context and all OpenGL calls have no effect.

    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    glGetError(); // Clear errors after call to glewInit
    if (GLEW_OK != err)
    {
      // Problem: glewInit failed, something is seriously wrong.
      fprintf(stderr, "Error initializing glew: %s\n", glewGetErrorString(err));
    }

    // init the Graphics object.
    m_app->init(new GameMenu(m_app));

    // Enable depth testing, so that objects are occluded based on depth instead of drawing order.
    glEnable(GL_DEPTH_TEST);

//    glEnable(GL_DOUBLEBUFFER);

    // Move the polygons back a bit so lines are still drawn even though they are coplanar with the
    // polygons they came from, which will be drawn before them.
    glEnable(GL_POLYGON_OFFSET_LINE);
    glPolygonOffset(-1, -1);

    // Enable back-face culling, meaning only the front side of every face is rendered.
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // Specify that the front face is represented by vertices in counterclockwise order (this is
    // the default).
    glFrontFace(GL_CCW);

    // Start a timer that will try to get 60 frames per second (the actual
    // frame rate depends on the operating system and other running programs)
    time.start();
    timer.start(1000 / 60);

    // Center the mouse, which is explained more in mouseMoveEvent() below.
    // This needs to be done here because the mouse may be initially outside
    // the fullscreen window and will not automatically receive mouse move
    // events. This occurs if there are two monitors and the mouse is on the
    // secondary monitor.
    QCursor::setPos(mapToGlobal(QPoint(width() / 2, height() / 2)));
}


void View::paintGL()
{
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // TODO: call your game rendering code here
    m_app->onRender();

    if (m_fpsInit)
    {
        totalfps += fps;
        counter++;

        if (counter >= freq)
        {
            float avg = (totalfps / counter);
            std::cout << "Avg fps: " << avg << std::endl;

            if (avg < 20.f)
                std::cout << "ERMAHGERD YE GEME ES SLEEEWWWWWW (fps: " << fps << ")" << std::endl;
            else if (avg < 30.f)
                std::cout << "Ya done messed up dawg. Get it together. (fps: " << fps << ")" << std::endl;

            totalfps = 0.f;
            counter = 0;
        }

    }
    else if (fps > 30.f)
        m_fpsInit = true;

//    // Can't use Core profile with this
//    glUseProgram(0);
//    glColor3f(1.f, 1.f, 1.f);
//    renderText(10, 20, "FPS: " + QString::number((int) (fps)));

}

void View::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
    m_app->onResize(w, h);
    QCursor::setPos(mapToGlobal(QPoint(width() / 2, height() / 2)));
}

void View::mousePressEvent(QMouseEvent *event)
{
    m_app->onMousePressed(event);
}

void View::mouseMoveEvent(QMouseEvent *event)
{
    // This starter code implements mouse capture, which gives the change in
    // mouse position since the last mouse movement. The mouse needs to be
    // recentered after every movement because it might otherwise run into
    // the edge of the screen, which would stop the user from moving further
    // in that direction. Note that it is important to check that deltaX and
    // deltaY are not zero before recentering the mouse, otherwise there will
    // be an infinite loop of mouse move events.

    int halfWidthI = width() / 2;
    int halfHightI = height() / 2;

    int deltaXI = event->x() - halfWidthI;
    int deltaYI = event->y() - halfHightI;
    if (!deltaXI && !deltaYI) return;

    QCursor::setPos(mapToGlobal(QPoint(halfWidthI, halfHightI)));

    // sets mouse deltas between -1 and 1
    float halfWidth = width() * .5f;
    float halfHeight = height() * .5f;

    float deltaX = (event->x() - halfWidth) / halfWidth;
    float deltaY = (event->y() - halfHeight) / halfHeight;

    m_app->onMouseMoved(event, deltaX, deltaY);
}

void View::mouseReleaseEvent(QMouseEvent *event)
{
    m_app->onMouseReleased(event);
}

void View::wheelEvent(QWheelEvent *event)
{
    m_app->onMouseWheel(event);
}

void View::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) QApplication::quit();

    m_app->onKeyPressed(event);
}

void View::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == m_app->getMouseDecoupleKey())
    {
        QCursor::setPos(mapToGlobal(QPoint(width() / 2, height() / 2)));
    }

    m_app->onKeyReleased(event);
}

void View::tick()
{
    // Get the number of seconds since the last tick (variable update rate)
    float seconds = time.restart() * 0.001f;
    fps = .02f / seconds + .98f * fps;

    // TODO: Implement the game update here
    m_app->onTick(seconds);

    // don't show cursor
    QApplication::setOverrideCursor(Qt::BlankCursor);

    // Flag this view for repainting (Qt will call paintGL() soon after)
    update();
}
