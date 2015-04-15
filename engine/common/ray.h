#ifndef RAY_H
#define RAY_H

#include <glm.hpp>

class Camera;

class Ray
{
public:
    Ray(Camera *camera);
    ~Ray();

    void setRay(float x, float y);

    glm::vec3 source;
    glm::vec3 direction;

private:
    int findT(float a, float b, float c, float *t1, float *t2);

    Camera *m_camera;
};

#endif // RAY_H
