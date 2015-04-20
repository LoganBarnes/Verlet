#include "gameworld.h"
#include <glm/gtc/constants.hpp>
#include <iostream>
using namespace std;

GameWorld::GameWorld()
{
    // light struct
    Light *light;

    for (int i = 0; i < 10; i++)
    {
        float angle = i*.2f*glm::pi<float>();

        light = new Light();
        light->id = i;                  // index into array in shader
        light->type = POINT;            // can be POINT or DIRECTIONAL for now
        light->color = glm::vec3(.2f);  // rgb color
        light->posDir = glm::vec3(cos(angle) * 10, 10, sin(angle) * 10 - 15);// position or direction depending on light type

        m_tempLights.append(light);
    }

    mode = 0;
}


GameWorld::~GameWorld()
{
    foreach (Light *l, m_tempLights)
        delete l;
}


Triangle* GameWorld::intersectWorld(glm::vec3 p, glm::vec3 d, float *t)
{
    QList<Triangle *> tris = this->getMesh();
    Triangle *triangle = NULL;
    *t = INFINITY;
    float t2;

    foreach (Triangle *tri, tris)
    {
        t2 = tri->intersectPlane(p, d);
        if (t2 < *t)
        {
            *t = t2;
            triangle = tri;
        }
    }

    return triangle;
}

void GameWorld::onKeyPressed(QKeyEvent *e)
{
    // temp lighting controls
    if(e->key()==49)
        mode=0;
    else if(e->key()==50)
        mode=1;
    else if(e->key()==51)
        mode=2;

    World::onKeyPressed(e);
}

void GameWorld::drawShapes(Graphics *g, int pass, GLuint shader){
    /*
     * Drawing shapes.
     * Set the color of the shape (including transparency and shininess)
     * then call the appropriate draw<shape>() method.
     */

    // if first pass, set the shininess of object
    // if third pass, set the diffuse and specular colors as well as any texture
    // if not using deferred lighting, simply call setColor

    // transparency left out cause it don't play nice with deferred lighting...

    glm::mat4 trans = glm::scale(glm::mat4(), glm::vec3(.1f));

    // sphere

    if(pass==1)
        glUniform1f(glGetUniformLocation(shader, "shininess"), 3.0);
    else if(pass==3){
        glUniform3f(glGetUniformLocation(shader, "cDiffuse"), 1, 0, 0);
        glUniform3f(glGetUniformLocation(shader, "cSpec"),.7,.7,.7);
    }
    else
        g->setColor(1, 0, 0, 1, 0);

    trans = glm::translate(glm::mat4(), glm::vec3(-5, 0, -35));
    g->drawSphere(trans);

    // cone
    if(pass==1)
        glUniform1f(glGetUniformLocation(shader, "shininess"), 3.0);
    else if(pass==3){
        glUniform3f(glGetUniformLocation(shader, "cDiffuse"), 1, .5, 0);
        glUniform3f(glGetUniformLocation(shader, "cSpec"),.7,.7,.7);
    }
    else
        g->setColor(1, .5, 0, 1, 0);
    trans = glm::translate(glm::mat4(), glm::vec3(-2.5, 0, -25));
    g->drawCone(trans);

    // cube
    if(pass==1)
        glUniform1f(glGetUniformLocation(shader, "shininess"), 3.0);
    else if(pass==3){
        glUniform3f(glGetUniformLocation(shader, "cDiffuse"), .5, 1, 0);
        glUniform3f(glGetUniformLocation(shader, "cSpec"),.7,.7,.7);
    }
    else
        g->setColor(.5, 1, 0, 1, 0);
    trans = glm::translate(glm::mat4(), glm::vec3(0, 0, -15));
    g->drawCube(trans);

    // cylinder
    if(pass==1)
        glUniform1f(glGetUniformLocation(shader, "shininess"), 3.0);
    else if(pass==3){
        glUniform3f(glGetUniformLocation(shader, "cDiffuse"), 0, 1, .3);
        glUniform3f(glGetUniformLocation(shader, "cSpec"),.7,.7,.7);
    }
    else
        g->setColor(0, 1, .3, 1, 0);
    trans = glm::translate(glm::mat4(), glm::vec3(2.5, 0, -25));
    g->drawCylinder(trans);

    // quad
    if(pass==1)
        glUniform1f(glGetUniformLocation(shader, "shininess"), 3.0);
    else if(pass==3){
        glUniform3f(glGetUniformLocation(shader, "cDiffuse"), 0, 0, 1);
        glUniform3f(glGetUniformLocation(shader, "cSpec"),.7,.7,.7);
    }
    else
        g->setColor(0, 0, 1, 1, 0);
    trans = glm::translate(glm::mat4(), glm::vec3(5, 0, -35));
    g->drawQuad(trans);

}

void GameWorld::onDraw(Graphics *g){

    if(useDeferredLighting){

        if(mode==1 || mode==2){
            // first pass:
            GLuint firstPassShader = g->setupFirstPass();
            drawShapes(g,1,firstPassShader);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glUseProgram(0);

            // second pass:
            GLuint secondPassShader = g->setupSecondPass();
            glm::vec3 pos = m_player->getEyePos();
            glUniform3f(glGetUniformLocation(secondPassShader, "eyePos"),pos.x, pos.y, pos.z);

            foreach(Light *light, m_tempLights){
                g->addLight(*light);
            }
            g->drawFullScreenQuad(glm::mat4());
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glUseProgram(0);

            // third pass:
            GLuint finalPassShader = g->setupFinalPass();
            glUniform1i( glGetUniformLocation(finalPassShader, "mode"), mode );
            // set global coefficients ka,kd,ks
            glUniform3f( glGetUniformLocation(finalPassShader, "globalConstants"), 1.0, 1.0, 1.0 );
            drawShapes(g,3,finalPassShader);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glUseProgram(0);

            // fog pass:
            GLuint fogShader = g->setupFogPass();
            glUniform3f(glGetUniformLocation(secondPassShader, "eyePos"),pos.x, pos.y, pos.z);
            g->drawFullScreenQuad(glm::mat4());
            glBindFramebuffer( GL_FRAMEBUFFER,0 );
            glUseProgram(0);

        }
        else{
            foreach(Light *light, m_tempLights){
                g->addLight(*light);
            }
            drawShapes(g,0,0);
        }
    }

}
