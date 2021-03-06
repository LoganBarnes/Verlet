#ifndef TESTLEVELSCREEN_H
#define TESTLEVELSCREEN_H

#include "screen.h"
#include "audio.h"
#include "lightparser.h"
#include "marker.h"
#include <glm.hpp>

class GameWorld;
class ObjectHandler;
class OBJ;
class VerletManager;
class Verlet;
struct Link;
class Half;
class ParticleSystemManager;

class TestLevelScreen : public ScreenH
{
public:
    TestLevelScreen(Application *parent);
    virtual ~TestLevelScreen();

    // update and render
    virtual void onTick(float secs);
    virtual void onRender(Graphics *g);

    // mouse events
    virtual void onMousePressed(QMouseEvent *e);
    virtual void onMouseMoved(QMouseEvent *e, float deltaX, float deltaY, glm::vec3 pos);
    virtual void onMouseReleased(QMouseEvent *e);

    virtual void onMouseDragged(QMouseEvent *e, float deltaX, float deltaY, glm::vec3 pos);
    virtual void onMouseWheel(QWheelEvent *e);

    // key events
    virtual void onKeyPressed(QKeyEvent *e);
    virtual void onKeyReleased(QKeyEvent *e);

    // resize
    virtual void onResize(int w, int h);

    OBJ* addIsland(const QString& path, GLuint shader, const glm::vec3& offset, glm::vec4 color, ParticleSystemManager *psm = NULL);
    void addMarker(const QString& objPath, GLuint shader, const glm::vec3& offset, const QString& signPath, glm::vec4 color);

    QList<Light*> makeLights();


private:
    void render2D(Graphics *g);
    void adjustDeltasForLeap(float *deltaX, float *deltaY);

    void resetWorld(glm::vec3 playerPos);

    GameWorld *m_world;
    ObjectHandler *m_oh;

    QList<Half*> m_resetHalves;
    int m_resetIndex;

    glm::mat4 m_cursor;
    bool m_drawCursor;
    glm::vec2 m_deltas;

    QList<Marker*> m_markers;
    int m_psmIndex;

protected:
    Marker* m_levelChanger;
    VerletManager *vm;
    Light* characterLight;
};

#endif // TESTLEVELSCREEN_H
