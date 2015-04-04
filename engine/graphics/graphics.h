#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <GL/glew.h>
#include <QString>
#include <QHash>
#include <QTime>
#include "camera.h"
#include "cubemap.h"
#include "shape.h"
#include "line.h"
#include "cone.h"
#include "cube.h"
#include "cylinder.h"
#include "sphere.h"
#include "facecube.h"
#include "particleemitter.h"

enum GraphicsMode
{
    DEFAULT, SPARSE, CUBEMAP, DRAW2D, RAY
};

enum ShapeType
{
    LINE, QUAD, CUBE, CYLINDER, SPHERE, CONE
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
    void drawLine(glm::mat4 trans, GLenum mode = GL_TRIANGLE_STRIP);
    void drawQuad(glm::mat4 trans, GLenum mode = GL_TRIANGLE_STRIP);
    void drawCone(glm::mat4 trans, GLenum mode = GL_TRIANGLE_STRIP);
    void drawCube(glm::mat4 trans, GLenum mode = GL_TRIANGLE_STRIP);
    void drawCyl(glm::mat4 trans, GLenum mode = GL_TRIANGLE_STRIP);
    void drawSphere(glm::mat4 trans, GLenum mode = GL_TRIANGLE_STRIP);
    void drawFaceCube(glm::mat4 trans, int info);
    void drawParticles(glm::vec3 source, float fuzziness);

    void resetParticles();
    void setParticleForce(glm::vec3 force);

    // ray stuff
    void rayAddObjects(ObjectsInfo *info);
    void rayAddTransparents(ObjectsInfo *info);
    void rayDrawQuad();

    static GLuint loadShaders(const char *vertex_file_path, const char *fragment_file_path);

private:
    void clearLights();

    void loadTexturesFromDirectory();
    void loadTexture(const QString &filename, const QString &key);

    glm::vec3 m_lightPos;

    QHash<QString, GLint> m_defaultLocs;
    QHash<QString, GLint> m_sparseLocs;
    QHash<QString, GLint> m_cubeLocs;
    QHash<QString, GLint> m_rayLocs;

    QHash<QString, GLint> m_textures;

    GLuint m_currentShader;
    GLuint m_defaultShader;
    GLuint m_sparseShader;
    GLuint m_cubeShader;
    GLuint m_rayShader;

    CubeMap *m_cubeMap;

    glm::mat4 m_currProj;
    glm::mat4 m_currView;
    glm::mat4 m_currScale;

    Shape *m_line;
    Shape *m_quad;
    Shape *m_cone;
    Shape *m_cube;
    Shape *m_cyl;
    Shape *m_sphere;
    Shape *m_faceCube;
    Shape *m_rayQuad;

    bool m_useCubeMap;
    bool m_usingAtlas;

    int m_w, m_h;

    glm::vec2 m_subImages;

    glm::mat4 m_frustum;
    ParticleEmitter *m_pe;

    QTime m_timer;
};

#endif // GRAPHICS_H
