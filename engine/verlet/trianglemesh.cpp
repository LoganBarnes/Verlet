#include "trianglemesh.h"
#include "verletmanager.h"
#include "mesh.h"
#include "graphics.h"

#define GLM_FORCE_RADIANS
#include <gtc/type_ptr.hpp>

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


    for(int i =0; i<r-1; i++){
        for(int j = 0; j<c; j++){
            if(i%2==0){  //even
                if(j<c-1){
                    //upper triangles
                    int n = c*i+j;
                    //std::cout<<n<<std::endl;
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

    //set scalar
    for(int i=0; i<numPoints; i++)
        _scalar[i]=1/(float)numTri[i];

    //shear and bend constraints
        for(int i = 0; i<r; i++){
            for(int j = 0; j<c; j++){
                if(j+2<c)  //horizontal
                    createLink(j+(i*c),(i*c)+j+2);
                //if(i+2<r && (j==0 || j==c-1)) //vertical, at edges
                //    createLink(j+(i*c),(i+2)*c+j);
                //if(i+2<r && j!=0 && j+1<c-1)
                //    createLink(j+c*i,j+1+c*(i+2));
                if(i+2<r && j>0 && j-1<c){
                    if((!(j==c-1&&i%2==0)) && (!(j==1&&i%2!=0)))
                            createLink(j+c*i,j-1+c*(i+2));
                }
            }
        }


    m_mesh = new Mesh();
//    m_mesh->initStrip(m_shader, c, r, getPosArray(), getNormArray());
    m_mesh->initTriangles(m_shader, _triangles, getPosArray());
}

TriangleMesh::~TriangleMesh()
{
    delete m_mesh;
}


void TriangleMesh::triangulate(int a, int b, int c){
    createLink(a,b);
    createLink(b,c);
    createLink(c,a);

    Tri t = Tri(a,b,c);
    numTri[a]+=1;
    numTri[b]+=1;
    numTri[c]+=1;
    calculate(t);
    _triangles.push_back(t);
}

void TriangleMesh::calculate(Tri& t){
    t.vertices[0]=_pos[t.a];
    t.vertices[1]=_pos[t.b];
    t.vertices[2]=_pos[t.c];

    t.normal =  glm::cross((t.vertices[1] - t.vertices[0]), (t.vertices[2] - t.vertices[0]));
    t.normal = glm::normalize(t.normal);

    _normal[t.a]+=t.normal*_scalar[t.a];
    _normal[t.b]+=t.normal*_scalar[t.b];
    _normal[t.c]+=t.normal*_scalar[t.c];
}

void TriangleMesh::applyWind(Tri& t){
    //wind has full effect to perpendicular cloth, none on parallel cloth
    glm::vec3 windDirection = _manager->wind;
    float windScalar =  glm::dot(windDirection, t.normal);

    if(windScalar<0)
        windScalar*=-1;
    t.windForce = windScalar;

    glm::vec3 windForce = windDirection*windScalar*_manager->windPow;

    _acc[t.a] += windForce;
    _acc[t.b] += windForce;
    _acc[t.c] += windForce;
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
    g->setColor(.5f,.5f,1.f,1.f,0.f);
    glUniformMatrix4fv(glGetUniformLocation(m_shader, "model"),
                       1, GL_FALSE, glm::value_ptr(glm::mat4()));
    m_mesh->onDraw(GL_TRIANGLES);
}

