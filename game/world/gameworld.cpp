#include "gameworld.h"
#include <glm/gtc/constants.hpp>

#include "debugprinting.h"

GameWorld::GameWorld()
{
    mode = 0;
    usingFog = false;
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

void GameWorld::drawShapes(Graphics *, int , GLuint ){
    /*
     * Drawing shapes.
     * Set the color of the shape (including transparency and shininess)
     * then call the appropriate draw<shape>() method.
     */

    // if first pass, set the shininess of object
    // if third pass, set the diffuse and specular colors as well as any texture
    // if not using deferred lighting, simply call setColor

    // transparency left out cause it don't play nice with deferred lighting...


//    // cone
//    if(pass==1){
//        glUniform1f(glGetUniformLocation(shader, "shininess"), 3.0);
//        glUniform4f(glGetUniformLocation(shader, "materialColor"), 1, .5, 0, .7);
//    }
//    else
//        g->setColor(1, .5, 0, 1, 0);
//    float offset = sin(m_timeElapsed);
//    glm::mat4 trans = glm::translate(glm::mat4(), glm::vec3(0, 5+offset, 3.5));
//    trans = glm::rotate(trans, 135.f, glm::vec3(1,0,0));

//    g->drawCone(trans);


//    // sphere

//    if(pass==1){
//        glUniform1f(glGetUniformLocation(shader, "shininess"), 3.0);
//        glUniform4f(glGetUniformLocation(shader, "materialColor"), 1, 0, 0, .7);
//    }
//    else
//        g->setColor(1, 0, 0, 1, 0);

//    trans = glm::translate(glm::mat4(), glm::vec3(0, 1, 0));
//    g->drawSphere(trans);

//    // cube
//    if(pass==1){
//        glUniform1f(glGetUniformLocation(shader, "shininess"), 3.0);
//        glUniform4f(glGetUniformLocation(shader, "materialColor"), .5, 1, 0, .7);
//    }
//    else
//        g->setColor(.5, 1, 0, 1, 0);
//    trans = glm::translate(glm::mat4(), glm::vec3(-2, 1, 2));
//    g->drawCube(trans);

//    // cylinder
//    if(pass==1){
//        glUniform1f(glGetUniformLocation(shader, "shininess"), 3.0);
//        glUniform4f(glGetUniformLocation(shader, "materialColor"), 0, 1, .3, .7);
//    }
//    else
//        g->setColor(0, 1, .3, 1, 0);
//    trans = glm::translate(glm::mat4(), glm::vec3(-10, 1, 2));
//    g->drawCylinder(trans);

//    if(pass==1){
//        glUniform1f(glGetUniformLocation(shader, "shininess"), 3.0);
//        glUniform4f(glGetUniformLocation(shader, "materialColor"), 0, 0, 1, .7);
//    }
//    else
//        g->setColor(0, 0, 1, 1, 0);

//    trans = glm::translate(glm::mat4(), glm::vec3(-13, 1, 5));
//    g->drawSphere(trans);

}

void GameWorld::onDraw(Graphics *g){

        GLuint shader = g->setGraphicsMode(GEOMETRY);

        // first pass:
        g->setupFirstPass();
        World::onDraw(g);
//        m_player->onDrawOpaque(g);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glUseProgram(0);

//        // second pass:
        GLuint secondPassShader = g->setupSecondPass();
        glm::vec4 pos = m_player->getCamEye();
        glUniform3f(glGetUniformLocation(secondPassShader, "eyePos"),pos.x, pos.y, pos.z);
        // set global coefficients ka,kd,ks
        glUniform3f( glGetUniformLocation(secondPassShader, "globalConstants"), 1.0, 1.0, 1.0 );
        //Instead of additive blending, add all the light info to the shader then render a full screen quad to the framebuffer

        glUniform1i(glGetUniformLocation(secondPassShader, "numLights"), m_lights.size());

        foreach(Light* l, m_lights)
            g->addLightToSecondPass(*l);
        g->drawFullScreenQuad(glm::mat4());
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glUseProgram(0);

//        // fog pass:
//        glm::vec3 playerPos = m_player->getPosition();
//        // fog pass:
//        GLuint fogShader = g->setupFogPass(usingFog);
//        glUniform3f(glGetUniformLocation(fogShader, "eyePos"),pos.x, pos.y, pos.z);
//        glUniform3f(glGetUniformLocation(fogShader, "playerPos"),playerPos.x, playerPos.y, playerPos.z);
//        g->drawFullScreenQuad(glm::mat4());
//        glBindFramebuffer( GL_FRAMEBUFFER,0 );
//        glUseProgram(0);

}
