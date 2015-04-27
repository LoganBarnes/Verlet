#include "ray.h"
#include "camera.h"

#define GLM_FORCE_RADIANS
#include <gtc/matrix_transform.hpp>

#define EPS 0.0001f

Ray::Ray(Camera *camera)
    : m_camera(camera)
{
    m_p = glm::vec4(0,0,0,1);
    m_d = glm::vec4(0);
}

Ray::Ray(const glm::vec3& source, const glm::vec3& direction){
    m_source = source;
    m_direction = direction;
}


Ray::~Ray()
{
}


void Ray::setRay(float x, float y)
{
    glm::mat4 ftw = glm::inverse(m_camera->getScaleMatrix() * m_camera->getViewMatrix());
    glm::vec4 farWorld = ftw * glm::vec4(x, y, -1, 1);

    m_p = glm::inverse(m_camera->getViewMatrix()) * glm::vec4(0,0,0,1);
    m_d = glm::normalize(farWorld - m_p);
}

glm::vec4 Ray::intersectPoint(glm::vec3 point, float radius)
{
    glm::mat4 inv = glm::translate(glm::mat4(), -point);

    glm::vec4 p_shape = inv * m_p;
    glm::vec4 d_shape = inv * m_d;

    return intersectSphereAtOrigin(p_shape, d_shape, radius);
}

float Ray::hitPlane(const glm::vec3& point, const glm::vec3& normal){
    float denom = glm::dot(normal, glm::vec3(m_d));
    float t = glm::dot((point-glm::vec3(m_p)), normal) / denom;
    return t;
}

glm::vec3 Ray::getPointonPlane(const glm::vec3& point, const glm::vec3& normal){
    float denom = glm::dot(normal, m_direction);
    float t = glm::dot((point-m_source), normal) / denom;
    return m_source+m_direction*t;
}

//glm::vec4 Ray::intersectSphere(glm::mat4 invTrans)
//{

//}



// assumes radius size .5f
glm::vec4 Ray::intersectSphereAtOrigin(glm::vec4 p, glm::vec4 d, float radius)
{
    glm::vec4 n = glm::vec4(0.0, 0.0, 0.0, std::numeric_limits<float>::infinity());

    glm::vec4 v;
    float t1, t2;
    float a = d.x * d.x + d.y * d.y + d.z * d.z;
    float b = 2.f * p.x * d.x + 2.f * p.y * d.y + 2.f * p.z * d.z;
    float c = p.x * p.x + p.y * p.y + p.z * p.z - radius * radius;
    int tees = findT(a, b, c, &t1, &t2);

    if (tees > 0)
    {
        v = p + d * t1;
        if (t1 < 0.f)
            t1 = std::numeric_limits<float>::infinity();
        if (t1 < n.w)
        {
            v.w = t1;
            n = v;
        }
        if (tees > 1)
        {
            v = p + d * t2;
            if (t2 < 0.f)
                t2 = std::numeric_limits<float>::infinity();
            if (t2 < n.w)
            {
                v.w = t2;
                n = v;
            }
        }
    }
    return n;
}



int Ray::findT(float a, float b, float c, float *t1, float *t2)
{
    if (std::abs(a) < EPS)
    {
        *t1 = -c / b;
        return 1;
    }

    float disc = b * b - 4.f * a * c;

    // one solution
    if (std::abs(disc) < EPS)
    {
        *t1 = -b / (2.f * a);
        return 1;
    }

    // no solutions
    if (disc < 0.f)
        return 0;

    // two solutions (disc > 0)
    *t1 = (-b + sqrt(disc)) / (2.f * a);
    *t2 = (-b - sqrt(disc)) / (2.f * a);
    return 2;
}
