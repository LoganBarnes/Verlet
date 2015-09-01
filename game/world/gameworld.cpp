#include "gameworld.h"
#include <glm/gtc/constants.hpp>

#include "debugprinting.h"

GameWorld::GameWorld()
{
    mode = 0;
    usingFog = true;
    freezeOn = false;
}


GameWorld::~GameWorld()
{
    foreach (Light *l, m_lights)
        delete l;
}



void GameWorld::setLights(QList<Light*> l){
    m_lights = l;

    foreach(Light* light, l)
        lightColors.append(light->color);
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

void GameWorld::toggleLightColors(){

    if(freezeOn){
        // lights to grayscale
        foreach(Light* l, m_lights)
            l->color = glm::vec3(.5);
    }
    else{
       // lights to regular
        for(int i=0; i<m_lights.size(); i++){
            m_lights.at(i)->color = lightColors.at(i);
        }
    }
}

void GameWorld::onKeyPressed(QKeyEvent *e)
{
    // temp lighting controls
    if(e->key()==Qt::Key_1){
        if(usingFog)
            usingFog = false;
        else
            usingFog = true;
    }
    if(e->key() == Qt::Key_2){
        if(useDeferredLighting)
            useDeferredLighting = false;
        else
            useDeferredLighting = true;
    }

    if(e->key() == Qt::Key_F){
        if(freezeOn)
            freezeOn = false;
        else
            freezeOn = true;
        toggleLightColors();
    }

    World::onKeyPressed(e);
}

void GameWorld::drawShapes(Graphics *, int , GLuint ){}

void GameWorld::onDraw(Graphics *g){


    if(useDeferredLighting){

        // first pass:
        g->setupFirstPass();
        World::onDraw(g);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glUseProgram(0);

        // second pass:
        GLuint secondPassShader = g->setupSecondPass();
        glm::vec4 pos = m_player->getCamEye();
        glUniform3f(glGetUniformLocation(secondPassShader, "eyePos"),pos.x, pos.y, pos.z);
        // set global coefficients ka,kd,ks
        glUniform3f( glGetUniformLocation(secondPassShader, "globalConstants"), 1.0, 1.0, 1.0 );
        //Instead of additive blending, add all the light info to the shader then render a full screen quad to the framebuffer

        glUniform1i(glGetUniformLocation(secondPassShader, "usingFog"), usingFog);
        glUniform1i(glGetUniformLocation(secondPassShader, "numLights"), m_lights.size());

        foreach(Light* l, m_lights)
            g->addLightToSecondPass(*l);
        g->drawFullScreenQuad(glm::mat4());
        glUseProgram(0);
        glBindFramebuffer( GL_FRAMEBUFFER,0 );

        // fog pass:
        GLuint fogShader = g->setupFogPass(usingFog);
        glUniform3f(glGetUniformLocation(fogShader, "eyePos"),pos.x, pos.y, pos.z);
        g->drawFullScreenQuad(glm::mat4());
        glBindFramebuffer( GL_FRAMEBUFFER,0 );
        glUseProgram(0);

    }
    else{
            g->setGraphicsMode(DEFAULT);
            foreach(Light* l, m_lights)
                g->addLight(*l);
            World::onDraw(g);

            glm::mat4 trans = glm::scale(glm::mat4(), glm::vec3(.5f));
            glm::mat4 posMat = glm::mat4();

            g->setAllWhite(true);
    //        g->setTransparentMode(true);
            g->setColor(1, 1, 1, .2, 0);
            foreach (Light *l, m_lights)
            {
                posMat[3] = glm::vec4(l->posDir, 1);
                g->drawSphere(posMat * trans);
            }
            g->setAllWhite(false);
    //        g->setTransparentMode(false);
        }





}
