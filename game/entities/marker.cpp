#include "marker.h"


/**
 * Support for telling when a checkpoint/obj has been reached and
 * a texture to display
 */
Marker::Marker(OBJ* obj, glm::vec2 center, glm::vec2 size, QString path)
{
    m_visualization = obj;

    m_display = new Button();
    m_display->setCenter(center.x, center.y);
    m_display->setSize(size.x, size.y);
    m_display->setColor(1,1,1,.5);
    m_display->setImage(path);
}


// Check if the member obj is in range of the position
bool Marker::isInRange(glm::vec3 pos, float range){

    return false;


}


void Marker::displayTexture(Graphics* g){
//    g->setTransparentMode(true);
    g->setWorldColor(1, 1, 1);
    g->setColor(1, 1, 1, .5, 0);

    g->setGraphicsMode(DRAW2D);
    m_display->onDraw(g);

    g->setTexture("");
    g->setColor(1, 0, 0, .5, 0);
//    g->setTransparentMode(false);
}
