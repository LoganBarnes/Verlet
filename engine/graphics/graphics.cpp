#include "graphics.h"
#include <QFile>
#include <QTextStream>
#include <QImage>
#include <QDir>
#include <QGLWidget>
#include <string>
#include <sstream>
#include <iostream>

#define GLM_FORCE_RADIANS
#include <gtc/type_ptr.hpp>
#include <gtx/transform.hpp>
#include <gtx/vector_angle.hpp>

#include "debugprinting.h"

#define MAX_NUM_LIGHTS 10

#include <iostream>
using namespace std;

Graphics::Graphics()
{
    m_quad = new Shape(10);
    m_cone = new Cone(50);
    m_cube = new Cube(10);
    m_cyl = new Cylinder(50);
    m_sphere = new Sphere(50);
    m_fullscreen_quad = new Shape(-1);

    m_dquad = new Shape(10);
    m_dcone = new Cone(50);
    m_dcube = new Cube(10);
    m_dcyl = new Cylinder(50);
    m_dsphere = new Sphere(50);
    m_dfullscreen_quad = new Shape(-1);

    m_cubeMap = new CubeMap();
    m_useCubeMap = false;
    m_usingAtlas = false;

    m_defaultLocs.clear();
    m_cubeLocs.clear();
    m_geomLocs.clear();
    m_lightLocs.clear();
    m_compositeLocs.clear();

    m_currProj = glm::mat4();
    m_currView = glm::mat4();
    m_currScale = glm::mat4();
    m_frustum = glm::mat4();

    m_fbos.clear();
    m_textures.clear();
    m_shaders.clear();

    m_usingFog = false;

    m_pe = new ParticleEmitter();

    m_w = 1;
    m_h = 1;
}

Graphics::~Graphics()
{
    // shapes
    delete m_quad;
    delete m_cone;
    delete m_cube;
    delete m_cyl;
    delete m_sphere;
    delete m_fullscreen_quad;

    delete m_dquad;
    delete m_dcone;
    delete m_dcube;
    delete m_dcyl;
    delete m_dsphere;
    delete m_dfullscreen_quad;

    // skybox
    delete m_cubeMap;

    delete m_pe;
}

void Graphics::init()
{

    GLuint defaultShader, sparseShader, cubeShader, geomShader, lightShader, compositeShader, fogShader;

    defaultShader = Graphics::loadShaders(
                ":/shaders/default.vert",
                ":/shaders/default.frag");

    m_defaultLocs["projection"] = glGetUniformLocation(defaultShader, "projection");
    m_defaultLocs["view"] = glGetUniformLocation(defaultShader, "view");
    m_defaultLocs["model"] = glGetUniformLocation(defaultShader, "model");

    m_defaultLocs["diffuse_color"] = glGetUniformLocation(defaultShader, "diffuse_color");
    m_defaultLocs["world_color"] = glGetUniformLocation(defaultShader, "world_color");

    m_defaultLocs["transparency"] = glGetUniformLocation(defaultShader, "transparency");
    m_defaultLocs["shininess"] = glGetUniformLocation(defaultShader, "shininess");
    m_defaultLocs["useTexture"] = glGetUniformLocation(defaultShader, "useTexture");
    m_defaultLocs["tex"] = glGetUniformLocation(defaultShader, "tex");

    m_defaultLocs["subImages"] = glGetUniformLocation(defaultShader, "subImages");
    m_defaultLocs["subPos"] = glGetUniformLocation(defaultShader, "subPos");
    m_defaultLocs["repeatUV"] = glGetUniformLocation(defaultShader, "repeatUV");

    m_defaultLocs["allBlack"] = glGetUniformLocation(defaultShader, "allBlack");
    m_defaultLocs["allWhite"] = glGetUniformLocation(defaultShader, "allWhite");

    m_shaders.insert("defaultShader",defaultShader);


    sparseShader = Graphics::loadShaders(
                ":/shaders/sparse.vert",
                ":/shaders/sparse.frag");

    m_sparseLocs["projection"] = glGetUniformLocation(sparseShader, "projection");
    m_sparseLocs["view"] = glGetUniformLocation(sparseShader, "view");
    m_sparseLocs["envMap"] = glGetUniformLocation(sparseShader, "envMap");

    m_sparseLocs["tint"] = glGetUniformLocation(sparseShader, "tint");
    m_sparseLocs["player"] = glGetUniformLocation(sparseShader, "player");
    m_sparseLocs["playerMode"] = glGetUniformLocation(sparseShader, "playerMode");

    m_sparseLocs["tex"] = glGetUniformLocation(sparseShader, "tex");
    m_sparseLocs["useTexture"] = glGetUniformLocation(sparseShader, "useTexture");
    m_sparseLocs["transparency"] = glGetUniformLocation(sparseShader, "transparency");

    m_sparseLocs["subImages"] = glGetUniformLocation(sparseShader, "subImages");
    m_sparseLocs["subPos"] = glGetUniformLocation(sparseShader, "subPos");
    m_sparseLocs["repeatUV"] = glGetUniformLocation(sparseShader, "repeatUV");

    m_shaders.insert("sparseShader", sparseShader);


    cubeShader = Graphics::loadShaders(
                ":/shaders/cubemap.vert",
                ":/shaders/cubemap.frag");

    m_cubeLocs["projection"] = glGetUniformLocation(cubeShader, "projection");
    m_cubeLocs["view"] = glGetUniformLocation(cubeShader, "view");
    m_cubeLocs["envMap"] = glGetUniformLocation(cubeShader, "envMap");

    m_cubeMap->init();

    m_shaders.insert("cubeShader", cubeShader);


    // load the geometry shader and set default locations
    geomShader = Graphics::loadShaders(
                ":/shaders/geomPass.vert",
                ":/shaders/geomPass.frag");
    m_geomLocs["projection"] = glGetUniformLocation(geomShader, "projection");
    m_geomLocs["view"] = glGetUniformLocation(geomShader, "view");
    m_geomLocs["model"] = glGetUniformLocation(geomShader, "model");
    m_geomLocs["materialColor"] = glGetUniformLocation(geomShader, "materialColor");
    m_geomLocs["shininess"] = glGetUniformLocation(geomShader, "shininess");

    m_shaders.insert("geomShader", geomShader);


    // load the light pass shader
    lightShader = Graphics::loadShaders(
                ":/shaders/lightPass.vert",
                ":/shaders/lightPass.frag");
    m_lightLocs["positions"] = glGetUniformLocation(lightShader, "positions");
    m_lightLocs["normals"] = glGetUniformLocation(lightShader, "normals");
    m_lightLocs["projection"] = glGetUniformLocation(lightShader, "projection");
    m_lightLocs["view"] = glGetUniformLocation(lightShader, "view");

    m_shaders.insert("lightShader", lightShader);

    // load the final pass shader
    compositeShader = Graphics::loadShaders(
                ":/shaders/finalPass.vert",
                ":/shaders/finalPass.frag");
    m_compositeLocs["projection"] = glGetUniformLocation(compositeShader, "projection");
    m_compositeLocs["view"] = glGetUniformLocation(compositeShader, "view");
    m_compositeLocs["diffuseLights"] = glGetUniformLocation(compositeShader, "diffuseLights");
    m_compositeLocs["specularLights"] = glGetUniformLocation(compositeShader, "specularLights");

    m_shaders.insert("compositeShader", compositeShader);
//    cout << m_defaultLocs["position"] << endl;

    // load fog shader
    fogShader = Graphics::loadShaders(
                ":/shaders/fog.vert",
                ":/shaders/fog.frag");

    m_shaders.insert("fogShader", fogShader);

    m_currentShader = defaultShader;

    m_quad->init(m_currentShader);
    m_cone->init(m_currentShader);
    m_cube->init(m_currentShader);
    m_cyl->init(m_currentShader);
    m_sphere->init(m_currentShader);
    m_fullscreen_quad->init(lightShader);

    m_currentShader = geomShader;

    m_dquad->init(m_currentShader);
    m_dcone->init(m_currentShader);
    m_dcube->init(m_currentShader);
    m_dcyl->init(m_currentShader);
    m_dsphere->init(m_currentShader);
    m_dfullscreen_quad->init(fogShader);
//    m_rayQuad->init(m_rayShader);

    loadTexturesFromDirectory();

    m_pe->initGL(glGetAttribLocation(m_shaders["sparseShader"], "position"));

    m_timer.start();
}

//Initialize three FBOs and their texture attachments for deferred lighting
void Graphics::loadDeferredLightFBOs(int width, int height){

    m_fbos.clear();

    //create and load the first pass framebuffer with position and normal texture attachments
    GLuint geomPass, lightPass, finalPass;
    GLuint positionAttachment, normalAttachment, depthAttachment, diffuseAttachment, specularAttachment, fullLightAttachment;
    GLuint materialColorAttachment;/*, materialTextureAttachment*/;

    // Bind first pass framebuffer with position and normal textures to write to
    // OpenGL garbage:
    glGenFramebuffers( 1, &geomPass );
    glBindFramebuffer( GL_FRAMEBUFFER, geomPass );

    glActiveTexture( GL_TEXTURE0 );
    glGenTextures( 1, &positionAttachment );
    glBindTexture( GL_TEXTURE_2D, positionAttachment );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL );
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, positionAttachment, 0);

    glActiveTexture( GL_TEXTURE1 );
    glGenTextures( 1, &normalAttachment );
    glBindTexture( GL_TEXTURE_2D, normalAttachment );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL );
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, normalAttachment, 0);

    // material properties:
    // diffuse color + spec color
    glActiveTexture( GL_TEXTURE2 );
    glGenTextures( 1, &materialColorAttachment );
    glBindTexture( GL_TEXTURE_2D, materialColorAttachment );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL );
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, materialColorAttachment, 0);

    // depth buffer
    glGenRenderbuffers( 1, &depthAttachment);
    glBindRenderbuffer( GL_RENDERBUFFER, depthAttachment);
    glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, width, height);
    glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthAttachment);

    m_fbos.insert("geomPass", geomPass);
    m_textures.insert("posAttachment", positionAttachment);
    m_textures.insert("normalAttachment", normalAttachment);
    m_textures.insert("materialColorAttachment", materialColorAttachment);
    m_textures.insert("depthAttachment", depthAttachment);

    glBindFramebuffer( GL_FRAMEBUFFER, 0 );

    // Bind second pass framebuffer with diffuse and specular light textures to write to
    glGenFramebuffers( 1, &lightPass);
    glBindFramebuffer( GL_FRAMEBUFFER, lightPass);

    glActiveTexture( GL_TEXTURE0 );
    glGenTextures( 1, &diffuseAttachment);
    glBindTexture( GL_TEXTURE_2D, diffuseAttachment);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL );
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, diffuseAttachment, 0);

    glActiveTexture( GL_TEXTURE1 );
    glGenTextures( 1, &specularAttachment);
    glBindTexture( GL_TEXTURE_2D, specularAttachment);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL );
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, specularAttachment, 0);

    m_fbos.insert("lightPass", lightPass);
    m_textures.insert("diffuseAttachment", diffuseAttachment);
    m_textures.insert("specularAttachment", specularAttachment);

    glBindFramebuffer( GL_FRAMEBUFFER, 0 );

    // bind final pass framebuffer
    glGenFramebuffers( 1, &finalPass);
    glBindFramebuffer( GL_FRAMEBUFFER, finalPass);

    glActiveTexture( GL_TEXTURE0 );
    glGenTextures( 1, &fullLightAttachment);
    glBindTexture( GL_TEXTURE_2D, fullLightAttachment);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL );
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fullLightAttachment, 0);

    m_fbos.insert("finalPass", finalPass);
    m_textures.insert("fullLightAttachment", fullLightAttachment);

    glBindFramebuffer( GL_FRAMEBUFFER, 0);
}


void Graphics::update()
{
    m_pe->updateParticles();
}


void Graphics::setCamera(Camera *camera, int w, int h)
{
    assert(camera);

    m_currProj = camera->getProjectionMatrix();
    m_currView = camera->getViewMatrix();
    m_currScale = camera->getScaleMatrix();
    m_frustum = camera->getFrustumMatrix();

    m_w = w; m_h = h;

    if (m_currentShader == m_shaders["defaultShader"])
        clearLights();
}


GLuint Graphics::setGraphicsMode(GraphicsMode gm)
{
    switch(gm)
    {
    case DEFAULT:
    {
        m_currentShader = m_shaders["defaultShader"];
        glUseProgram(m_shaders["defaultShader"]);

        // Set scene uniforms.
        glUniformMatrix4fv(m_defaultLocs["projection"], 1, GL_FALSE,
                glm::value_ptr(m_currProj));
        glUniformMatrix4fv(m_defaultLocs["view"], 1, GL_FALSE,
                glm::value_ptr(m_currView));
        break;
    }
    case SPARSE:
    {
        m_currentShader = m_shaders["sparseShaders"];
        glUseProgram(m_currentShader);

        // Set scene uniforms.
        glUniformMatrix4fv(m_sparseLocs["projection"], 1, GL_FALSE,
                glm::value_ptr(m_currProj));
        glUniformMatrix4fv(m_sparseLocs["view"], 1, GL_FALSE,
                glm::value_ptr(m_currView));
        break;
    }
    case CUBEMAP:
    {
        break;
    }
    case DRAW2D:
    {
        m_currentShader = m_shaders["defaultShader"];
        glUseProgram(m_shaders["defaultShader"]);
        glm::mat4 trans = glm::mat4();

        // Set scene uniforms.
        glUniformMatrix4fv(m_defaultLocs["projection"], 1, GL_FALSE,
                glm::value_ptr(trans));
        glUniformMatrix4fv(m_defaultLocs["view"], 1, GL_FALSE,
                glm::value_ptr(trans));
        break;
    }
    case GEOMETRY:
    {
        m_currentShader = m_shaders["geomShader"];
        glUseProgram(m_currentShader);

        // Set scene uniforms.
        glUniformMatrix4fv(m_geomLocs["projection"], 1, GL_FALSE,
                glm::value_ptr(m_currProj));
        glUniformMatrix4fv(m_geomLocs["view"], 1, GL_FALSE,
                glm::value_ptr(m_currView));
        break;
    }
    case LIGHT:
    {
        m_currentShader = m_shaders["lightShader"];
        glUseProgram(m_currentShader);

        glUniformMatrix4fv(m_lightLocs["projection"], 1, GL_FALSE,
                glm::value_ptr(m_currProj));
        glUniformMatrix4fv(m_lightLocs["view"], 1, GL_FALSE,
                glm::value_ptr(m_currView));

        // Send attachments from the geometry fbo as textures to lightPass
        glUniform1i( m_lightLocs["positions"], 0 );
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_textures["posAttachment"]);

        glUniform1i( m_lightLocs["normals"], 1 );
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, m_textures["normalAttachment"]);

        glUniform2f(glGetUniformLocation(m_shaders["lightShader"], "viewport") , m_w, m_h);
        break;
    }
    case COMPOSITE:
    {
        m_currentShader = m_shaders["compositeShader"];
        glUseProgram(m_currentShader);

        // Send attachments from the geometry fbo as textures to lightPass
        glUniform1i( glGetUniformLocation(m_currentShader, "diffuseLights"), 0 );
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_textures["diffuseAttachment"]);

        glUniform1i( glGetUniformLocation(m_currentShader, "specularLights"), 1 );
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, m_textures["specularAttachment"]);

        glUniform1i( glGetUniformLocation(m_currentShader, "materialColors"), 2 );
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, m_textures["materialColorAttachment"]);

        glUniform2f(glGetUniformLocation(m_currentShader, "viewport") , m_w, m_h);
        break;
    }
    case FOG:
    {
        // attach lit image and viewport
        m_currentShader = m_shaders["fogShader"];
        glUseProgram(m_currentShader);

        glUniform1i( glGetUniformLocation(m_currentShader, "litImage"), 0 );
        glActiveTexture( GL_TEXTURE0 );
        glBindTexture( GL_TEXTURE_2D, m_textures["fullLightAttachment"]);

        glUniform1i( glGetUniformLocation(m_currentShader, "positions"), 1 );
        glActiveTexture( GL_TEXTURE1 );
        glBindTexture( GL_TEXTURE_2D, m_textures["posAttachment"]);

        glUniform1i(glGetUniformLocation(m_currentShader, "usingFog"), m_usingFog);
        glUniform2f( glGetUniformLocation(m_currentShader, "viewport"), m_w, m_h );
        break;
    }
    }
    return m_currentShader;
}


void Graphics::clearLights()
{
    for (int i = 0; i < MAX_NUM_LIGHTS; i++) {
        std::ostringstream os;
        os << i;
        std::string indexString = "[" + os.str() + "]"; // e.g. [0], [1], etc.
        glUniform3f(glGetUniformLocation(m_shaders["defaultShader"], ("lightColors" + indexString).c_str()), 0, 0, 0);
    }
}


void Graphics::setWorldColor(float r, float g, float b)
{
    glUniform3f(m_defaultLocs["world_color"], r, g, b);
}


void Graphics::setColor(float r, float g, float b, float transparency, float shininess)
{
    if (m_currentShader == m_shaders["geomShader"])
    {
//        glUniform4f(m_geomLocs["materialColor"], .05, .05, .05, .7);
        glUniform4f(m_geomLocs["materialColor"], r, g, b, transparency);
        glUniform1f(m_geomLocs["shininess"], shininess);
        return;
    }
    glUniform3f(m_defaultLocs["diffuse_color"], r, g, b);
    glUniform1f(m_defaultLocs["transparency"], transparency);
    glUniform1f(m_defaultLocs["shininess"], shininess);
}


void Graphics::setTint(float r, float g, float b)
{
    glUniform3f(m_sparseLocs["tint"], r, g, b);
}


void Graphics::setAtlas(const QString &key)
{
    GLint tex = m_textures.value(key);

    if (tex && key.length() > 0)
    {
        glUniform1i(m_sparseLocs["useTexture"], 1);
        glUniform1i(m_sparseLocs["tex"], 1);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, m_textures.value(key));
        glActiveTexture(GL_TEXTURE0);

        m_usingAtlas = true;
    }
    else
    {
        glUniform1i(m_sparseLocs["useTexture"], 0);
        m_usingAtlas = false;
    }
}


void Graphics::setAtlasPosition(float x, float y)
{
    glUniform2f(m_sparseLocs["subPos"], x, y);
}


void Graphics::setTexture(const QString &key, float repeatU, float repeatV)
{
    GLint tex = m_textures.value(key);

    if (tex && key.length() > 0)
    {
        glUniform1i(m_defaultLocs["useTexture"], 1);
        glUniform1i(m_defaultLocs["tex"], 1);
        glUniform2f(m_defaultLocs["repeatUV"], repeatU, repeatV);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, m_textures.value(key));
        glActiveTexture(GL_TEXTURE0);
    }
    else
    {
        glUniform1i(m_defaultLocs["useTexture"], 0);
    }
    m_usingAtlas = false;
}


void Graphics::setTransparentMode(bool on)
{
    if (on)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    else
        glDisable(GL_BLEND);
}


glm::mat4 Graphics::getFrustum()
{
    return m_frustum;
}


void Graphics::setPlayer(glm::vec3 player, int mode)
{
    glUniform3fv(m_sparseLocs["player"], 1, glm::value_ptr(player));
    glUniform1i(m_sparseLocs["playerMode"], mode);
}


void Graphics::useCubeMap(bool use)
{
    m_useCubeMap = use;
}


bool Graphics::cubeMapIsActive()
{
    return m_useCubeMap;
}


void Graphics::drawCubeMap(Camera *camera)
{
    glUseProgram(m_shaders["sparseShader"]);
    glDepthMask(GL_FALSE);

    glUniformMatrix4fv(m_cubeLocs["projection"], 1, GL_FALSE,
            glm::value_ptr(camera->getProjectionMatrix()));
    glUniformMatrix4fv(m_cubeLocs["view"], 1, GL_FALSE,
            glm::value_ptr(camera->getViewMatrix()));
    glUniform1i(m_cubeLocs["envMap"], 1);

    m_cubeMap->render();

    glDepthMask(GL_TRUE);
}


void Graphics::particlesReset()
{
    m_pe->resetParticles();
}


void Graphics::particlesSetForce(glm::vec3 force)
{
    m_pe->setForce(force);
}


void Graphics::setAllWhite(bool allWhite)
{
    if (allWhite)
        glUniform3f(glGetUniformLocation(m_currentShader, "allWhite"), 1, 1, 1);
    else
        glUniform3f(glGetUniformLocation(m_currentShader, "allWhite"), 0, 0, 0);
}









void Graphics::addLight(const Light &light)
{
    std::ostringstream os;
    os << light.id;
    std::string indexString = "[" + os.str() + "]"; // e.g. [0], [1], etc.

//    glUniform1i(glGetUniformLocation(m_shaders["defaultShader"], ("lightTypes" + indexString).c_str()), light.type);
//    glUniform3fv(glGetUniformLocation(m_shaders["defaultShader"], ("lightPositions" + indexString).c_str()), 1,
//            glm::value_ptr(light.posDir));
//    glUniform3fv(glGetUniformLocation(m_shaders["defaultShader"], ("lightColors" + indexString).c_str()), 1,
//                glm::value_ptr(light.color));
//    glUniform3fv(glGetUniformLocation(m_shaders["defaultShader"], ("lightAttenuations" + indexString).c_str()), 1,
//            glm::value_ptr(light.function));

    glUniform1i(glGetUniformLocation(m_shaders["lightShader"], ("lightTypes" + indexString).c_str()), light.type);
    glUniform3fv(glGetUniformLocation(m_shaders["lightShader"], ("lightPositions" + indexString).c_str()), 1,
            glm::value_ptr(light.posDir));
    glUniform3fv(glGetUniformLocation(m_shaders["lightShader"], ("lightColors" + indexString).c_str()), 1,
                glm::value_ptr(light.color));
    glUniform3fv(glGetUniformLocation(m_shaders["lightShader"], ("lightAttenuations" + indexString).c_str()), 1,
            glm::value_ptr(light.function));

}


void Graphics::drawLineSeg(glm::vec3 p1, glm::vec3 p2, float width, GLenum mode)
{
    glm::vec3 d = p2 - p1;


    if (glm::dot(d, d) < 0.00001f)
    {
        m_quad->transformAndRender(m_currentShader,
                                   glm::translate(glm::mat4(), p1) *
                                   glm::scale(glm::mat4(), glm::vec3(0.00001f)), mode);
        return;
    }

    float dmag = glm::length(d);
    glm::vec3 dn = glm::normalize(d);
    glm::vec3 v = glm::vec3(0, 1, 0);

    float angle = glm::angle(dn, v);

    glm::mat4 trans = glm::translate(glm::mat4(), p1 + d * .5f);
    if (angle > 0.0001f)
        trans *= glm::rotate(glm::mat4(), angle, glm::cross(v, dn));
    trans *= glm::scale(glm::mat4(), glm::vec3(width, dmag, width));
    m_cyl->transformAndRender(m_currentShader, trans, mode);
}


void Graphics::drawQuad(glm::mat4 trans, GLenum mode)
{
    if (m_currentShader == m_shaders["geomShader"])
        m_dquad->transformAndRender(m_currentShader, trans, mode);
    else
        m_quad->transformAndRender(m_currentShader, trans, mode);
}

void Graphics::drawFullScreenQuad(glm::mat4 trans, GLenum mode)
{
    if (m_currentShader == m_shaders["fogShader"])
        m_dfullscreen_quad->transformAndRender(m_currentShader, trans, mode);
    else
        m_fullscreen_quad->transformAndRender(m_currentShader, trans, mode);
}


void Graphics::drawCone(glm::mat4 trans, GLenum mode)
{
    if (m_currentShader == m_shaders["geomShader"])
        m_dcone->transformAndRender(m_currentShader, trans, mode);
    else
        m_cone->transformAndRender(m_currentShader, trans, mode);
}


void Graphics::drawCube(glm::mat4 trans, GLenum mode)
{
    if (m_currentShader == m_shaders["geomShader"])
        m_dcube->transformAndRender(m_currentShader, trans, mode);
    else
        m_cube->transformAndRender(m_currentShader, trans, mode);
}


void Graphics::drawCylinder(glm::mat4 trans, GLenum mode)
{
    if (m_currentShader == m_shaders["geomShader"])
        m_dcyl->transformAndRender(m_currentShader, trans, mode);
    else
        m_cyl->transformAndRender(m_currentShader, trans, mode);
}


void Graphics::drawSphere(glm::mat4 trans, GLenum mode)
{
    if (m_currentShader == m_shaders["geomShader"])
        m_dsphere->transformAndRender(m_currentShader, trans, mode);
    else
        m_sphere->transformAndRender(m_currentShader, trans, mode);
}


void Graphics::drawParticles(glm::vec3 source, float fuzziness)
{
    m_pe->setFuzziness(fuzziness);
    m_pe->drawParticlesVAO(m_currentShader, source);
}


// Set up the first frame buffer for rendering into and bind geom shader
GLuint Graphics::setupFirstPass(){

    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);

    // bind appropriate framebuffer and shader
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbos["geomPass"]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor( 0.0f, 0.0f, 0.0f, 0.0f);

    // Necessary to enable multiple drawing targets
    GLenum buffersToDraw[] = { GL_COLOR_ATTACHMENT0 , GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers( 3, buffersToDraw );

    return setGraphicsMode(GEOMETRY);
}

GLuint Graphics::setupSecondPass(){

    glBindFramebuffer(GL_FRAMEBUFFER, m_fbos["lightPass"]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor( 0.0f, 0.0f, 0.0f, 0.0f);

    // Necessary to enable multiple drawing targets
    GLenum buffersToDraw[] = { GL_COLOR_ATTACHMENT0 , GL_COLOR_ATTACHMENT1 };
    glDrawBuffers( 2, buffersToDraw );

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    return setGraphicsMode(LIGHT);
}

GLuint Graphics::setupFinalPass(){
    // composite the final image from lighting
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbos["finalPass"]);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor( 0.0f, 0.0f, 0.0f, 0.0f);

    // Necessary to enable multiple drawing targets
    GLenum buffersToDraw[] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers( 1, buffersToDraw );

    return setGraphicsMode(COMPOSITE);
}

GLuint Graphics::setupFogPass(bool fog){
    // composite final image with fog
    m_usingFog = fog;
    return setGraphicsMode(FOG);
}

// Draw light shapes relative to the given eye position
void Graphics::drawLightShapes(glm::vec3 eyePos, GLuint lightShader, QList<Light*> lights){

    // draw the light shape based on type
    foreach(Light* light, lights){

        // position and radius determine parameters of shape
        glm::mat4 trans;
        //set the uniforms for this light in the light shader
        glUniform1i( glGetUniformLocation(lightShader, "lightType"), light->type );
        glUniform3f( glGetUniformLocation(lightShader, "lightPosition"), light->posDir.x, light->posDir.y, light->posDir.z);
        glUniform3f( glGetUniformLocation(lightShader, "lightAttenuation"), light->function.x, light->function.y, light->function.z);
        glUniform3f( glGetUniformLocation(lightShader, "lightColor"), light->color.x, light->color.y, light->color.z);

        bool inLight = isInLight(light, eyePos);

        if(light->type==POINT){
            glUniform1i( glGetUniformLocation(lightShader, "inLight"), inLight );

            // check if eye position is in light radius
            if(inLight)              // if in the light, render as a full screen quad
                drawFullScreenQuad(glm::mat4());
            else{
                trans = glm::translate(glm::mat4(), light->posDir);
                trans = glm::scale(trans, glm::vec3(light->radius, light->radius, light->radius));
                drawSphere(trans);
            }
        }
    }

}

// returns whether or not given position is in light
bool Graphics::isInLight(Light* l, glm::vec3 pos){

    if(l->type==POINT){
        glm::vec3 diff = pos - l->posDir;
        float lengthSquared = diff.x*diff.x + diff.y*diff.y + diff.z*diff.z;
        float r = l->radius+1;
        return lengthSquared <= (r*r);
    }
    else if(l->type==DIRECTIONAL)
        return true;

    return false;
}


















void Graphics::loadTexturesFromDirectory()
{
    QDir imageDir(":/images");
    QFileInfoList fileList = imageDir.entryInfoList();

    foreach (QFileInfo fileInfo, fileList)
    {
        QString filename = fileInfo.fileName();
        loadTexture(":/images/" + filename, filename);
    }

    std::cout << "Loaded images:" << std::endl;

    QList<QString> keys = m_textures.keys();
    foreach(QString filename, keys)
        std::cout << filename.toStdString() << std::endl;
}


void Graphics::loadTexture(const QString &filename, const QString &key)
{
    // make sure file exists
    QFile file(filename);
    if (!file.exists() || m_textures.contains(key))
    {
        std::cout << "no tex" << std::endl;
        return;
    }

    // load file into memory
    QImage image;
    image.load(file.fileName());
    image = image.mirrored(false, true);
    QImage texture = QGLWidget::convertToGLFormat(image);

    // generate texture ID
    GLuint id = 0;
    glGenTextures(1, &id);

    // make the texture
    glBindTexture(GL_TEXTURE_2D, id);

    // copy image data into texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture.width(), texture.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, texture.bits());
//    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, texture.width(), texture.height(), GL_RGBA, GL_UNSIGNED_BYTE, texture.bits());

    // filtering options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // coordinate wrapping options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    m_textures.insert(key, id);
}


GLuint Graphics::loadShaders(const char *vertex_file_path, const char *fragment_file_path)
{
    // NOTE: MUST INIT GLEW BEFORE USING THIS CODE

    // Create the shaders
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    // Read the Vertex Shader code from the file
    std::string VertexShaderCode;
    QString vertFilePath = QString(vertex_file_path);
    QFile vertFile(vertFilePath);
    if (vertFile.open(QIODevice::ReadOnly | QIODevice::Text)){
        QTextStream vertStream(&vertFile);
        VertexShaderCode = vertStream.readAll().toStdString();
    }


    // Read fragment shader code from file
    std::string FragmentShaderCode;
    QString fragFilePath = QString(fragment_file_path);
    QFile fragFile(fragFilePath);
    if (fragFile.open(QIODevice::ReadOnly | QIODevice::Text)){
        QTextStream fragStream(&fragFile);
        FragmentShaderCode = fragStream.readAll().toStdString();
    }

    GLint Result = GL_FALSE;
    int InfoLogLength;

    // Compile Vertex Shader
    char const * VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
    glCompileShader(VertexShaderID);

    // Check Vertex Shader
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> VertexShaderErrorMessage(InfoLogLength);
    glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
    if (!Result)
        fprintf(stderr, "Error compiling shader: %s\n%s\n",
                vertex_file_path, &VertexShaderErrorMessage[0]);

    // Compile Fragment Shader
    char const * FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
    glCompileShader(FragmentShaderID);

    // Check Fragment Shader
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
    glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
    if (!Result)
        fprintf(stderr, "Error compiling shader: %s\n%s\n",
                fragment_file_path, &FragmentShaderErrorMessage[0]);

    // Link the program
    GLuint programId = glCreateProgram();
    glAttachShader(programId, VertexShaderID);
    glAttachShader(programId, FragmentShaderID);
    glLinkProgram(programId);

    // Check the program
    glGetProgramiv(programId, GL_LINK_STATUS, &Result);
    glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> ProgramErrorMessage( std::max(InfoLogLength, int(1)) );
    glGetProgramInfoLog(programId, InfoLogLength, NULL, &ProgramErrorMessage[0]);
    if (!Result)
        fprintf(stderr, "Error linking shader: %s\n", &ProgramErrorMessage[0]);

    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);

    return programId;
}




void Graphics::drawLine(const glm::vec3 &a, const glm::vec3 &b)
{
    glLineWidth(5.f);
    glBegin(GL_LINES);
    glVertex3f(a.x, a.y, a.z);
    glVertex3f(b.x, b.y, b.z);
    glEnd();
}



