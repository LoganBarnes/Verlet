#include "cubemap.h"
#include <QFile>
#include <QTextStream>
#include <QImage>
#include <iostream>
#include <cassert>
#include <QGLWidget>

#define RIGHT ":/cubemap/posx.jpg"
#define LEFT ":/cubemap/negx.jpg"
#define TOP ":/cubemap/posy.jpg"
#define BOTTOM ":/cubemap/negy.jpg"
#define BACK ":/cubemap/posz.jpg"
#define FRONT ":/cubemap/negz.jpg"

using namespace std;

CubeMap::CubeMap()
{
}


CubeMap::~CubeMap()
{
    glDeleteTextures(1, &m_texID);
    glDeleteBuffers(1, &m_vboID);
    glDeleteVertexArrays(1, &m_vaoID);
}


void CubeMap::init()
{
    setCube(550.f);
    setTextures();
}


void CubeMap::setCube(float radius)
{
    float points[] = {
        -radius ,  radius,  radius, // 4
         radius ,  radius,  radius, // 3
        -radius , -radius,  radius, // 7
         radius , -radius,  radius, // 8

         radius , -radius, -radius, // 5
         radius ,  radius,  radius, // 3
         radius ,  radius, -radius, // 1

        -radius ,  radius,  radius, // 4
        -radius ,  radius, -radius, // 2
        -radius , -radius,  radius, // 7

        -radius , -radius, -radius, // 6
         radius , -radius, -radius, // 5
        -radius ,  radius, -radius, // 2
         radius ,  radius, -radius  // 1
    };

    glGenBuffers (1, &m_vboID);
    glBindBuffer (GL_ARRAY_BUFFER, m_vboID);
    glBufferData (GL_ARRAY_BUFFER, 3 * 14 * sizeof (float), &points, GL_STATIC_DRAW);

    glGenVertexArrays (1, &m_vaoID);
    glBindVertexArray (m_vaoID);
    glEnableVertexAttribArray (0);
    glBindBuffer (GL_ARRAY_BUFFER, m_vboID);
    glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    // Clean up -- unbind things.
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}


void CubeMap::setTextures()
{
    glActiveTexture (GL_TEXTURE0);
    glGenTextures(1, &m_texID);

    assert(loadTexture(m_texID, GL_TEXTURE_CUBE_MAP_POSITIVE_X, QString::fromStdString(RIGHT)));
    assert(loadTexture(m_texID, GL_TEXTURE_CUBE_MAP_NEGATIVE_X, QString::fromStdString(LEFT)));
    assert(loadTexture(m_texID, GL_TEXTURE_CUBE_MAP_POSITIVE_Y, QString::fromStdString(TOP)));
    assert(loadTexture(m_texID, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, QString::fromStdString(BOTTOM)));
    assert(loadTexture(m_texID, GL_TEXTURE_CUBE_MAP_POSITIVE_Z, QString::fromStdString(BACK)));
    assert(loadTexture(m_texID, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, QString::fromStdString(FRONT)));

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}


bool CubeMap::loadTexture(GLuint tex, GLenum side, const QString &filename)
{
    // make sure file exists
    QFile file(filename);
    if (!file.exists())
        return false;

    // load file into memory
    QImage image;
    image.load(file.fileName());
    image = image.mirrored(false, true);
    QImage texture = QGLWidget::convertToGLFormat(image);

    // make the texture
    glBindTexture(GL_TEXTURE_CUBE_MAP, tex);

    glTexImage2D(side, 0, GL_RGBA, texture.width(), texture.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, texture.bits());

    return true;
}


void CubeMap::bindTexture()
{
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_texID);
    glActiveTexture(GL_TEXTURE0);
}

void CubeMap::render()
{
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_texID);
    glActiveTexture(GL_TEXTURE0);

    glBindVertexArray(m_vaoID);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 14);
    glBindVertexArray(0);
}


