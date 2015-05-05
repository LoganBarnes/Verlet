#include "trianglemesh.h"
#include "verletmanager.h"
#include "mesh.h"
#include "graphics.h"

#define GLM_FORCE_RADIANS
#include <gtc/type_ptr.hpp>

#include "debugprinting.h"



//******************************CONSTRUCTING**********************************//
TriangleMesh::TriangleMesh(const glm::vec2& dimension, float w,
                           const glm::vec3& start, VerletManager* vm, GLuint shader,
                           Axis axis, bool flat, PinMode p)
    : Verlet(vm),
    m_shader(shader)
{
    int r = dimension.y;
    int c = dimension.x;
    float h = (sqrt(3)/2.0) * w;

    glm::vec3 width, height, half_width;

    if(axis==0){ //x-axis
        width = glm::vec3(w,0,0);
        height = flat ? glm::vec3(0,0,-h) : glm::vec3(0,-h,0);
    }
    if(axis==1){ //y-axis
        width = glm::vec3(0,w,0);
        height = flat ? glm::vec3(-h,0,0) : glm::vec3(0,0,-h);
    }
    if(axis==2){ //z-axis
        width = glm::vec3(0,0,w);
        height = flat ? glm::vec3(-h,0,0) : glm::vec3(0,-h,0);
    }
    half_width = width*.5f;

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

    //pin
    pin(p,r,c);
}

void TriangleMesh::pin(PinMode p, int r, int c){
    switch (p){
    case TOP_CORNERS:
        createPin(0);
        createPin(c-1);
        break;
    case ALL_CORNERS:
        createPin(0);
        createPin(c-1);
        createPin((r-1)*c);
        createPin(r*c-1);
        break;
    case TOP_EDGE:
        for(int i = 0; i<c; i++)
            createPin(i);
        break;
    case HORIZONTAL_EDGE:
        for(int i = 0; i<c; i++){
            createPin(i);
            createPin(i+((r-1)*c));
        }
        break;
    case ALL_EDGE:
        for(int i = 0; i<c; i++){
            createPin(i);
            createPin(i+((r-1)*c));
        }
        for(int i = 0; i<r; i++){
            createPin(i*c);
            createPin(i*c+c-1);
        }
        break;
    case NONE:
        break;
    }
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

bool TriangleMesh::checkShearValid(Link* s){
    QList<int> points;
    points.push_back(s->pointA);
    points.push_back(s->pointB);
    points.push_back(s->pointC);

    QList<Link*> segments = shear_to_link[s];
    bool a = points.contains(segments[0]->pointA);
    bool b = points.contains(segments[1]->pointA);
    bool c = points.contains(segments[0]->pointB);
    bool d = points.contains(segments[1]->pointB);

    if(!((a && b) && (c && d))){
        //std::cout<<"*invalid shear: non-matching"<<std::endl;
        //std::cout<<"indices:"<<s->pointA<<","<<s->pointB<<","<<s->pointC<<std::endl;
        //std::cout<<"edges:"<<segments[0]->pointA<<","<<segments[0]->pointB<<std::endl;
        //std::cout<<"edges:"<<segments[1]->pointA<<","<<segments[1]->pointB<<std::endl;
        return false;
    }
    return true;
}

void TriangleMesh::removeShear(Link* s){
    //remove link from 'shear_map' of a and b
    shear_map[s->pointA].removeOne(s);
    shear_map[s->pointB].removeOne(s);
    //remove link from 'crossover_map' of c
    crossover_map[s->pointC].removeOne(s);
    //remove shear from 'link_to_shear' of its segments
    QList<Link*> segments = shear_to_link[s];
    link_to_shear[segments[0]].removeOne(s);
    link_to_shear[segments[1]].removeOne(s);
    //remove shear from 'shear_to_link'
    shear_to_link.remove(s);
    //remove link from 'links'
    links.erase(std::remove(links.begin(), links.end(), s), links.end());

    delete s;
}
//********************************TEARING*************************************//
/*
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
                shear_to_link[shears_at_c[0]].removeOne(l);
                shear_to_link[shears_at_c[1]].removeOne(l);
            }
            else
                std::cout<<"Error in tearLink(): shears_at_c isn't 1/2"<<std::endl;
        }
        //3. Erase old link
        removeLink(l);

        //4. Check each end to see if it needs to be torn
        if(checkTorn(a))
            insertPoint(a,t1,l1,t2,l2);
        if(checkTorn(b)){
            insertPoint(b,t2,l2,t1,l1);
        }
    }
}

//Returns whether point 'a' needs to be torn
bool TriangleMesh::checkTorn(int a){
    QList<Link*> list = link_map[a];
    int splitLinks = 0;
    foreach(Link* l, list){
        QList<Tri*> tri = link_to_tri[l];
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
void TriangleMesh::findConnecting(int index, Tri*& t1, Link*& l1, QList<Tri*>& triangles,
                                  QList<Link*>& links, QList<int> &points){
    links.push_back(l1);
    triangles.push_back(t1);

    Tri* currentTriangle = t1;
    Link* oppositeEdge = NULL; //other link in triangle connected to new point

    while(currentTriangle!=NULL){
        points.push_back(currentTriangle->a);
        points.push_back(currentTriangle->b);
        points.push_back(currentTriangle->c);

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
void TriangleMesh::insertPoint(int index, Tri* t1, Link* l1, Tri* t2, Link* l2){
    //1.Duplicate point
    int index2 = duplicatePoint(index);

    //2.Find triangles + edges on each side (from given edge until next split edge)
    //Side1: to be reassigned with index2
    QList<Tri*> connectedTri1; QList<Link*> connectedLink1; QList<int> points1;
    findConnecting(index,t1,l1,connectedTri1,connectedLink1,points1);
    //Side2: to keep index
    QList<Tri*> connectedTri2; QList<Link*> connectedLink2; QList<int> points2;
    findConnecting(index,t2,l2,connectedTri2,connectedLink2,points2);

    //Keep track of:
    QList<Link*> shear_a;
    QList<Link*> shear_b;
    QList<Link*> border_a;
    QList<Link*> border_b;
    Link* link_a1 = l1;
    Link* link_a2 = connectedLink1.last();
    Link* link_b1 = l2;
    Link* link_b2 = connectedLink2.last();

    //3.Assign index2 to triangles (connectedTri1)
    foreach(Tri* t, connectedTri1)
        t->replaceIndex(index,index2);
    points1.removeAll(index);  //update points1 to contain index2 instead of index
    points1.push_back(index2);

    //4.Assign index2 to structural edges (connectedLink1): update link_map
    foreach(Link* l, connectedLink1){
        (l->pointA==index) ? l->pointA=index2 : l->pointB=index2;
        link_map[index2]+=l;
        link_map[index].removeOne(l);
        QList<Link*> shears = link_to_shear[l];
        foreach(Link* s, shears)
            if(s->pointC==index) s->pointC=index2;
    }

    //5.Group shears into:
    //shear_a / shear_b: on side1 / side2, doesn't contain border links
    //border_a / border_b: on side1/ side2, contain border links
    QList<Link*> shear = shear_map[index];

    foreach(Link* s, shear){
        QList<Link*> segments = shear_to_link[s];
        for(int i = 1; i<connectedLink1.size()-1; i++){
            Link* l = connectedLink1[i];
            if(segments.contains(l))
                shear_a.push_back(s);
        }
        for(int i = 1; i<connectedLink2.size()-1; i++){
            Link* l = connectedLink2[i];
            if(segments.contains(l))
                shear_b.push_back(s);
        }
        if(segments.contains(link_a1)||segments.contains(link_a2))
            border_a.push_back(s);
        if(segments.contains(link_b1)||segments.contains(link_b2))
            border_b.push_back(s);
    }


    //6.Assign index2 to shear (shear_a, border_a): update shear map
    foreach(Link* s, shear_a){
        (s->pointA==index) ? s->pointA=index2 : s->pointB=index2;
        shear_map[index2]+=s;
        shear_map[index].removeOne(s);
    }
     foreach(Link* s, border_a){
         (s->pointA==index) ? s->pointA=index2 : s->pointB=index2;
         shear_map[index2]+=s;
         shear_map[index].removeOne(s);
     }

     //7.Remove shears in which segments aren't connected
     foreach(Link* s, border_a){
         if(!checkShearValid(s)){
             border_a.removeOne(s);
             removeShear(s);
         }
     }
     foreach(Link* s, border_b){
         if(!checkShearValid(s)){ //the check shouldn't be reliant on shears having two links...
             border_b.removeOne(s);
             removeShear(s);
         }
     }
     //8.Destroy any crossing shear constraints in which a and b aren't on the same side
     QList<Link*> cross = crossover_map[index];
     foreach(Link* s, cross){
         //Handle duplicates
         QList<Link*> segments = shear_to_link[s];
         if(segments.size()==1&&segments[0]==link_a1){
             shear_to_link[s]+=link_a2;
             link_to_shear[link_a2]+=s;
             //crossover_map[index2]+=s;
             //crossover_map[index].removeOne(s);
         }
         else if(segments.size()==1&&segments[0]==link_a2){
             shear_to_link[s]+=link_a1;
             link_to_shear[link_a1]+=s;
             //crossover_map[index2]+=s;
             //crossover_map[index].removeOne(s);
         }
         else if(segments.size()==1&&segments[0]==link_b1){
             shear_to_link[s]+=link_b2;
             link_to_shear[link_b2]+=s;
         }
         else if(segments.size()==1&&segments[0]==link_b2){
             shear_to_link[s]+=link_b1;
             link_to_shear[link_b1]+=s;
         }
         else if(segments.size()==1){
             std::cout<<"no match:"<<std::endl;
             std::cout<<segments[0]->pointA<<",b:"<<segments[0]->pointB<<",c:"<<segments[0]->pointC;
                }
     }

     //Update index
     foreach(Link* c, cross){
         QList<Link*> segments = shear_to_link[c];
         //***************HERE'S WHERE THE INDEX IS OUT OF RANGE, BC THERE WAS NO MATCH
         if(segments[0]->pointA==index2||segments[0]->pointB==index2||segments[1]->pointA==index2||segments[1]->pointB==index2){
             c->pointC=index2;
             crossover_map[index2]+=c;
             crossover_map[index].removeAll(c);
             //cross2.removeOne(c);
             //cross1.push_back(c);
             //removeFromHash(index,c,crossover_map);
         }
     }


     QList<Link*> cross2 = crossover_map[index];  //on the side of index
     QList<Link*> cross1 = crossover_map[index2]; //on the side of index2

     foreach(Link* c, cross2){
         int a = c->pointA;
         int b = c->pointB;
         bool onSide = points2.contains(a)&&points2.contains(b)&&points2.contains(c->pointC);
         if(!onSide||!checkShearValid(c)){
             removeShear(c);
             cross2.removeOne(c);
             //cross.removeOne(c);
         }
     }
     foreach(Link* c, cross1){
         int a = c->pointA;
         int b = c->pointB;
         bool onSide = points1.contains(a)&&points1.contains(b)&&points1.contains(c->pointC);
         if(!onSide||!checkShearValid(c)){
             removeShear(c);
             cross1.removeOne(c);
             //cross.removeOne(c);
         }
     }
     crossover_map[index]=cross2;
     crossover_map[index2]=cross1;

    //recalculate scalar + numTri for index + i (new index)
    numTri[index2] = connectedTri1.size();
    _scalar[index2]=1/(float)numTri[index2];

    numTri[index] = numTri[index]-connectedTri1.size();
    _scalar[index]=1/(float)numTri[index];
}
*/
//********************************UPDATING*************************************//
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
    g->drawMesh(m_mesh, glm::mat4());
}

