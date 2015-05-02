#ifndef MARKER_H
#define MARKER_H

#include "button.h"
#include "obj.h"

class Marker
{
public:
    Marker(OBJ* obj, glm::vec2 center, glm::vec2 size, QString path);
    bool isInRange(glm::vec3 pos, float range);        // returns whether pos is in range of the marker
    void displayTexture(Graphics* g);

protected:
    Button* m_display;      // texture to display when in range of marker
    OBJ* m_visualization;   // visualization of marker in world
};

#endif // MARKER_H
