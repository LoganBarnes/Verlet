#ifndef VOXELMANAGER_H
#define VOXELMANAGER_H

#include "chunk.h"
#include "movableentity.h"
#include "manager.h"
#include <QHash>

class ChunkBuilder;
//class World;

class VoxelManager : public Manager
{
public:
    VoxelManager(Camera *cam, GLuint shader, Point dim, Point chunkSize, ChunkBuilder *cb);
    virtual ~VoxelManager();

    QList<Chunk*> getChunks();
    void setAtlas(QString atlasName);

    virtual void manage(World *world, float onTickSecs);
    virtual void onDraw(Graphics *g);

    void addChunk(Point pnt);
    void addBlock(Point p);
    void removeBlock(Point p);

    glm::vec3 castRay(glm::vec3 p, glm::vec3 dir, float& t, int& face);

    inline int getIndex(int x, int y, int z);
    int roundDown(int num, int multiple);

private:
    Point checkCenterPosition();
    void addBlocks(int xmin, int xmax, int ymin, int ymax, int zmin, int zmax);
    void removeBlocks(int xmin, int xmax, int ymin, int ymax, int zmin, int zmax);

    Collision *predictCollision(MovableEntity *me, float secs);
    Collision *checkCollision3D(CollisionShape *cs, glm::vec3 distance);
    void checkCollision1D(Collision *col, glm::vec3 pos, glm::vec3 dim, glm::vec3 dest,
                           float dir, int outer, int mid, int inner);

    QHash<Point, Chunk *> m_chunks;
    ChunkBuilder *m_chunkBuilder;

    QList<Point> m_chunksToAdd;
    QList<Point> m_chunksToRemove;

    Point m_chunkSize;
    Point m_dim;
    Point m_max, m_min;

    GLuint m_shader;
    Camera *m_camera;
    QString m_atlasName;

    float m_timer;
    bool m_loadOnTick;
};

#endif // VOXELMANAGER_H
