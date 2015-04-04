#ifndef BUTTON_H
#define BUTTON_H

#include <glm.hpp>
#include <QString>
#include "graphics.h"

class Button
{
public:
    Button();
    virtual ~Button();

    void setCenter(float x, float y);
    void setSize(float w, float h);
    void setColor(float r, float g, float b, float a = 1.f);
    void setImage(QString image);

    /**
     * @brief onDraw - Graphics::setGraphicsMode(GraphicsMode::DRAW2D) should
     * be called prior to using this method. Othersize render locations are not
     * guaranteed.
     *
     * @param g
     */
    void onDraw(Graphics *g);

    bool contains(float x, float y);

private:
    QString m_image;
    glm::mat4 m_trans;
    glm::vec4 m_color;
    glm::vec2 m_size;

};

#endif // BUTTON_H
