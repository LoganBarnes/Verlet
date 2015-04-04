#include "graphics.h"
#include <QFile>
#include <QTextStream>
#include <QImage>
#include <QDir>
#include <QGLWidget>
#include <string>
#include <sstream>

#define GLM_FORCE_RADIANS
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/vector_angle.hpp>

//#include "printing.h"
#include <iostream>
using namespace std;

#define MAX_NUM_LIGHTS 10

Graphics::Graphics()
{
    m_line = new Line(1);
    m_quad = new Shape(10);
    m_cone = new Cone(50);
    m_cube = new Cube(10);
    m_cyl = new Cylinder(50);
    m_sphere = new Sphere(50);
    m_faceCube = new FaceCube();
    m_rayQuad = new Shape(-1);

    m_cubeMap = new CubeMap();
    m_useCubeMap = false;
    m_usingAtlas = false;
    m_subImages = glm::vec2(1.f);

    m_defaultLocs.clear();
    m_sparseLocs.clear();
    m_cubeLocs.clear();

    m_currProj = glm::mat4();
    m_currView = glm::mat4();
    m_currScale = glm::mat4();
    m_frustum = glm::mat4();

    m_pe = new ParticleEmitter();

    m_w = 1;
    m_h = 1;
}

Graphics::~Graphics()
{
    // shapes
    delete m_line;
    delete m_quad;
    delete m_cone;
    delete m_cube;
    delete m_cyl;
    delete m_sphere;
    delete m_faceCube;
    delete m_rayQuad;

    // skybox
    delete m_cubeMap;

    delete m_pe;
}

void Graphics::init()
{
    m_defaultShader = Graphics::loadShaders(
                ":/shaders/default.vert",
                ":/shaders/default.frag");

    m_defaultLocs["projection"] = glGetUniformLocation(m_defaultShader, "projection");
    m_defaultLocs["view"] = glGetUniformLocation(m_defaultShader, "view");
    m_defaultLocs["model"] = glGetUniformLocation(m_defaultShader, "model");

    m_defaultLocs["diffuse_color"] = glGetUniformLocation(m_defaultShader, "diffuse_color");
    m_defaultLocs["world_color"] = glGetUniformLocation(m_defaultShader, "world_color");

    m_defaultLocs["transparency"] = glGetUniformLocation(m_defaultShader, "transparency");
    m_defaultLocs["shininess"] = glGetUniformLocation(m_defaultShader, "shininess");
    m_defaultLocs["useTexture"] = glGetUniformLocation(m_defaultShader, "useTexture");
    m_defaultLocs["tex"] = glGetUniformLocation(m_defaultShader, "tex");

    m_defaultLocs["subImages"] = glGetUniformLocation(m_defaultShader, "subImages");
    m_defaultLocs["subPos"] = glGetUniformLocation(m_defaultShader, "subPos");
    m_defaultLocs["repeatUV"] = glGetUniformLocation(m_defaultShader, "repeatUV");

    m_defaultLocs["allBlack"] = glGetUniformLocation(m_defaultShader, "allBlack");
    m_defaultLocs["allWhite"] = glGetUniformLocation(m_defaultShader, "allWhite");


    m_sparseShader = Graphics::loadShaders(
                ":/shaders/sparse.vert",
                ":/shaders/sparse.frag");

    m_sparseLocs["projection"] = glGetUniformLocation(m_sparseShader, "projection");
    m_sparseLocs["view"] = glGetUniformLocation(m_sparseShader, "view");
    m_sparseLocs["envMap"] = glGetUniformLocation(m_sparseShader, "envMap");

    m_sparseLocs["tint"] = glGetUniformLocation(m_sparseShader, "tint");
    m_sparseLocs["player"] = glGetUniformLocation(m_sparseShader, "player");
    m_sparseLocs["playerMode"] = glGetUniformLocation(m_sparseShader, "playerMode");

    m_sparseLocs["tex"] = glGetUniformLocation(m_sparseShader, "tex");
    m_sparseLocs["useTexture"] = glGetUniformLocation(m_sparseShader, "useTexture");
    m_sparseLocs["transparency"] = glGetUniformLocation(m_sparseShader, "transparency");

    m_sparseLocs["subImages"] = glGetUniformLocation(m_sparseShader, "subImages");
    m_sparseLocs["subPos"] = glGetUniformLocation(m_sparseShader, "subPos");
    m_sparseLocs["repeatUV"] = glGetUniformLocation(m_sparseShader, "repeatUV");


    m_rayShader = Graphics::loadShaders(
                ":/shaders/ray.vert",
                ":/shaders/ray2.frag");

    m_rayLocs["viewport"] = glGetUniformLocation(m_rayShader, "viewport");
    m_rayLocs["filmToWorld"] = glGetUniformLocation(m_rayShader, "filmToWorld");
    m_rayLocs["camEye"] = glGetUniformLocation(m_rayShader, "camEye");
    m_rayLocs["envMap"] = glGetUniformLocation(m_rayShader, "envMap");
    m_rayLocs["time"] = glGetUniformLocation(m_rayShader, "time");


    m_cubeShader = Graphics::loadShaders(
                ":/shaders/cubemap.vert",
                ":/shaders/cubemap.frag");

    m_cubeLocs["projection"] = glGetUniformLocation(m_cubeShader, "projection");
    m_cubeLocs["view"] = glGetUniformLocation(m_cubeShader, "view");
    m_cubeLocs["envMap"] = glGetUniformLocation(m_cubeShader, "envMap");

    m_cubeMap->init();

    m_line->init(m_defaultShader);
    m_quad->init(m_defaultShader);
    m_cone->init(m_defaultShader);
    m_cube->init(m_defaultShader);
    m_cyl->init(m_defaultShader);
    m_sphere->init(m_defaultShader);
    m_faceCube->init(m_sparseShader);
    m_rayQuad->init(m_rayShader);

    loadTexturesFromDirectory();

    m_currentShader = m_defaultShader;
    m_pe->initGL(glGetAttribLocation(m_sparseShader, "position"));

    m_timer.start();
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

    if (m_currentShader == m_defaultShader)
        clearLights();
}


GLuint Graphics::setGraphicsMode(GraphicsMode gm)
{
    switch(gm)
    {
    case DEFAULT:
        m_currentShader = m_defaultShader;
        glUseProgram(m_defaultShader);

        // Set scene uniforms.
        glUniformMatrix4fv(m_defaultLocs["projection"], 1, GL_FALSE,
                glm::value_ptr(m_currProj));
        glUniformMatrix4fv(m_defaultLocs["view"], 1, GL_FALSE,
                glm::value_ptr(m_currView));
        break;
    case SPARSE:
        m_currentShader = m_sparseShader;
        glUseProgram(m_sparseShader);

        // Set scene uniforms.
        glUniformMatrix4fv(m_sparseLocs["projection"], 1, GL_FALSE,
                glm::value_ptr(m_currProj));
        glUniformMatrix4fv(m_sparseLocs["view"], 1, GL_FALSE,
                glm::value_ptr(m_currView));
        break;
    case RAY:
        m_currentShader = m_rayShader;
        glUseProgram(m_rayShader);

        // Set scene uniforms.
        glUniformMatrix4fv(m_rayLocs["filmToWorld"], 1, GL_FALSE,
                glm::value_ptr(glm::inverse(m_currScale * m_currView)));
        glUniform4fv(m_rayLocs["camEye"], 1,
                glm::value_ptr((glm::inverse(m_currView) * glm::vec4(0, 0, 0, 1))));
        glUniform2f(m_rayLocs["viewport"], m_w, m_h);
        glUniform1i(m_rayLocs["envMap"], 1);
        glUniform1f(m_rayLocs["time"], (m_timer.elapsed() * 0.001f));
        m_cubeMap->bindTexture();
        break;
    case CUBEMAP:
        break;
    case DRAW2D:
        m_currentShader = m_defaultShader;
        glUseProgram(m_defaultShader);
        glm::mat4 trans = glm::mat4();

        // Set scene uniforms.
        glUniformMatrix4fv(m_defaultLocs["projection"], 1, GL_FALSE,
                glm::value_ptr(trans));
        glUniformMatrix4fv(m_defaultLocs["view"], 1, GL_FALSE,
                glm::value_ptr(trans));
        break;
    }
    return m_currentShader;
}


void Graphics::clearLights()
{
    for (int i = 0; i < MAX_NUM_LIGHTS; i++) {
        std::ostringstream os;
        os << i;
        std::string indexString = "[" + os.str() + "]"; // e.g. [0], [1], etc.
        glUniform3f(glGetUniformLocation(m_defaultShader, ("lightColors" + indexString).c_str()), 0, 0, 0);
    }
}


void Graphics::setWorldColor(float r, float g, float b)
{
    glUniform3f(m_defaultLocs["world_color"], r, g, b);
}


void Graphics::setColor(float r, float g, float b, float transparency, float shininess)
{
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
//        m_subImages = numSubImages;
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
    glUseProgram(m_cubeShader);
    glDepthMask(GL_FALSE);

    glUniformMatrix4fv(m_cubeLocs["projection"], 1, GL_FALSE,
            glm::value_ptr(camera->getProjectionMatrix()));
    glUniformMatrix4fv(m_cubeLocs["view"], 1, GL_FALSE,
            glm::value_ptr(camera->getViewMatrix()));
    glUniform1i(m_cubeLocs["envMap"], 1);

    m_cubeMap->render();

    glDepthMask(GL_TRUE);
}


void Graphics::resetParticles()
{
    m_pe->resetParticles();
}


void Graphics::setParticleForce(glm::vec3 force)
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

    glUniform1i(glGetUniformLocation(m_defaultShader, ("lightTypes" + indexString).c_str()), light.type);
    glUniform3fv(glGetUniformLocation(m_defaultShader, ("lightPositions" + indexString).c_str()), 1,
            glm::value_ptr(light.posDir));
    glUniform3fv(glGetUniformLocation(m_defaultShader, ("lightColors" + indexString).c_str()), 1,
                glm::value_ptr(light.color));
    glUniform3fv(glGetUniformLocation(m_defaultShader, ("lightAttenuations" + indexString).c_str()), 1,
            glm::value_ptr(light.function));
}


void Graphics::drawLineSeg(glm::vec3 p1, glm::vec3 p2, float width, GLenum mode)
{
    glm::vec3 d = p2 - p1;

    if (glm::dot(d, d) < 0.00001f)
    {
        m_line->transformAndRender(m_currentShader,
                                   glm::scale(glm::mat4(), glm::vec3(0.00001f)), mode);
        return;
    }

    float dmag = glm::length(d);
    glm::vec3 dn = glm::normalize(d);
    glm::vec3 v = glm::vec3(0, 1, 0);

    glm::mat4 trans = glm::translate(glm::mat4(), p1 + d * .5f);
    trans *= glm::rotate(glm::mat4(), glm::angle(dn, v), glm::cross(v, dn));
    trans *= glm::scale(glm::mat4(), glm::vec3(width, dmag, width));
    m_cyl->transformAndRender(m_currentShader, trans, mode);
}


void Graphics::drawLine(glm::mat4 trans, GLenum mode)
{
    m_line->transformAndRender(m_defaultShader, trans, mode);
}


void Graphics::drawQuad(glm::mat4 trans, GLenum mode)
{
    m_quad->transformAndRender(m_currentShader, trans, mode);
}


void Graphics::drawCone(glm::mat4 trans, GLenum mode)
{
    m_cone->transformAndRender(m_currentShader, trans, mode);
}


void Graphics::drawCube(glm::mat4 trans, GLenum mode)
{
    m_cube->transformAndRender(m_currentShader, trans, mode);
}


void Graphics::drawCyl(glm::mat4 trans, GLenum mode)
{
    m_cyl->transformAndRender(m_currentShader, trans, mode);
}


void Graphics::drawSphere(glm::mat4 trans, GLenum mode)
{
    m_sphere->transformAndRender(m_currentShader, trans, mode);
}


void Graphics::drawFaceCube(glm::mat4 trans, int faces)
{
    m_faceCube->transformAndRender(m_currentShader, trans, faces);
}


void Graphics::drawParticles(glm::vec3 source, float fuzziness)
{
    m_pe->setFuzziness(fuzziness);
    m_pe->drawParticlesVAO(m_currentShader, source);
}










void Graphics::rayAddObjects(ObjectsInfo *info)
{
    int size = info->invs.size();
    for (int i = 0; i < size; i++)
    {
        std::ostringstream os;
        os << i;
        std::string indexString = "[" + os.str() + "]"; // e.g. [0], [1], etc.

        glUniformMatrix4fv(glGetUniformLocation(m_rayShader, ("invs" + indexString).c_str()), 1, GL_FALSE, glm::value_ptr(info->invs[i]));
        glUniform4fv(glGetUniformLocation(m_rayShader, ("colors" + indexString).c_str()), 1, glm::value_ptr(info->colors[i]));
        glUniform1i(glGetUniformLocation(m_rayShader, ("types" + indexString).c_str()), info->shapeType[i]);
    }
    glUniform1i(glGetUniformLocation(m_rayShader, "NUM_OBJECTS"), size);

    delete info;
}


void Graphics::rayAddTransparents(ObjectsInfo *info)
{
    int size = info->invs.size();
    for (int i = 0; i < size; i++)
    {
        std::ostringstream os;
        os << i;
        std::string indexString = "[" + os.str() + "]"; // e.g. [0], [1], etc.

        glUniformMatrix4fv(glGetUniformLocation(m_rayShader, ("invsT" + indexString).c_str()), 1, GL_FALSE, glm::value_ptr(info->invs[i]));
        glUniform4fv(glGetUniformLocation(m_rayShader, ("colorsT" + indexString).c_str()), 1, glm::value_ptr(info->colors[i]));
        glUniform1i(glGetUniformLocation(m_rayShader, ("typesT" + indexString).c_str()), info->shapeType[i]);
    }
    glUniform1i(glGetUniformLocation(m_rayShader, "NUM_TRANSPARENTS"), size);

    delete info;
}


void Graphics::rayDrawQuad()
{
    m_rayQuad->render();
}












void Graphics::loadTexturesFromDirectory()
{
    QDir imageDir(":/images");
    QFileInfoList fileList = imageDir.entryInfoList();

    cout << "Loaded images:" << endl;
    foreach (QFileInfo fileInfo, fileList)
    {
        QString filename = fileInfo.fileName();
        cout << filename.toStdString() << endl;
        loadTexture(":/images/" + filename, filename);
    }
}


void Graphics::loadTexture(const QString &filename, const QString &key)
{
    // make sure file exists
    QFile file(filename);
    if (!file.exists() || m_textures.contains(key))
    {
        cout << "no tex" << endl;
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



