#include "ray.h"
#include "camera.h"

#define EPS 0.0001f

Ray::Ray(Camera *camera)
    : m_camera(camera)
{
}

Ray::~Ray()
{
}


void Ray::setRay(float x, float y)
{
    glm::mat4 ftw = glm::inverse(m_camera->getScaleMatrix() * m_camera->getViewMatrix());
    glm::vec4 eye = glm::inverse(m_camera->getViewMatrix()) * glm::vec4(0,0,0,1);

    glm::vec4 farWorld = ftw * glm::vec4(x, y, -1, 1);

    source = glm::vec3(eye);
    direction = glm::normalize(glm::vec3(farWorld - eye));
}







int Ray::findT(float a, float b, float c, float *t1, float *t2)
{
    // intersections, intersectDist1, intersectDist2
    glm::vec3 t = glm::vec3(0, 0, std::numeric_limits<float>::infinity());

    if (abs(a) < EPS)
    {
        *t1 = -c / b;
        return 1;
    }

    float disc = b * b - 4.f * a * c;

    // one solution
    if (abs(disc) < EPS)
    {
        *t1 = -b / (2.f * a);
        t.x = 0.f;
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
