#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <GL/glew.h>
#include <QString>
#include <QHash>
#include <QTime>
#include "camera.h"
#include "cubemap.h"
#include "shape.h"
#include "cone.h"
#include "cube.h"
#include "cylinder.h"
#include "sphere.h"
#include "facecube.h"
#include "particleemitter.h"

enum GraphicsMode
{
    DEFAULT, SPARSE, CUBEMAP, DRAW2D, GEOMETRY, LIGHT, COMPOSITE, FOG
};

enum ShapeType
{
    QUAD, CUBE, CYLINDER, SPHERE, CONE
};

enum LightType
{
    POINT, DIRECTIONAL
};

struct RenderShape
{
    ShapeType type;
    glm::vec3 color;
    float shininess;
    float transparency;
    glm::mat4 trans;
    glm::mat4 inv;
    QString texture;
    float repeatU, repeatV;
};

// Data for a single light
struct Light
{
    int id;
    LightType type;
    glm::vec3 color;
    glm::vec3 posDir;   // position for point, direction for directional
    glm::vec3 function; // Attenuation function
    float radius;
};

struct ObjectsInfo
{
    QList<glm::mat4> invs;
    QList<glm::vec4> colors;
    QList<ShapeType> shapeType;
};

class Graphics
{
public:
    Graphics();
    ~Graphics();

    void update();

    void init();
    void setCamera(Camera *camera, int w, int h);
    GLuint setGraphicsMode(GraphicsMode gm);

    void setWorldColor(float r, float g, float b);
    void setColor(float r, float g, float b, float transparency, float shininess);
    void setTexture(const QString &key, float repeatU = 1.f, float repeatV = 1.f);
    void setAtlas(const QString &key);
    void setTint(float r, float g, float b);
    void setAtlasPosition(float x, float y);
    void setTransparentMode(bool on);
    void setPlayer(glm::vec3 player, int mode);
    void setAllWhite(bool allWhite);

    glm::mat4 getFrustum();

    void useCubeMap(bool use);
    bool cubeMapIsActive();
    void drawCubeMap(Camera *camera);

    void addLight(const Light &light);

    void drawLineSeg(glm::vec3 p1, glm::vec3 p2, float width, GLenum mode = GL_TRIANGLE_STRIP);
    void drawQuad(glm::mat4 trans, GLenum mode = GL_TRIANGLE_STRIP);
    void drawCone(glm::mat4 trans, GLenum mode = GL_TRIANGLE_STRIP);
    void drawCube(glm::mat4 trans, GLenum mode = GL_TRIANGLE_STRIP);
    void drawCylinder(glm::mat4 trans, GLenum mode = GL_TRIANGLE_STRIP);
    void drawSphere(glm::mat4 trans, GLenum mode = GL_TRIANGLE_STRIP);
    void drawParticles(glm::vec3 source, float fuzziness);
    void drawFullScreenQuad(glm::mat4 trans, GLenum mode = GL_TRIANGLE_STRIP);

    void particlesReset();
    void particlesSetForce(glm::vec3 force);

    static GLuint loadShaders(const char *vertex_file_path, const char *fragment_file_path);

    void loadDeferredLightFBOs(int width, int height);
    GLuint setupFirstPass();
    GLuint setupSecondPass();
    GLuint setupFinalPass();
    GLuint setupFogPass(bool fog);

    void drawLightShapes(glm::vec3 eyePos, GLuint lightShader, QList<Light*> lights);
    bool isInLight(Light* l, glm::vec3 pos);

    void drawLine(const glm::vec3 &a, const glm::vec3 &b);

private:
    void clearLights();

    void loadTexturesFromDirectory();
    void loadTexture(const QString &filename, const QString &key);

    glm::vec3 m_lightPos;

    QHash<QString, GLint> m_defaultLocs;
    QHash<QString, GLint> m_sparseLocs;
    QHash<QString, GLint> m_cubeLocs;
    QHash<QString, GLint> m_geomLocs;
    QHash<QString, GLint> m_lightLocs;
    QHash<QString, GLint> m_compositeLocs;

    QHash<QString, GLuint> m_textures;
    QHash<QString, GLuint> m_shaders;
    QHash<QString, GLint> m_fbos;

    GLuint m_currentShader;
    CubeMap *m_cubeMap;

    glm::mat4 m_currProj;
    glm::mat4 m_currView;
    glm::mat4 m_currScale;

    Shape *m_quad;
    Shape *m_cone;
    Shape *m_cube;
    Shape *m_cyl;
    Shape *m_sphere;

    // deferred
    Shape *m_dquad;
    Shape *m_dcone;
    Shape *m_dcube;
    Shape *m_dcyl;
    Shape *m_dsphere;

    Shape *m_lsphere;

    Shape* m_fullscreen_quad;
    Shape* m_dfullscreen_quad;

    bool m_useCubeMap;
    bool m_usingAtlas;
    bool m_usingFog;

    int m_w, m_h;

    glm::vec2 m_subImages;

    glm::mat4 m_frustum;
    ParticleEmitter *m_pe;

    QTime m_timer;
};

#endif // GRAPHICS_H
