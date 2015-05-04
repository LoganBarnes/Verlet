#include "gameworld.h"
#include <glm/gtc/constants.hpp>

#include "debugprinting.h"

GameWorld::GameWorld()
{
    mode = 0;
    usingFog = false;
}


GameWorld::~GameWorld()
{
    foreach (Light *l, m_lights)
        delete l;
}

void GameWorld::setLights(QList<Light*> l){
//    m_lights = l;

    m_lights.clear();

    int counter = m_lights.size();

    // Custom lights
    QList<glm::vec3> positions;
    positions.append(glm::vec3(0,3,0));
    positions.append(glm::vec3(10,5,0));
    positions.append(glm::vec3(-10,5,0));
    positions.append(glm::vec3(-50,5,0));

    for(int i=0; i<positions.size(); i++){

        Light* light;

        light = new Light();
        light->id = counter++;
        light->type = POINT;
        light->color = glm::vec3(.750, .750, 1.5f);  // rgb color

        light->posDir = positions.at(i);

        light->radius = 50.f;
        light->function = glm::vec3(1.0, .1, .01);

        m_lights.append(light);
    }

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


    if(useDeferredLighting){

        GLuint shader = g->setGraphicsMode(GEOMETRY);

        // first pass:
        g->setupFirstPass();
        World::onDraw(g);
        drawShapes(g, 1, shader);
        m_player->onDrawOpaque(g);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glUseProgram(0);

        // second pass:
        GLuint secondPassShader = g->setupSecondPass();
        glm::vec4 pos = m_player->getCamEye();
        glUniform3f(glGetUniformLocation(secondPassShader, "eyePos"),pos.x, pos.y, pos.z);

        g->drawLightShapes(glm::vec3(pos.x,pos.y,pos.z), secondPassShader, m_lights);
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
