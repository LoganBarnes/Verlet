#include "trianglemesh.h"
#include "verletmanager.h"
#include "mesh.h"
#include "graphics.h"

#define GLM_FORCE_RADIANS
#include <gtc/type_ptr.hpp>

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
                createShear(j+(i*c),(i*c)+j+2,j+1+(i*c));
            if(i+2<r && (j==0 || j==c-1)) //vertical, at edges
                createShear(j+(i*c),(i+2)*c+j,(i+1)*c+j);
            if(i+2<r && j+1<c){ //diagonal right-down
                if(j+1!=c-1&& (j!=0||i%2==0)){
                    if(i%2==0)
                        createShear(j+c*i,j+1+c*(i+2),(i+1)*c+j+1);
                    else
                        createShear(j+c*i,j+1+c*(i+2),(i+1)*c+j);
                }
                else if(j+1==c-1 && i%2==1)
                    createShear(j+c*i,j+1+c*(i+2),(i+1)*c+j);
            }
            if(i+2<r && j>0 && j-1<c){ //diagonal left-down
                if((!(j==c-1&&i%2==0)) && (!(j==1&&i%2!=0))){
                    if(i%2==1)
                        createShear(j+c*i,j-1+c*(i+2),(i+1)*c+j-1);
                    else
                        createShear(j+c*i,j-1+c*(i+2),(i+1)*c+j);
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
Link* TriangleMesh::createShear(int a, int b, int c, Link* seg1, Link* seg2){
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
void TriangleMesh::tearLink(Link* l){
    QList<Tri*> tri = link_to_tri[l];
    if(tri.size()==2){
        Tri* t1 = tri[0];
        Tri* t2 = tri[1];
        int a = l->pointA;
        int b = l->pointB;

        //1. Duplicate l into l1 and l2, and reassign 1 to each triangle (t1, t2)
        Link* l1 = createLink(a,b);
        Link* l2 = createLink(a,b);
        t1->replaceLink(l,l1);
        t2->replaceLink(l,l2);
        link_to_tri[l1]+=t1;
        link_to_tri[l2]+=t2;

        //2. Duplicate 'single' shears l is part of, record 'duplicates'
        QList<Link*> shears = link_to_shear[l];
        std::vector<Duplicate> duplicate;
        QHash<int,QList<Link*> > cMap;

        foreach(Link* s, shears)
            cMap[s->pointC]+=s;
        foreach(const int &key, cMap.keys()){
            QList<Link*> shears_at_c = cMap[key];
            //Single: other end of shear is solid (no shared midpoint)
            //Duplicate the shear, and assign l1 / l2 (to the 'l' segment)
            if(shears_at_c.size()==1){
                Link* s = shears_at_c[0];
                QList<Link*> segments = shear_to_link[s];
                Link* other_link = (segments[0]==l) ? segments[1] : segments[0];

                createShear(s->pointA,s->pointB,s->pointC,other_link,l1);
                createShear(s->pointA,s->pointB,s->pointC,other_link,l2);
                removeShear(s);
            }
            //Duplciate: other end of shear is split (they share midpoint)
            //Assign l1 and l2 (to each 'l' segment)
            else if(shears_at_c.size()==2){
                Link* s1 = shears_at_c[0];
                Link* s2 = shears_at_c[1];
                QList<Link*> segments1 = shear_to_link[s1];
                Link* other_link1 = (segments1[0]==l) ? segments1[1] : segments1[0];
                QList<Link*> segments2 = shear_to_link[s2];
                Link* other_link2 = (segments2[0]==l) ? segments2[1] : segments2[0];
                Duplicate d = Duplicate(s1,s2,l1,l2,other_link1,other_link2);
                duplicate.push_back(d);
            }
            else
                std::cout<<"Error in tearLink(): shears_at_c isn't 1/2"<<std::endl;
        }
        //3. Erase old link
        removeLink(l);

        //4. Check each end to see if it needs to be torn
        if(checkTorn(a))
            insertPoint(a,t1,l1,t2,l2,duplicate);
        if(checkTorn(b))
            insertPoint(b,t2,l2,t1,l1,duplicate);
    }
}

//Returns whether point 'a' needs to be torn
bool TriangleMesh::checkTorn(int a){
    QList<Link*> l = link_map[a];
    int splitLinks = 0;
    for(int i = 0; i<l.size(); i++){
        QList<Tri*> tri = link_to_tri[l[i]];
        if(tri.size()<2) //edge only has 1 triangle -> tearing= necessary
            splitLinks++;
    }
    return splitLinks>2; //called from tearLink, so there's 2 guaranteed
}

//Copies point, and returns index of copy
int TriangleMesh::duplicatePoint(int index){
    _pos[numPoints] = _pos[index];
    _prevPos[numPoints] = _prevPos[index];
    _acc[numPoints] = _acc[index];
    _normal[numPoints] = _normal[index];
    numPoints++;
    return numPoints-1;
}

//Finds all links + triangles adjoining l1 and attached to index
void TriangleMesh::findConnecting(int index, Tri*& t1, Link*& l1,
                                  QList<Tri*>& triangles, QList<Link*>& links){
    links.push_back(l1);
    triangles.push_back(t1);

    Tri* currentTriangle = t1;
    Link* oppositeEdge = NULL; //other link in triangle connected to new point

    while(currentTriangle!=NULL){
        //find opposite edge
        for(int i = 0; i<3; i++){
            Link* l = currentTriangle->edges[i];
            if((l->pointA==index || l->pointB==index)&&!links.contains(l)){
                oppositeEdge=l;
                links.push_back(oppositeEdge);
            }
        }
        //find the triangle attached to it, set it to currentTriangle
        QList<Tri*> list = link_to_tri[oppositeEdge];
        if(list.size()==2){  //triangle that isn't this one
            currentTriangle = (list[0]==currentTriangle) ? list[1] : list[0];
            triangles.push_back(currentTriangle);
        }
        else
            currentTriangle = NULL;
    }
}

//If 'a' needs to be torn, insert another point there is is
//and reassign triangles/ links as necessary
void TriangleMesh::insertPoint(int index, Tri* t1, Link* l1, Tri* t2, Link* l2, std::vector<Duplicate> duplicate){
    //1.Duplicate point
    int index2 = duplicatePoint(index);

    //2.Find triangles + edges on each side (from given edge until next split edge)
    //Side1: to be reassigned with index2
    QList<Tri*> connectedTri1; QList<Link*> connectedLink1;
    findConnecting(index,t1,l1,connectedTri1,connectedLink1);
    //Side2: to keep index
    QList<Tri*> connectedTri2; QList<Link*> connectedLink2;
    findConnecting(index,t2,l2,connectedTri2,connectedLink2);

    QList<int> points1;
    QList<int> points2;
    foreach(Tri* t, connectedTri1){
        points1.push_back(t->a);
        points1.push_back(t->b);
        points1.push_back(t->c);
    }
    foreach(Tri* t, connectedTri2){
        points2.push_back(t->a);
        points2.push_back(t->b);
        points2.push_back(t->c);
    }

    //3.Assign index2 to triangles (connectedTri1)
    foreach(Tri* t, connectedTri1)
        t->replaceIndex(index,index2);

    //4.Assign index2 to structural edges (connectedLink1): update link_map
    QList<Link*> remainingLinks = link_map[index];
    foreach(Link* l, connectedLink1){
        (l->pointA==index) ? l->pointA=index2 : l->pointB=index2;
        link_map[index2]+=l;
        remainingLinks.removeOne(l);
    }
    link_map[index]=remainingLinks;

    //5.Resolve duplicates: insure that edges each overlaps is consistent w/ its indices
    for(int i = 0; i<duplicate.size(); i++){
        Duplicate d = duplicate[i];
        QPair<Link*,Link*> pair1;
        QPair<Link*,Link*> pair2;
        pair1.first = d.seg3;
        pair2.first = d.seg4;
        if(d.seg3->pointA==d.seg1->pointA||d.seg3->pointA==d.seg1->pointB||
                d.seg3->pointB==d.seg1->pointA||d.seg3->pointB==d.seg1->pointB){
            pair1.second=d.seg1;
            pair2.second=d.seg2;
            link_to_shear[d.seg1]+=d.shear1;
            link_to_shear[d.seg2]+=d.shear2;
        }
        else{
            pair1.second=d.seg2;
            pair2.second=d.seg1;
            link_to_shear[d.seg2]+=d.shear1;
            link_to_shear[d.seg1]+=d.shear2;
        }
        QList<Link*> s1;
        s1.push_back(pair1.first);
        s1.push_back(pair1.second);
        shear_to_link[d.shear1]=s1;
        QList<Link*> s2;
        s2.push_back(pair2.first);
        s2.push_back(pair2.second);
        shear_to_link[d.shear2]=s2;

        if(d.shear1->pointC==index)
            d.shear1->pointC=index2;
        else if(d.shear1->pointA==index)
            d.shear1->pointA=index2;
        else
            d.shear1->pointB=index2;
    }

    //6.Find all shears to be reassigned to index2 (shear1) by matching them w/ connectedLink1
    QList<Link*> shear = shear_map[index];
    QList<Link*> shear1;
    foreach(Link* s, shear){
        QList<Link*> segments = shear_to_link[s];
        foreach(Link* l, connectedLink1){
            if(l==segments[0]||l==segments[1]){  //matching segment
                Link* other = (segments[0]==l) ? segments[1] : segments[0];
                //Check if other segment is connected to it
                if(other->pointA==l->pointA||other->pointB==l->pointA||
                        other->pointA==l->pointB||other->pointB==l->pointB)
                    shear1.push_back(s);
                else
                    removeShear(s);
            }
        }
        foreach(Link* l, connectedLink2){
            if(l==segments[0]||l==segments[1]){
                Link* other = (segments[0]==l) ? segments[1] : segments[0];
                if(!(other->pointA==l->pointA||other->pointB==l->pointA||
                        other->pointA==l->pointB||other->pointB==l->pointB))
                    removeShear(s);
            }
        }
    }
    shear = shear_map[index];

    //7.Assign index2 to shear constraints (shear1): update shear_map
    foreach(Link* s, shear1){
        //if(!duplicates.contains(s)){
            (s->pointA==index) ? s->pointA=index2 : s->pointB=index2;
            shear_map[index2]+=s;
            shear.removeOne(s);
        //}
    }
    shear_map[index]=shear;

    //8.Destroy any crossing shear constraints in which a and b aren't on the same side
    QList<Link*> cross = crossover_map[index];
    foreach(Link* c, cross){
        int a = c->pointA;
        int b = c->pointB;
        bool onSide1 = points1.contains(a)&&points1.contains(b);
        bool onSide2 = points2.contains(a)&&points2.contains(b);
        if(!(onSide1||onSide2))
            removeShear(c);
    }

    //recalculate scalar + numTri for index + i (new index)
    numTri[index2] = connectedTri1.size();
    _scalar[index2]=1/(float)numTri[index2];

    numTri[index] = numTri[index]-connectedTri1.size();
    _scalar[index]=1/(float)numTri[index];
}

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
    g->setColor(.5f,.5f,1.f,1.f,0.f);
    glUniformMatrix4fv(glGetUniformLocation(m_shader, "model"),
                       1, GL_FALSE, glm::value_ptr(glm::mat4()));
    m_mesh->onDraw(GL_TRIANGLES);
}

