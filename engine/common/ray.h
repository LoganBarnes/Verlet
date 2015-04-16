#ifndef RAY_H
#define RAY_H

#include <glm.hpp>

class Camera;

class Ray
{
public:
    Ray(Camera *camera);
    virtual ~Ray();

    glm::vec3 getSource() { return glm::vec3(m_p); }
    glm::vec3 getDirection() { return glm::vec3(m_d); }

    void setRay(float x, float y);
    glm::vec4 intersectPoint(glm::vec3 point, float radius);
//    glm::vec4 intersectSphere(glm::mat4 invTrans);

    glm::vec3 getPoint(float t) {
        return t*getDirection()+getSource();}
    float hitPlane(const glm::vec3& point, const glm::vec3& normal);
private:
    // assumes radius size .5f
    glm::vec4 intersectSphereAtOrigin(glm::vec4 p, glm::vec4 d, float radius);
    int findT(float a, float b, float c, float *t1, float *t2);

    glm::vec4 m_p;
    glm::vec4 m_d;

    Camera *m_camera;
};

#endif // RAY_H
