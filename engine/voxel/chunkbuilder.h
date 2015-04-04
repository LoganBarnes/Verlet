#ifndef CHUNKBUILDER_H
#define CHUNKBUILDER_H

#include <GL/glew.h>

struct Point;
class Chunk;

class ChunkBuilder
{
public:
    ChunkBuilder(int seed) { m_seed = seed; }
    virtual ~ChunkBuilder() {}

    virtual Chunk *getChunk(GLuint shader, Point p, Point dim) = 0;
    virtual void resetChunk(GLuint shader, Chunk* chunk, Point dim) = 0;

protected:
    int m_seed;

};

#endif // CHUNKBUILDER_H
