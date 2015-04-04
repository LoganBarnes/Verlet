#include "voxelmanager.h"
#include "chunkbuilder.h"
#include <QHash>
#include "movableentity.h"
#include "collisionshape.h"
#include "world.h"

#define GLM_FORCE_RADIANS
#include <glm/gtc/type_ptr.hpp>

VoxelManager::VoxelManager(Camera *cam, GLuint shader, Point dim, Point chunkSize, ChunkBuilder *cb)
    : Manager(SPARSE)
{
    m_camera = cam;
    m_shader = shader;
    m_dim = dim;
    m_chunkSize = chunkSize;

    m_chunkBuilder = cb;

    glm::vec3 cent = glm::round(glm::vec3(m_camera->getEye()));
    Point center = Point(roundDown(cent.x, m_chunkSize.x), roundDown(cent.y, m_chunkSize.y), roundDown(cent.z, m_chunkSize.z));
    center = Point(center.x / m_chunkSize.x, center.y / m_chunkSize.y, center.z / m_chunkSize.z);

    m_min = center;
    m_min.y -= dim.y;
    m_max = center;
    m_max.y += dim.y;

    m_chunks.clear();
    m_chunksToAdd.clear();
    m_chunksToRemove.clear();

    addBlocks(center.x, center.x, m_min.y, m_max.y, center.z, center.z);

    m_timer = -.1;
    m_loadOnTick = true;
}

VoxelManager::~VoxelManager()
{
    foreach(Chunk *chunk, m_chunks)
        delete chunk;

    delete m_chunkBuilder;
}


void VoxelManager::addChunk(Point pnt)
{
    Chunk *chunk = m_chunkBuilder->getChunk(m_shader, pnt, m_chunkSize);

    Point p = chunk->getLocation();
    m_chunks.insert(p, chunk);

}


QList<Chunk*> VoxelManager::getChunks()
{
    return m_chunks.values();
}


void VoxelManager::setAtlas(QString atlasName)
{
    m_atlasName = atlasName;
}


void VoxelManager::onDraw(Graphics *g)
{
    g->setAtlas(m_atlasName);

    glm::mat4 frust = g->getFrustum();

    glm::vec4 planes[6];
    planes[0] = frust[3] - frust[0]; // negx
    planes[1] = frust[3] - frust[1]; // negy
    planes[2] = frust[3] - frust[2]; // negz
    planes[3] = frust[3] + frust[0]; // posx
    planes[4] = frust[3] + frust[1]; // posy
    planes[5] = frust[3] + frust[2]; // posz


    glm::mat4 trans = glm::mat4();

    glm::vec4 pos, dim;
    glm::vec4 plan, corn;
    glm::vec4 corners[8];

    foreach (Chunk *c, m_chunks)
    {
        pos = c->getLocationV();
        dim = c->getDimensionV();

        corners[0] = pos + dim * glm::vec4(-1,-1,-1, 0 );
        corners[1] = pos + dim * glm::vec4( 1,-1,-1, 0 );
        corners[2] = pos + dim * glm::vec4(-1, 1,-1, 0 );
        corners[3] = pos + dim * glm::vec4(-1,-1, 1, 0 );
        corners[4] = pos + dim * glm::vec4(-1, 1, 1, 0 );
        corners[5] = pos + dim * glm::vec4( 1,-1, 1, 0 );
        corners[6] = pos + dim * glm::vec4( 1, 1,-1, 0 );
        corners[7] = pos + dim * glm::vec4( 1, 1, 1, 0 );

        int numOutside;
        bool render = true;
        for (int p = 0; p < 6; p++)
        {
            plan = planes[p];
            numOutside = 0;

            for (int c = 0; c < 8; c++)
            {
                corn = corners[c];

                // inside plane
                if (glm::dot(plan, corn) > 0.f)
                {
                    break;
                }
                numOutside++;
            }
            if (numOutside >= 8)
            {
                render = false;
                break;
            }
        }

        if (render)
        {
            trans[3] = pos;
            glUniformMatrix4fv(glGetUniformLocation(m_shader, "model"), 1, GL_FALSE, glm::value_ptr(trans));
            c->onDraw(g);
        }
    }
}

void VoxelManager::manage(World *world, float onTickSecs)
{
    m_loadOnTick = true;

    if (m_timer > 0)
        m_timer -= onTickSecs;

    Point center = checkCenterPosition();

    foreach (Point p, m_chunksToRemove)
    {
        m_chunksToAdd.removeAll(p);
        Chunk *c = m_chunks.take(p);
        if (c)
            delete c;
    }
    m_chunksToRemove.clear();


    QList<Collision *> cols;
    cols.clear();
    QList<MovableEntity *> mes = world->getMovableEntities();

    Collision *col;
    foreach(MovableEntity *me, mes)
    {
        col = predictCollision(me, onTickSecs);
        if (col)
            cols.append(col);
    }

    foreach(Collision *col, cols)
    {
        col->e1->handleCollision(col);
        delete col;
    }

    glm::vec3 pos = glm::vec3(m_camera->getEye());
    int border = 5;

    glm::bvec4 b = glm::bvec4(
                pos.x > center.x + m_chunkSize.x - border,
                pos.x < center.x + border,
                pos.z > center.z + m_chunkSize.z - border,
                pos.z < center.z + border);

    if (m_loadOnTick)
    {
        // add current chunk if it isn't loaded
        if (!m_chunks.contains(center))
            addChunk(center);

        // add nearby chunks as the player approaches the edges
        else if (b.x && !m_chunks.contains(center + Point(m_chunkSize.x, 0, 0)))
            addChunk(center + Point(m_chunkSize.x, 0, 0));
        else if (b.y && !m_chunks.contains(center + Point(-m_chunkSize.x, 0, 0)))
            addChunk(center + Point(-m_chunkSize.x, 0, 0));
        else if (b.z && !m_chunks.contains(center + Point(0, 0, m_chunkSize.z)))
            addChunk(center + Point(0, 0, m_chunkSize.z));
        else if (b.w && !m_chunks.contains(center + Point(0, 0, -m_chunkSize.z)))
            addChunk(center + Point(0, 0, -m_chunkSize.z));

        // add nearby chunks as the player approaches the corners
        else if (b.x and b.z && !m_chunks.contains(center + Point(m_chunkSize.x, 0, m_chunkSize.z)))
            addChunk(center + Point(m_chunkSize.x, 0, m_chunkSize.z));
        else if (b.x and b.w && !m_chunks.contains(center + Point(m_chunkSize.x, 0, -m_chunkSize.z)))
            addChunk(center + Point(m_chunkSize.x, 0, -m_chunkSize.z));
        else if (b.y and b.z && !m_chunks.contains(center + Point(-m_chunkSize.x, 0, m_chunkSize.z)))
            addChunk(center + Point(-m_chunkSize.x, 0, m_chunkSize.z));
        else if (b.y and b.w && !m_chunks.contains(center + Point(-m_chunkSize.x, 0, -m_chunkSize.z)))
            addChunk(center + Point(-m_chunkSize.x, 0, -m_chunkSize.z));

        // add a chunk at the bottom if the player is approaching the next chunk
        else if (pos.y < center.y + border * 2 && !m_chunks.contains(center + Point(0, -m_chunkSize.y, 0)))
            addChunk(center + Point(0, -m_chunkSize.y, 0));

        // add all the other chunks waiting to be loaded
        else if (!m_chunksToAdd.isEmpty())
            addChunk(m_chunksToAdd.takeFirst());

    }

}

Point VoxelManager::checkCenterPosition()
{
    glm::vec3 cent = glm::round(glm::vec3(m_camera->getEye()));
    Point centerPoint = Point(roundDown(cent.x, m_chunkSize.x), roundDown(cent.y, m_chunkSize.y), roundDown(cent.z, m_chunkSize.z));
    Point center = Point(centerPoint.x / m_chunkSize.x, centerPoint.y / m_chunkSize.y, centerPoint.z / m_chunkSize.z);

    Point diffMin = center - m_min;
    Point diffMax = m_max - center;

    if (diffMin.z < m_dim.z)
    { m_min.z -= 1; addBlocks(m_min.x, m_max.x, m_min.y, m_max.y, m_min.z, m_min.z); }
    else if (diffMin.z > m_dim.z + 1)
    { removeBlocks(m_min.x, m_max.x, m_min.y, m_max.y, m_min.z, m_min.z); m_min.z += 1; }
    if (diffMax.z < m_dim.z)
    { m_max.z += 1; addBlocks(m_min.x, m_max.x, m_min.y, m_max.y, m_max.z, m_max.z); }
    else if (diffMax.z > m_dim.z + 1)
    { removeBlocks(m_min.x, m_max.x, m_min.y, m_max.y, m_max.z, m_max.z); m_max.z -= 1; }
    if (diffMin.x < m_dim.x)
    { m_min.x -= 1; addBlocks(m_min.x, m_min.x, m_min.y, m_max.y, m_min.z, m_max.z); }
    else if (diffMin.x > m_dim.x + 1)
    { removeBlocks(m_min.x, m_min.x, m_min.y, m_max.y, m_min.z, m_max.z); m_min.x += 1; }
    if (diffMax.x < m_dim.x)
    { m_max.x += 1; addBlocks(m_max.x, m_max.x, m_min.y, m_max.y, m_min.z, m_max.z); }
    else if (diffMax.x > m_dim.x + 1)
    { removeBlocks(m_max.x, m_max.x, m_min.y, m_max.y, m_min.z, m_max.z); m_max.x -= 1; }

    return centerPoint;
}

void VoxelManager::addBlocks(int xmin, int xmax, int ymin, int ymax, int zmin, int zmax)
{
    Point p;
    for (int y = ymin; y <= ymax; y++)
    {
        for (int z = zmin; z <= zmax; z++)
        {
            for (int x = xmin; x <= xmax; x++)
            {
                p = Point(x, y, z) * m_chunkSize;
                if (!m_chunks.contains(p))
                    m_chunksToAdd.append(p);
            }
        }
    }
}

void VoxelManager::removeBlocks(int xmin, int xmax, int ymin, int ymax, int zmin, int zmax)
{
    Point p;
    for (int y = ymin; y <= ymax; y++)
    {
        for (int z = zmin; z <= zmax; z++)
        {
            for (int x = xmin; x <= xmax; x++)
            {
                p = Point(x, y, z) * m_chunkSize;
                    m_chunksToRemove.append(p);
            }
        }
    }

}

Collision *VoxelManager::predictCollision(MovableEntity *me, float secs)
{
    QList<CollisionShape *> cshapes = me->getCollisionShapes();
    Collision *col = NULL;

    glm::vec3 distance = me->getVelocity() * secs;

    foreach(CollisionShape *cs, cshapes)
    {
        col = checkCollision3D(cs, distance);
        if (col)
        {
            col->e1 = me;
            glm::vec3 diff = glm::abs(col->mtv - distance);
            glm::vec3 vel = me->getVelocity();
            if (diff.x > 0.0000001f)
                vel.x = 0;
            if (diff.y > 0.0000001f)
                vel.y = 0;
            if (diff.z > 0.0000001f)
                vel.z = 0;
            me->setVelocity(vel);
            break;
        }
    }

    return col;
}

Collision *VoxelManager::checkCollision3D(CollisionShape *cs, glm::vec3 distance)
{
    Collision *col;
    col = new Collision();
    col->mtv = distance;
    col->impulse = glm::vec3(-1.f);

    glm::vec3 dir = glm::vec3(
                (distance.x >= 0 ? 1 : -1),
                (distance.y >= 0 ? 1 : -1),
                (distance.z >= 0 ? 1 : -1));

    glm::vec3 dim = cs->getDim() * .5f;
    glm::vec3 pos = cs->getPos();
    glm::vec3 dest;

    if (abs(distance.x) < abs(distance.z))
    {
        // check x
        dest = pos + glm::vec3(distance.x, 0, 0);
        checkCollision1D(col, pos, dim, dest, dir.x, 0, 1, 2);
        pos.x += col->mtv.x;

        // check z
        dest = pos + glm::vec3(0, 0, distance.z);
        checkCollision1D(col, pos, dim, dest, dir.z, 2, 0, 1);
        pos.z += col->mtv.z;
    }
    else
    {
        // check z
        dest = pos + glm::vec3(0, 0, distance.z);
        checkCollision1D(col, pos, dim, dest, dir.z, 2, 0, 1);
        pos.z += col->mtv.z;

        // check x
        dest= pos + glm::vec3(distance.x, 0, 0);
        checkCollision1D(col, pos, dim, dest, dir.x, 0, 1, 2);
        pos.x += col->mtv.x;
    }

    // check y
    dest = pos + glm::vec3(0, distance.y, 0);
    checkCollision1D(col, pos, dim, dest, dir.y, 1, 2, 0);

    return col;
}

void VoxelManager::checkCollision1D(Collision *col, glm::vec3 pos, glm::vec3 dim, glm::vec3 dest,
                           float dir, int outer, int mid, int inner)
{
    float bump = 0.5001f;

    glm::vec3 minBlocks = glm::round(pos - dim);
    glm::vec3 maxBlocks = glm::round(pos + dim);

    glm::vec3 nearBlocks = glm::round(pos + dim * dir);
    glm::vec3 farBlocks = glm::round(dest + dim * dir);

    Point bp;
    Chunk *c;
    int far = (int) (farBlocks[outer] + dir);
    for (int i = (int) nearBlocks[outer]; i != far; i += (int) dir)
    {
        for (int j = (int) minBlocks[mid]; j <= maxBlocks[mid]; j++)
        {
            for (int k = (int) minBlocks[inner]; k <= maxBlocks[inner]; k++)
            {
                int x, y, z;
                switch(outer) { case 0: x=i; break; case 1: y=i; break; case 2: z=i; break; }
                switch(mid) { case 0: x=j; break; case 1: y=j; break; case 2: z=j; break; }
                switch(inner) { case 0: x=k; break; case 1: y=k; break; case 2: z=k; break; }

                bp = Point(roundDown(x, m_chunkSize.x), roundDown(y, m_chunkSize.y), roundDown(z, m_chunkSize.z));
                c = m_chunks.value(bp, NULL);

                // load the block if it isn't already loaded
                if (c == NULL)
                {
                    float yPos = bp.y / m_chunkSize.y;
                    if (yPos <= m_dim.y && yPos >= -m_dim.y)
                    {
                        addChunk(bp);
                        m_chunksToAdd.removeAll(bp);
                    }
                    c = m_chunks.value(bp, NULL);
                    m_loadOnTick = false;
                }

                if (c && c->getSingleBlock(x - bp.x, y - bp.y, z - bp.z))
                {
                    col->mtv[outer] = farBlocks[outer] - ((pos[outer] + dim[outer] * dir));
                    col->mtv[outer] += (col->mtv[outer] >= 0.f ? -bump : bump);

                    col->impulse[outer] = 1.f;
                    return;
                }
            }
        }
    }
}

glm::vec3 VoxelManager::castRay(glm::vec3 p, glm::vec3 dir, float &t, int &face)
{
    t = 0;
    face = 0;

    Point step = Point( (dir.x > 0 ? 1 : -1),
                        (dir.y > 0 ? 1 : -1),
                        (dir.z > 0 ? 1 : -1));

    glm::vec3 tDelta = glm::abs(glm::vec3(1.f / dir.x, 1.f / dir.y, 1.f / dir.z));
    Point point = Point((int) glm::round(p.x), (int) glm::round(p.y), (int) glm::round(p.z));

    Point next = point + step;
    glm::vec3 nextf = glm::vec3(next.x - .5f * step.x, next.y - .5f * step.y, next.z - .5f * step.z);
    glm::vec3 tMax = glm::abs(nextf - p) * tDelta;

    Point bp = Point(roundDown(point.x, m_chunkSize.x), roundDown(point.y, m_chunkSize.y), roundDown(point.z, m_chunkSize.z));
    Chunk *c = m_chunks.value(bp, NULL);

    while(c)
    {
        if (c->getSingleBlock(point.x - bp.x, point.y - bp.y, point.z - bp.z))
            break;
        if(tMax.x < tMax.y)
        {
            if(tMax.x < tMax.z) {
                point.x += step.x;
                tMax.x += tDelta.x;
                face = 0b001000;
            } else {
                point.z += step.z;
                tMax.z += tDelta.z;
                face = 0b100000;
            }
        } else {
            if(tMax.y < tMax.z) {
                point.y += step.y;
                tMax.y += tDelta.y;
                face = 0b000010;
            } else {
                point.z += step.z;
                tMax.z += tDelta.z;
                face = 0b100000;
            }
        }
        bp = Point(roundDown(point.x, m_chunkSize.x), roundDown(point.y, m_chunkSize.y), roundDown(point.z, m_chunkSize.z));
        c = m_chunks.value(bp, NULL);
    }

    t = 0.f;

    if (step.z > 0 && face == 0b100000)
        face = 0b010000;
    else if (step.x > 0 && face == 0b001000)
        face = 0b000100;
    else if (step.y > 0 && face == 0b000010)
        face = 0b000001;

    return glm::vec3(point.x , point.y, point.z);
}

int VoxelManager::roundDown(int num, int multiple)
{
//    if (multiple == 0)
//        return num;
    int r = abs(num) % multiple;
    if (r == 0)
        return num;
    if (num < 0)
        return -(abs(num) + multiple - r);
    return num - r;
}

void VoxelManager::addBlock(Point p)
{
    Point bp = Point(roundDown(p.x, m_chunkSize.x), roundDown(p.y, m_chunkSize.y), roundDown(p.z, m_chunkSize.z));
    Chunk *c = m_chunks.value(bp, NULL);
    if (c)
    {
        c->addBlock(p - bp);
        m_chunkBuilder->resetChunk(m_shader, c, m_chunkSize);
    }
}

void VoxelManager::removeBlock(Point p)
{
    Point bp = Point(roundDown(p.x, m_chunkSize.x), roundDown(p.y, m_chunkSize.y), roundDown(p.z, m_chunkSize.z));
    Chunk *c = m_chunks.value(bp, NULL);
    if (c)
    {
        c->removeBlock(p - bp);
        m_chunkBuilder->resetChunk(m_shader, c, m_chunkSize);
    }
}


int VoxelManager::getIndex(int x, int y, int z)
{
    return z*m_chunkSize.y*m_chunkSize.x + x*m_chunkSize.y + y;
}

