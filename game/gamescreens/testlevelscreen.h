#ifndef TESTLEVELSCREEN_H
#define TESTLEVELSCREEN_H

#include "screen.h"

class GameWorld;
class ObjectHandler;
class OBJ;
class VerletManager;
class Verlet;

class TestLevelScreen : public Screen
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

private:
    void render2D(Graphics *g);
    void adjustDeltasForLeap(float *deltaX, float *deltaY);

    GameWorld *m_world;
    ObjectHandler *m_oh;
    OBJ *m_level;

    VerletManager *vm;

    glm::mat4 m_cursor;
    bool m_drawCursor;
    glm::vec2 m_deltas;

    //testing dragging
    bool dragMode = false; //true if player selects point + holds LMB
    //Selected attributes- don't change once dragMode is enabled
    int draggedPoint = 0;
    Verlet* draggedVerlet = NULL;
    //For moving the selected point
    //World-space pt: where cursor's ray intersects w/ draggedPoint's plane
    glm::vec3 draggedMouse;
    //from draggedPoint to draggedMouse
    glm::vec3 interpolate;

    //testing wind
    glm::vec3 windDirection;

};

#endif // TESTLEVELSCREEN_H
