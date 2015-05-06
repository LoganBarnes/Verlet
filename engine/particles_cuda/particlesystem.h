#ifndef PARTICLESYSTEM_H
#define PARTICLESYSTEM_H

#include <deque>
#include <vector>
#include "kernel.cuh"

typedef unsigned int GLuint;
typedef unsigned int uint;

class ParticleSystem
{
public:
    ParticleSystem(float particleRadius, uint3 gridSize, uint maxParticles, int3 minBounds, int3 maxBounds, int iterations);
    ~ParticleSystem();

    void update(float deltaTime, float3 playerPos, float playerRadius);
    void resetGrid();

    void addFluid(int3 ll, int3 ur, float mass, float density);
    void addParticleGrid(int3 ll, int3 ur, float mass, bool addJitter);
    void addHorizCloth(int2 ll, int2 ur, float3 spacing, float2 dist, float mass, bool holdEdges);
    void addRope(float3 start, float3 spacing, float dist, int numLinks, float mass, bool constrainStart);
    void addGrassBlade(float3 start, float3 spacing, float bendDist, int numLinks, float mass);
    void addGrass(int2 ll, int2 ur, float3 spacing);
    void addStaticSphere(int3 ll, int3 ur, float spacing);

    void addCollisionTriangleGroup(uint start, uint end, float3 center, float radius);
    void addCollisionTriangle(float3 a, float3 b, float3 c, float3 n);

    void setParticleToAdd(float3 pos, float3 vel, float mass);
    void setFluidToAdd(float3 pos, float3 color, float mass, float density);

    void makePointConstraint(uint index, float3 point);
    void makeDistanceConstraint(uint2 index, float distance);


    GLuint getCurrentReadBuffer() const { return m_posVbo; }
    uint getNumParticles() const { return m_numParticles; }
    float getParticleRadius() const { return m_particleRadius; }
    SimParams *getParams() { return &m_params; }
    uint getNumTris() { return m_numTris; }

    float4 mousePos;
    void addParticle(float4 pos, float4 vel, float mass, float ro, int phase);
    void toggleFreeze() { m_freeze = !m_freeze; }

private:
    void _init(uint numParticles, uint maxParticles);
    void _finalize();

    GLuint createVBO(uint size);
    void setArray(bool isVboArray, const float *data, int start, int count);

    void addParticles();

    void addFluids();
    void addFluidBlock();

    void addNewStuff();

    bool m_initialized;

    float m_particleRadius;

    uint m_maxParticles;
    uint m_numParticles;
//    uint m_numPointConstraints;
//    uint m_numDistanceConstraints;

    // GPU data
    float *m_dSortedPos;
    float *m_dSortedW;
    int   *m_dSortedPhase;

    // grid data for sorting method
    uint  *m_dGridParticleHash; // grid hash value for each particle
    uint  *m_dGridParticleIndex;// particle index for each particle
    uint  *m_dCellStart;        // index of start of each cell in sorted list
    uint  *m_dCellEnd;          // index of end of cell

    uint   m_gridSortBits;

//    uint *m_dPointConstraintIndex;
//    float *m_dPointConstraintPoint;

//    uint *m_dDistanceConstraintIndex;
//    float *m_dDistanceConstraint;

    // vertex buffer object for particle positions
    GLuint   m_posVbo;

    // handles OpenGL-CUDA exchange
    struct cudaGraphicsResource *m_cuda_posvbo_resource;

    // params
    SimParams m_params;
    uint3 m_gridSize;
    uint m_numGridCells;

    int m_rigidIndex;

    std::deque<float4> m_particlesToAdd;
    std::deque<float4> m_fluidsToAdd;

    int3 m_minBounds;
    int3 m_maxBounds;

    float m_prevTime;
    uint m_numTris;

    bool m_freeze;

    uint m_solverIterations;
};

#endif // PARTICLESYSTEM_H
