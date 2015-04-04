#include "chunk.h"
#include "movableentity.h"
#include "collisionshape.h"

Chunk::Chunk(Point p, Point dim)
{
    m_p = p;
    m_dim = dim;
    m_size = m_dim.x * m_dim.y * m_dim.z;

    m_neighbors[0] = Point( 0, 1, 0);
    m_neighbors[1] = Point( 1, 0, 0);
    m_neighbors[2] = Point( 0, 0, 1);
    m_neighbors[3] = Point(-1, 0, 0);
    m_neighbors[4] = Point( 0, 0,-1);
    m_neighbors[5] = Point( 0,-1, 0);

    m_blocks = 0;
    m_vaoID = 0;
    m_vboID = 0;
    m_blocks = 0; // gets set in init
}

Chunk::~Chunk()
{
    if (m_blocks)
        delete[] m_blocks;

    if (m_vaoID)
        glDeleteVertexArrays(1, &m_vaoID);
    if (m_vboID)
        glDeleteBuffers(1, &m_vboID);
}

void Chunk::init(GLuint shader, char *blocks, QSet<int> drawables, float *vertexData, int numVerts)
{
    m_blocks = blocks;
    setVBO(shader, drawables, vertexData, numVerts);
}


void Chunk::setVBO(GLuint shader, QSet<int> drawables, float *vertexData, int numVerts)
{
    m_drawables = QSet<int>(drawables);
    m_numVerts = numVerts;

    if (m_vaoID)
        glDeleteVertexArrays(1, &m_vaoID);
    if (m_vboID)
        glDeleteBuffers(1, &m_vboID);

    // Initialize the vertex array object.
    glGenVertexArrays(1, &m_vaoID);
    glBindVertexArray(m_vaoID);

    // Initialize the vertex buffer object.
    glGenBuffers(1, &m_vboID);
    glBindBuffer(GL_ARRAY_BUFFER, m_vboID);

    glBufferData(GL_ARRAY_BUFFER, 5 * m_numVerts * sizeof(GLfloat), vertexData, GL_STATIC_DRAW);

    GLuint position = glGetAttribLocation(shader, "position");
    GLuint texCoord = glGetAttribLocation(shader, "texCoord");

    glEnableVertexAttribArray(position);
    glVertexAttribPointer(
        position,
        3,                   // Num coordinates per position
        GL_FLOAT,            // Type
        GL_FALSE,            // Normalized
        sizeof(GLfloat) * 5, // Stride
        (void*) 0            // Array buffer offset
    );
    glEnableVertexAttribArray(texCoord);
    glVertexAttribPointer(
        texCoord,
        2,                              // Num coordinates per position
        GL_FLOAT,                       // Type
        GL_TRUE,                        // Normalized
        sizeof(GLfloat) * 5,            // Stride
        (void*) (sizeof(GLfloat) * 3)   // Array buffer offset
    );

    // Unbind buffers.
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Chunk::updateBlock(int index, int drawable, char type)
{
    m_blocks[index] = type;
    if (drawable) // at least one size needs to be rendered
        m_drawables.insert(index);
    else
        m_drawables.remove(index);
}

void Chunk::addBlock(Point p)
{
    assert(p.x >= 0 && p.x < m_dim.x);
    assert(p.y >= 0 && p.y < m_dim.y);
    assert(p.z >= 0 && p.z < m_dim.z);
    m_drawables.insert(getIndex(p.x, p.y, p.z, m_dim));
    m_blocks[getIndex(p.x, p.y, p.z, m_dim)] = 1; // temporary, gets reset in chunk builder
    m_size++;
}


void Chunk::removeBlock(Point p)
{
    assert(p.x >= 0 && p.x < m_dim.x);
    assert(p.y >= 0 && p.y < m_dim.y);
    assert(p.z >= 0 && p.z < m_dim.z);
    m_drawables.remove(getIndex(p.x, p.y, p.z, m_dim));
    m_blocks[getIndex(p.x, p.y, p.z, m_dim)] = 0;
    m_size--;

    if (p.z + 1 < m_dim.z && m_blocks[Chunk::getIndex(p.x, p.y, p.z + 1, m_dim)])
        m_drawables.insert(Chunk::getIndex(p.x, p.y, p.z + 1, m_dim));
    if (p.x + 1 < m_dim.x && m_blocks[Chunk::getIndex(p.x + 1, p.y, p.z, m_dim)])
        m_drawables.insert(Chunk::getIndex(p.x + 1, p.y, p.z, m_dim));
    if (p.z - 1 >= 0 && m_blocks[Chunk::getIndex(p.x, p.y, p.z - 1, m_dim)])
        m_drawables.insert(Chunk::getIndex(p.x, p.y, p.z - 1, m_dim));
    if (p.x - 1 >= 0 && m_blocks[Chunk::getIndex(p.x - 1, p.y, p.z, m_dim)])
        m_drawables.insert(Chunk::getIndex(p.x - 1, p.y, p.z, m_dim));
    if (p.x + 1 < m_dim.y && m_blocks[Chunk::getIndex(p.x, p.y + 1, p.z, m_dim)])
        m_drawables.insert(Chunk::getIndex(p.x, p.y + 1, p.z, m_dim));
    if (p.x - 1 >= 0 && m_blocks[Chunk::getIndex(p.x, p.y - 1, p.z, m_dim)])
        m_drawables.insert(Chunk::getIndex(p.x, p.y - 1, p.z, m_dim));
}


int Chunk::getNeighbor(Point block, Point dir)
{
    Point n = block + dir;
    if (n.x < 0 || n.y < 0 || n.z < 0 || n.x >= m_dim.x ||
            n.y >= m_dim.y || n.z >= m_dim.z)
        return 0;
    return m_blocks[(getIndex(n.x, n.y, n.z, m_dim))];
}

void Chunk::onTick(float) {}

Point Chunk::getLocation()
{
    return m_p;
}

glm::vec4 Chunk::getLocationV()
{
    return glm::vec4(m_p.x, m_p.y, m_p.z, 1.f);
}

Point Chunk::getDimension()
{
    return m_dim;
}

glm::vec4 Chunk::getDimensionV()
{
    return glm::vec4(m_dim.x, m_dim.y, m_dim.z, 0.f);
}

char* Chunk::getBlocks(int &size)
{
    size = m_size;
    return m_blocks;
}


QSet<int> Chunk::getDrawables()
{
    return m_drawables;
}

void Chunk::onDraw(Graphics *)
{
    glBindVertexArray(m_vaoID);
    glDrawArrays(GL_TRIANGLES, 0, m_numVerts);
    glBindVertexArray(0);
}


char Chunk::getSingleBlock(int x, int y, int z)
{
    assert( x >= 0 && x < m_dim.x);
    assert( y >= 0 && y < m_dim.y);
    assert( z >= 0 && z < m_dim.z);

    return m_blocks[getIndex(x, y, z, m_dim)];
}


int Chunk::getIndex(int x, int y, int z, Point dim)
{
    return z*dim.y*dim.x + x*dim.y + y;
}


Point Chunk::getPoint(int index, Point dim)
{
    Point p;
    p.y = index % dim.y;
    p.x = (index / dim.y) % dim.x;
    p.z = index / (dim.y * dim.x);
    return p;
}


