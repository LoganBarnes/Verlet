#ifndef CHUNK_H
#define CHUNK_H

#include "graphics.h"
#include "point3d.h"
//#include "staticentity.h"
#include <QSet>

class Chunk// : public StaticEntity
{
public:
    Chunk(Point p, Point dim);
    virtual ~Chunk();

    void init(GLuint shader, char *blocks, QSet<int> drawables, float *vertexData, int numVerts);
    void setVBO(GLuint shader, QSet<int> drawables, float *vertexData, int numVerts);

    void updateBlock(int index, int drawable, char type);
    char *getBlocks(int& size);
    QSet<int> getDrawables();

    Point getLocation();
    glm::vec4 getLocationV();
    Point getDimension();
    glm::vec4 getDimensionV();

    void addBlock(Point p);
    void removeBlock(Point p);

    char getSingleBlock(int x, int y, int z);

    void onTick(float secs);
    void onDraw(Graphics *g);

    static int getIndex(int x, int y, int z, Point dim);
    static Point getPoint(int index, Point dim);
    inline int getNeighbor(Point block, Point dir);

//    virtual QList<Collision *> collides(Entity *e, float secs);
//    virtual void handleCollision(Collision *col);

//    virtual void onDrawOpaque(Graphics *g);
//    virtual void onDrawTransparent(Graphics *g);

//    bool contains(glm::vec3 point);

private:
//    Collision *checkCollisionY(CollisionShape *cs, glm::vec3 distance);

    Point m_p, m_dim;
    int m_size;

    char *m_blocks;
    QSet<int> m_drawables;

    Point m_neighbors[6];

    GLuint m_vaoID;
    GLuint m_vboID;
    int m_numVerts;
};

#endif // CHUNK_H
