#include "trianglemesh.h"
#include "verletmanager.h"
#include "mesh.h"
#include "graphics.h"

#define GLM_FORCE_RADIANS
#include <gtc/type_ptr.hpp>

#include "debugprinting.h"

//******************************CONSTRUCTING**********************************//
TriangleMesh::TriangleMesh(const glm::vec2& dimension, float w,
      const glm::vec3& start, VerletManager* vm, GLuint shader)
    : Verlet(vm),
    m_shader(shader)
{
    int r = dimension.y;
    int c = dimension.x;

    float half_w = w*.5;
    float h = (sqrt(3)/2.0) * w;

    glm::vec3 width = glm::vec3(w,0,0);
    glm::vec3 height = glm::vec3(0,-h,0);
    glm::vec3 half_width = glm::vec3(half_w,0,0);

    //Create points
    for(int i =0; i<r; i++){
        for(int j = 0; j<c; j++){
            if(i%2==0){  //even
                if(j==c-1)
                    createPoint(start+half_width+((float)j-1)*width+(float)i*height);
                else
                    createPoint(start+(float)j*width+(float)i*height);
            }
            else{  //odd
                if(j==0)
                    createPoint(start+(float)i*height);
                else
                    createPoint(half_width+start+((float)j-1)*width+(float)i*height);
            }
        }
    }

    //Create structural links
    for(int i =0; i<r; i++){
        for(int j = 0; j<c; j++){
            int index = j+i*c;
            if(j<c-1)  //horizontal
                createLink(index, index+1);
            if(i<r-1)  //vertical
                createLink(index, index+c);
            if(j<c-1&&i<r-1&&i%2==0) //diagonal on even rows
                createLink(index, index+c+1);
            if(j>0&&i<r-1&&i%2==1) //diagonal on odd rows
                createLink(index, index+c-1);
        }
    }

    //Triangulate
    for(int i =0; i<r-1; i++){
        for(int j = 0; j<c; j++){
            if(i%2==0){  //even
                if(j<c-1){
                    //upper triangles
                    int n = c*i+j;
                    triangulate(n,n+c+1,n+c);
                    //lower triangles
                    triangulate(n,n+1,n+c+1);
                }
            }
            else{  //odd
                if(j!=0){
                    int n = c*i+j;
                    //upper triangles
                    triangulate(n,n+c,n+c-1);
                    //lower triangles
                    triangulate(n,n+c-1,n-1);
                }
            }
        }
    }

    //Create shear and bend constraints
    for(int i = 0; i<r; i++){
        for(int j = 0; j<c; j++){
            if(j+2<c)  //horizontal
                createShearLink(j+(i*c),(i*c)+j+2,j+1+(i*c));
            if(i+2<r && (j==0 || j==c-1)) //vertical, at edges
                createShearLink(j+(i*c),(i+2)*c+j,(i+1)*c+j);
            if(i+2<r && j+1<c){ //diagonal right-down
                if(j+1!=c-1&& (j!=0||i%2==0)){
                    if(i%2==0)
                        createShearLink(j+c*i,j+1+c*(i+2),(i+1)*c+j+1);
                    else
                        createShearLink(j+c*i,j+1+c*(i+2),(i+1)*c+j);
                }
                else if(j+1==c-1 && i%2==1)
                    createShearLink(j+c*i,j+1+c*(i+2),(i+1)*c+j);
            }
            if(i+2<r && j>0 && j-1<c){ //diagonal left-down
                if((!(j==c-1&&i%2==0)) && (!(j==1&&i%2!=0))){
                    if(i%2==1)
                        createShearLink(j+c*i,j-1+c*(i+2),(i+1)*c+j-1);
                    else
                        createShearLink(j+c*i,j-1+c*(i+2),(i+1)*c+j);
                }
            }
        }
    }


    //set scalar
    for(int i=0; i<numPoints; i++)
        _scalar[i]=1/(float)numTri[i];

    m_mesh = new Mesh();
//    m_mesh->initStrip(m_shader, c, r, getPosArray(), getNormArray());
    m_mesh->initTriangles(m_shader, _triangles, getPosArray());
}

TriangleMesh::~TriangleMesh()
{
    delete m_mesh;
    for (std::vector<Tri*>::iterator it = _triangles.begin() ; it != _triangles.end(); ++it)
        delete (*it);
    _triangles.clear();
}


void TriangleMesh::triangulate(int a, int b, int c){
    Tri* t = new Tri(a,b,c);
    numTri[a]+=1;
    numTri[b]+=1;
    numTri[c]+=1;
    calculate(t);
    _triangles.push_back(t);

    t->edges[0] = findLink(a,b);
    t->edges[1] = findLink(b,c);
    t->edges[2] = findLink(c,a);

    link_to_tri[t->edges[0]]+=t;
    link_to_tri[t->edges[1]]+=t;
    link_to_tri[t->edges[2]]+=t;
}

void TriangleMesh::calculate(Tri* t){
    t->vertices[0]=_pos[t->a];
    t->vertices[1]=_pos[t->b];
    t->vertices[2]=_pos[t->c];

    t->normal =  glm::cross((t->vertices[1] - t->vertices[0]), (t->vertices[2] - t->vertices[0]));
    t->normal = glm::normalize(t->normal);

    _normal[t->a]+=t->normal*_scalar[t->a];
    _normal[t->b]+=t->normal*_scalar[t->b];
    _normal[t->c]+=t->normal*_scalar[t->c];
}

//******************************EDITING SHEAR**********************************//
Link* TriangleMesh::createShearLink(int a, int b, int c, Link* seg1, Link* seg2){
    float length = glm::length(_pos[b]-_pos[a]);
    Link* l = new Link(a, b, c, length);
    links.push_back(l);

    shear_map[a]+=l;
    shear_map[b]+=l;
    crossover_map[c]+=l;

    if(seg1==NULL&&seg2==NULL){
        seg1 = findLink(a,c);
        seg2 = findLink(b,c);
    }

    shear_to_link[l]+=seg1;
    shear_to_link[l]+=seg2;

    link_to_shear[seg1]+=l;
    link_to_shear[seg2]+=l;

    return l;
}

void TriangleMesh::removeShear(Link* s){
    //remove link from 'shear_map' of a and b
    removeFromHash(s->pointA,s,shear_map);
    removeFromHash(s->pointB,s,shear_map);
    //remove link from 'crossover_map' of c
    removeFromHash(s->pointC,s,crossover_map);
    //remove shear from 'link_to_shear' of its segments
    QList<Link*> segments = shear_to_link[s];
    removeFromHash(segments[0],s,link_to_shear);
    removeFromHash(segments[1],s,link_to_shear);
    //remove shear from 'shear_to_link'
    shear_to_link.remove(s);
    //remove link from 'links'
    links.erase(std::remove(links.begin(), links.end(), s), links.end());
    delete s;
}
//********************************TEARING*************************************//

//********************************UPDATING*************************************//
void TriangleMesh::applyWind(Tri* t){
    //wind has full effect to perpendicular cloth, none on parallel cloth
    glm::vec3 windDirection = _manager->wind;
    float windScalar =  glm::dot(windDirection, t->normal);

    if(windScalar<0)
        windScalar*=-1;
    t->windForce = windScalar;

    glm::vec3 windForce = windDirection*windScalar*_manager->windPow;

    _acc[t->a] += windForce;
    _acc[t->b] += windForce;
    _acc[t->c] += windForce;
}

void TriangleMesh::onTick(float ){
    for(unsigned int i = 0; i<_triangles.size(); i++){
        calculate(_triangles.at(i));
        applyWind(_triangles.at(i));
    }
}

void TriangleMesh::updateBuffer()
{
//    m_mesh->setVerts(getPosArray(), getNormArray());
    m_mesh->setTriangles(_triangles, getPosArray());
}

void TriangleMesh::onDraw(Graphics *g)
{
    g->setColor(1, 1, 1, .7, 0);

    glUniformMatrix4fv(glGetUniformLocation(m_shader, "model"),
                       1, GL_FALSE, glm::value_ptr(glm::mat4()));
    m_mesh->onDraw(GL_TRIANGLES);
}

