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
        light->color = glm::vec3(.5f);  // rgb color
        light->posDir = glm::vec3(cos(angle), 10, sin(angle));// position or direction depending on light type
        light->radius = 10.f;

        m_tempLights.append(light);
    }

    mode = 0;
    usingFog = false;
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
    if(e->key()==49){
        if(usingFog)
            usingFog = false;
        else
            usingFog = true;
    }

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

    glm::mat4 trans = glm::scale(glm::mat4(), glm::vec3(1.f));

    // sphere

    if(pass==1){
        glUniform1f(glGetUniformLocation(shader, "shininess"), 3.0);
        glUniform4f(glGetUniformLocation(shader, "materialColor"), 1, 0, 0, .7);
    }
    else
        g->setColor(1, 0, 0, 1, 0);

    trans = glm::translate(glm::mat4(), glm::vec3(0, 1, 0));
    g->drawSphere(trans);

    // cone
    if(pass==1){
        glUniform1f(glGetUniformLocation(shader, "shininess"), 3.0);
        glUniform4f(glGetUniformLocation(shader, "materialColor"), 1, .5, 0, .7);
    }
    else
        g->setColor(1, .5, 0, 1, 0);
    trans = glm::translate(glm::mat4(), glm::vec3(2, 1, 2));
    g->drawCone(trans);

    // cube
    if(pass==1){
        glUniform1f(glGetUniformLocation(shader, "shininess"), 3.0);
        glUniform4f(glGetUniformLocation(shader, "materialColor"), .5, 1, 0, .7);
    }
    else
        g->setColor(.5, 1, 0, 1, 0);
    trans = glm::translate(glm::mat4(), glm::vec3(-2, 1, 2));
    g->drawCube(trans);

    // cylinder
    if(pass==1){
        glUniform1f(glGetUniformLocation(shader, "shininess"), 3.0);
        glUniform4f(glGetUniformLocation(shader, "materialColor"), 0, 1, .3, .7);
    }
    else
        g->setColor(0, 1, .3, 1, 0);
    trans = glm::translate(glm::mat4(), glm::vec3(-10, 1, 2));
    g->drawCylinder(trans);


    if(pass==1){
        glUniform1f(glGetUniformLocation(shader, "shininess"), 3.0);
        glUniform4f(glGetUniformLocation(shader, "materialColor"), 0, 0, 1, .7);
    }
    else
        g->setColor(0, 0, 1, 1, 0);

    trans = glm::translate(glm::mat4(), glm::vec3(-13, 1, 5));
    g->drawSphere(trans);

}


void GameWorld::onDraw(Graphics *g, OBJ* level, VerletManager* vm){

    if(useDeferredLighting){


        // first pass:
        GLuint firstPassShader = g->setupFirstPass();
        level->draw(glm::mat4(), g, 1, firstPassShader);
        drawShapes(g,1,firstPassShader);        //render all geometry
        vm->onDraw(g, firstPassShader, 1);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glUseProgram(0);

        // second pass:
        GLuint secondPassShader = g->setupSecondPass();
        glm::vec4 pos = m_player->getCamEye();
        glUniform3f(glGetUniformLocation(secondPassShader, "eyePos"),pos.x, pos.y, pos.z);

        g->drawLightShapes(glm::vec3(pos.x,pos.y,pos.z), secondPassShader, m_tempLights);
        glDisable(GL_BLEND);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glUseProgram(0);

        // third pass:
        glEnable(GL_DEPTH_TEST);
        GLuint finalPassShader = g->setupFinalPass();
        // set global coefficients ka,kd,ks
        glUniform3f( glGetUniformLocation(finalPassShader, "globalConstants"), 1.0, 1.0, 1.0 );
        g->drawFullScreenQuad(glm::mat4());
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glUseProgram(0);

        // fog pass:
        GLuint fogShader = g->setupFogPass(usingFog);
        glUniform3f(glGetUniformLocation(fogShader, "eyePos"),pos.x, pos.y, pos.z);
        g->drawFullScreenQuad(glm::mat4());
        glBindFramebuffer( GL_FRAMEBUFFER,0 );
        glUseProgram(0);

    }

}
