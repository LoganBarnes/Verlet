#include "raytracer.h"
#include "ellipsoid.h"
#include "triangle.h"
#include "verlet.h"
#include "GL/glew.h"
//#include <qgl.h>
//#if defined(__APPLE__) || defined(MACOSX)
//    #include <OpenGL/glu.h>
//#else
//    #include <GL/glu.h>
//#endif

static glm::vec3 unProject(float winX, float winY, double *modelview, double *projection, int *viewport)
{
    double objX, objY, objZ;
    gluUnProject(winX, winY, 1, modelview, projection, viewport, &objX, &objY, &objZ);
    return glm::vec3(objX, objY, objZ);
}

RayTracer::RayTracer(const glm::vec3& s, const glm::vec3& d){
    source = s;
    direction = d;
}

// Read in the current modelview and projection matrices and remember them
RayTracer::RayTracer(double modelview[16], int mouseX, int mouseY)
{    
    //Mac doesn't seem to return the right values

    #if defined(__APPLE__) || defined(MACOSX)
        viewport[0]=0;
        viewport[1]=0;
        viewport[2]=800;
        viewport[3]=600;
    #else
        glGetIntegerv(GL_VIEWPORT, viewport);
    #endif
    update(modelview, mouseX, mouseY);

}

void RayTracer::update(double modelview[], int mouseX, int mouseY){
    //glGetIntegerv(GL_VIEWPORT, viewport);
    // Read camera information from OpenGL
    double projection[16];
    glGetDoublev(GL_PROJECTION_MATRIX, projection);

    // Reconstruct the eye position
    glm::vec3 xaxis(modelview[0], modelview[1], modelview[2]);
    glm::vec3 yaxis(modelview[4], modelview[5], modelview[6]);
    glm::vec3 zaxis(modelview[8], modelview[9], modelview[10]);
    glm::vec3 offset(modelview[12], modelview[13], modelview[14]);
    // Stores the origin in world space at the time of the constructor
    source = -glm::vec3(glm::dot(offset, xaxis), glm::dot(offset, yaxis), glm::dot(offset, zaxis));

    // Generate the four corner rays
    int xmin = viewport[0];
    int ymin = viewport[1];
    int xmax = xmin + viewport[2] - 1;
    int ymax = ymin + viewport[3] - 1;
    ray00 = unProject(xmin, ymin, modelview, projection, viewport) - source;
    ray10 = unProject(xmax, ymin, modelview, projection, viewport) - source;
    ray01 = unProject(xmin, ymax, modelview, projection, viewport) - source;
    ray11 = unProject(xmax, ymax, modelview, projection, viewport) - source;

    direction = getRayForPixel(mouseX, mouseY);
}

RayTracer::~RayTracer(){

}

glm::vec3 RayTracer::getRayForPixel(int x, int y) const
{
    // Construct the per-pixel ray using linear interpolation from the rays
    // along each corner of the frustum (the origin is the eye point: source)

    float fx = (float)(x - viewport[0]) / (float)viewport[2];
    float fy = 1 - (float)(y - viewport[1]) / (float)viewport[3];
//    glm::vec3 ray0 = glm::lerp(ray00, ray10, fx);
//    glm::vec3 ray1 = glm::lerp(ray01, ray11, fx);
//    return glm::vec3::lerp(ray0, ray1, fy).unit();
    return glm::vec3();
}

float RayTracer::hitPlane(const glm::vec3& point, const glm::vec3& normal){
    float denom = glm::dot(normal, direction);
    float t = glm::dot((point-source), normal) / denom;
    return t;
}

bool RayTracer::hitTriangle(Triangle *tri, HitTest &result){
    if(tri==NULL)
        return false;

//    float t = -(glm::dot(tri->normal, source-tri->vertices[1]))/(glm::dot(tri->normal, direction));

//    if(t>0){   //check if ray points towards triangle
//        glm::vec3 v1 = tri->vertices[0]-source;
//        glm::vec3 v2 = tri->vertices[1]-source;
//        glm::vec3 v3 = tri->vertices[2]-source;

//        glm::vec3 cross1 = v1.cross(v2);
//        glm::vec3 cross2 = v2.cross(v3);
//        glm::vec3 cross3 = v3.cross(v1);

        //check if point where ray intersects w/ plane triangle is on, is in triangle
//        if(!(cross1.dot(direction)>0) && !(cross2.dot(direction)>0) && !(cross3.dot(direction)>0)){
//            result.hit = true;
//            result.t = t;
//            result.normal = tri->normal;
//            result.tri = tri;
//            return true;
//        }
//    }
    return false;
}

bool RayTracer::hitMesh(std::vector<Triangle *> triangles, HitTest &result){
    bool h = false;
    for (int i = 0; i < triangles.size(); i++){
        Triangle* t = triangles.at(i);
        HitTest temp;
        bool hit = hitTriangle(t,temp);
        if(hit&&(temp.t<result.t)){
            h = true;
            result = temp;
        }
    }
    return h;
}

bool RayTracer::hitVerlet(Verlet* verlet, HitTest &result){
    bool h = false;
    glm::vec3 dim = verlet->rayTraceSize;
    for(int i = 0; i<verlet->getSize(); i++){
        glm::vec3 pos = verlet->getPoint(i);
        HitTest temp;
        bool hit = hitEllipsoid(pos,dim,i,temp);
        if(hit&&(temp.t<result.t)){
            h = true;
            result = temp;
        }
    }
    if(h)
        result.v = verlet;
    return h;
}

bool RayTracer::hitAABB(const glm::vec3 &cubeMin, const glm::vec3 &cubeMax, HitTest &result)
{
    // This uses the slab intersection method
    glm::vec3 tMin = (cubeMin - source) / direction;
    glm::vec3 tMax = (cubeMax - source) / direction;
    glm::vec3 t1 = glm::min(tMin, tMax);
    glm::vec3 t2 = glm::max(tMin, tMax);
    float tNear = 1.f;//t1.max();
    float tFar = 1.f;//t2.min();


    // Pick the intersection with the front faces (not the back faces)
    if (tNear > 0 && tNear < tFar) {

        const float epsilon = 1.0e-6;
        glm::vec3 hit = source + direction * tNear;
        if (hit.x < cubeMin.x + epsilon) result.normal = glm::vec3(-1, 0, 0);
        else if (hit.y < cubeMin.y + epsilon) result.normal = glm::vec3(0, -1, 0);
        else if (hit.z < cubeMin.z + epsilon) result.normal = glm::vec3(0, 0, -1);
        else if (hit.x > cubeMax.x - epsilon) result.normal = glm::vec3(1, 0, 0);
        else if (hit.y > cubeMax.y - epsilon) result.normal = glm::vec3(0, 1, 0);
        else result.normal = glm::vec3(0, 0, 1);
        result.hit = true;
        result.t = tNear;
        return true;
    }
    return false;
}

bool RayTracer::hitEllipsoid(const glm::vec3 &pos, const glm::vec3 &dim, int id, HitTest &result){
    //Translate circle center + ray source by -(circle center)
    glm::vec3 newSource = source-pos;
    //Scale world so ellipse is unit sphere- do not renormalize
    glm::vec3 basis = 1.f / dim;
    newSource*=basis;
    glm::vec3 newDir = direction*basis;

    //Unit sphere at origin: x^2+y^2+z^2 = 1
    float a = glm::dot(newDir, newDir);
    float b = 2.f*glm::dot(newDir, newSource);
    float c = glm::dot(newSource, newSource)-1;

    float t = solveQuadratic(a,b,c);

    if(t>0){
        result.hit = true;
        result.t = t;
        result.id = id;
        return true;
    }

    return false;
}

bool RayTracer::hitEllipsoid(Ellipsoid* e, HitTest &result)
{
    if(e==NULL)
        return false;

//    return hitEllipsoid(e->getPos(), e->getDimension(),0,result);
    return true;
}

float RayTracer::solveQuadratic(float a, float b, float c){
   float d = b*b-(4.0*a*c);

   if(d==0){   //1 intersection
       float t = -b/a;
       if(t>0)
           return t;
   }
   else if(d>0){  //2 intersections
       float square = sqrt(d);
       float r1 = (-b-square)/(2*a);
       float r2 = (-b+square)/(2*a);

       if(r1>r2){
           float temp = r2;
           r2=r1;
           r1=temp;
       }

       if(r1>0)
           return r1;
       else if(r2>0)
           return r2;
   }
   return 0;
}

