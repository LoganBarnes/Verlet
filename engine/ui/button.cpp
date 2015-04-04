#include "button.h"
#define GLM_FORCE_RADIANS
#include <glm/gtx/transform.hpp>

#include <iostream>
using namespace std;


Button::Button()
{
    m_image = "";
    m_trans = glm::scale(glm::mat4(), glm::vec3(.2, .124, 1));
    m_trans[3][2] = -1;
    m_color = glm::vec4(.5, .5, .5, 1.f);
}

Button::~Button()
{
}

void Button::setCenter(float x, float y)
{
    m_trans[3][0] = x;
    m_trans[3][2] = y;
}

void Button::setSize(float w, float h)
{
    m_size.x = w;
    m_size.y = h;

    glm::vec4 center = m_trans[3];
    m_trans = glm::scale(glm::mat4(), glm::vec3(m_size, 1));
    m_trans[3] = center;
}

void Button::setColor(float r, float g, float b, float a)
{
    m_color.r = r;
    m_color.g = g;
    m_color.b = b;
    m_color.a = a;
}

void Button::setImage(QString image)
{
    m_image = image;
}

void Button::onDraw(Graphics *g)
{
    g->setWorldColor(1, 1, 1);
    g->setColor(m_color.r, m_color.g, m_color.b, m_color.a, 0);

    if (m_image.size() > 0)
        g->setTexture(m_image);

    g->drawQuad(m_trans);
}

bool Button::contains(float x, float y)
{
    glm::vec2 center = glm::vec2(m_trans[3]);
    glm::vec2 ul = center - m_size * .5f; // upper left
    glm::vec2 br = center + m_size * .5f; // bottom right

    return !(x < ul.x || y < ul.y || x > br.x || y > br.y);
}

