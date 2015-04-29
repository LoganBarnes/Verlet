#include "half.h"
#include "triangle.h"
#include "ray.h"

Half::Half()
{}

Half::Half(QList<Triangle *> tris, bool _top){
    top = _top;
    glm::vec3 min = glm::vec3(std::numeric_limits<float>::infinity(),
                  std::numeric_limits<float>::infinity(),
                  std::numeric_limits<float>::infinity());
    glm::vec3 max = glm::vec3(-std::numeric_limits<float>::infinity(),
                  -std::numeric_limits<float>::infinity(),
                  -std::numeric_limits<float>::infinity());

    foreach(Triangle* tri, tris){
        if(tri->normal.y>normalThreshold&&top){
            _triangles.push_back(tri);
            tri->adjustHitbox(min,max);
        }
        else if(tri->normal.y<normalThreshold&&!top){
            _triangles.push_back(tri);
            tri->adjustHitbox(min,max);
        }
    }
    calcHitbox(min,max);
    createFlat();
}


Half::~Half()
{}

void Half::calcHitbox(const glm::vec3& min, const glm::vec3& max){
    glm::vec3 average = .5f*(min+max);
    center = glm::vec2(average.x,average.z);
    float diffx = max.x-min.x;
    float diffz = max.z-min.z;
    radius = (diffx>diffz) ? diffx*.5 : diffz*.5;
    yLimits = glm::vec2(min.y,max.y);
}

//*******************Finding points on the surface***********************//
bool Half::inHitBox(const glm::vec3& pos){
    if(pos.x>center.x-radius&&pos.x<center.x+radius&&
            pos.z>center.y-radius&&pos.z<center.y+radius)
        return true;
    return false;
}

bool Half::placeOnSurface(glm::vec3 &surfacePt){
    if(!(surfacePt.y>yLimits.x && surfacePt.y<yLimits.y))
        return false;
    if(surfacePt.x>center.x+radius||surfacePt.x<center.x-radius)
        return false;
    if(surfacePt.z>center.y+radius||surfacePt.z<center.y-radius)
        return false;
    int flatID = getFlatTriangle(surfacePt.x,surfacePt.z);
    if(flatID>0){
        Triangle* tri = _triangles[flatID];
        if(top){
            Ray r = Ray(surfacePt,glm::vec3(0,-1,0));
            glm::vec3 pointOnSurface = r.getPointonPlane(tri->vertices[0],tri->normal)+glm::vec3(0,allowance,0);
            if(surfacePt.y<pointOnSurface.y){
                surfacePt = pointOnSurface;
                return true;
            }
        }
        else if(!top){
            Ray r = Ray(surfacePt,glm::vec3(0,1,0));
            glm::vec3 pointOnSurface = r.getPointonPlane(tri->vertices[0],tri->normal)+glm::vec3(0,-allowance,0);
            if(surfacePt.y>pointOnSurface.y){
                surfacePt = pointOnSurface;
                return true;
            }
        }

    }
//    else
        return false;
}

bool Half::findY(const glm::vec2 &coord, float& y){
    int flatID = getFlatTriangle(coord.x,coord.y);
    if(flatID>0){
        Triangle* tri = _triangles[flatID];
        Ray r = (top) ?
                    Ray(glm::vec3(coord.x,1000,coord.y),glm::vec3(0,-1,0)):
                    Ray(glm::vec3(coord.x,-1000,coord.y),glm::vec3(0,1,0));
        glm::vec3 pointOnSurface = r.getPointonPlane(tri->vertices[0],tri->normal);
        y = pointOnSurface.y;
        return true;
    }
    else
        return false;
}

//*******************2D projection***********************//
void Half::createFlat(){
    float height = (top) ? yLimits.y : yLimits.x;
    for(uint i = 0; i<_triangles.size(); i++){
        Triangle* t = _triangles[i];
        glm::vec3 a = t->vertices[0]; a.y=height;
        glm::vec3 b = t->vertices[1]; b.y=height;
        glm::vec3 c = t->vertices[2]; c.y=height;

        Triangle flat = Triangle(a,b,c);
        flatTri.push_back(flat);
    }
}

int Half::getFlatTriangle(float x, float z){
    for(unsigned int i = 0; i<flatTri.size(); i++){
        Triangle tri = flatTri[i];
        glm::vec3 edge0 = tri.vertices[2]-tri.vertices[0];
        glm::vec2 v0 = glm::vec2(edge0.x,edge0.z);
        glm::vec3 edge1 = tri.vertices[1]-tri.vertices[0];
        glm::vec2 v1 = glm::vec2(edge1.x,edge1.z);
        glm::vec2 v2 = glm::vec2(x-tri.vertices[0].x,z-tri.vertices[0].z);

        //Compute dot products
        float dot00 = glm::dot(v0,v0);
        float dot01 = glm::dot(v0,v1);
        float dot02 = glm::dot(v0,v2);
        float dot11 = glm::dot(v1,v1);
        float dot12 = glm::dot(v1,v2);

        //Compute barycentric coordinates
        float invDenom = 1.0/ (dot00*dot11-dot01*dot01);
        float u = (dot11*dot02-dot01*dot12) * invDenom;
        float v = (dot00*dot12-dot01*dot02) * invDenom;

        if(u>=0&&v>=0&&u+v<1)
            return i;
    }
    return -1;
}



