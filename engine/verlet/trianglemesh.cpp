#include "trianglemesh.h"
#include "verletmanager.h"
#include "mesh.h"
#include "graphics.h"
#include "bend.h"

#define GLM_FORCE_RADIANS
#include <gtc/type_ptr.hpp>

#include "debugprinting.h"

//******************************CONSTRUCTING**********************************//
TriangleMesh::TriangleMesh(const glm::vec2& dimension, float w,
                           const glm::vec3& start, VerletManager* vm, GLuint shader, int angle,
                           Axis axis, PinMode p)
    : Verlet(vm),
    m_shader(shader)
{
    m_row = dimension.y;
    m_col = dimension.x;
    int r = m_row;
    int c = m_col;

    glm::vec3 width, height, half_width;

    QPair<glm::vec2,glm::vec2> dim = rotateTriangle(w,angle);
    glm::vec2 test_w = dim.first;
    glm::vec2 test_h = dim.second;
    if(axis==X){
       width = glm::vec3(0,test_w.x,test_w.y);
       height = glm::vec3(0,test_h.x,test_h.y);
    }
    if(axis==Y){
        width = glm::vec3(test_w.x,0,test_w.y);
        height = glm::vec3(test_h.x,0,test_h.y);
    }
    if(axis==Z){
        width = glm::vec3(test_w.x,test_w.y,0);
        height = glm::vec3(test_h.x,test_h.y,0);
    }

//    float h = (sqrt(3)/2.0) * w;
//    if(axis==0){ //x-axis
//        width = glm::vec3(w,0,0);
//        height = flat ? glm::vec3(0,0,-h) : glm::vec3(0,-h,0);
//    }
//    if(axis==1){ //y-axis
//        width = glm::vec3(0,w,0);
//        height = flat ? glm::vec3(-h,0,0) : glm::vec3(0,0,-h);
//    }
//    if(axis==2){ //z-axis
//        width = glm::vec3(0,0,w);
//        height = flat ? glm::vec3(-h,0,0) : glm::vec3(0,-h,0);
//    }
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

QPair<glm::vec2,glm::vec2> TriangleMesh::rotateTriangle(float _w,int angle){
    float h = (sqrt(3)/2.0) * _w;
    float w = -_w;
    if(h<0){
//        h*=-1;
        w*=-1;
    }

    glm::vec2 p1 = glm::vec2(0,(2.0/3.0)*h);
    glm::vec2 p2 = glm::vec2(-.5*w,-(1.0/3.0)*h);
    glm::vec2 p3 = glm::vec2(.5*w,-(1.0/3.0)*h);

    glm::vec2 v1 = rotatePoint(p1,angle);
    glm::vec2 v2 = rotatePoint(p2,angle);
    glm::vec2 v3 = rotatePoint(p3,angle);

    glm::vec2 width = v3-v2;
    glm::vec2 height = v1-((v2+v3)*.5f);

    return QPair<glm::vec2,glm::vec2>(width,height);
}

glm::vec2 TriangleMesh::rotatePoint(const glm::vec2& p, int angle){
    float radians = angle * (M_PI / 180.0);
    float x = p.x*cos(radians)-p.y*sin(radians);
    float y = p.y*cos(radians)+p.x*sin(radians);
    return glm::vec2(x,y);
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
    case ONE_CORNER:
        createPin(0);
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
Bend* TriangleMesh::createShear(int a, int b, int c, Link* seg1, Link* seg2){
    float length = glm::length(_pos[b]-_pos[a]);
    Bend* l = new Bend(a, b, length);
    links.push_back(l);

    if(seg1==NULL&&seg2==NULL){
        seg1 = findLink(a,c);
        seg2 = findLink(b,c);
    }

    //Make sure that segments are arranged so that the shared point is between them
    if(seg1->pointB==seg2->pointA){
        l->seg1 = seg1;
        l->seg2 = seg2;
    }
    else if(seg2->pointB==seg1->pointA){
        l->seg2 = seg1;
        l->seg1 = seg2;
    }
    else
        std::cout<<"createShear(): shear constraint's segments are not in linear order"<<std::endl;

    _linkMap[a].shears+=l;
    _linkMap[b].shears+=l;
    _linkMap[c].crosses+=l;
    link_to_shear[seg1]+=l;
    link_to_shear[seg2]+=l;

    return l;
}

void TriangleMesh::removeShear(Bend* s){
    if(s->pointA<0||s->pointA>numPoints||s->pointB<0||s->pointB>numPoints){
        std::cout<<"removeShear(): nonsense value for points"<<std::endl;
        std::cout<<"s:"<<s->pointA<<","<<s->pointB<<std::endl;
        //if(s->seg2==NULL||s->seg1==NULL)
        //    std::cout<<"nulls"<<std::endl;
        std::cout<<"seg2:"<<s->seg2->pointA<<","<<s->seg2->pointB<<std::endl;
        std::cout<<"seg1:"<<s->seg1->pointA<<","<<s->seg1->pointB<<std::endl;
    }
    _linkMap[s->pointA].shears.removeAll(s);
    _linkMap[s->pointB].shears.removeAll(s);
    _linkMap[s->seg1->pointB].crosses.removeAll(s);
    _linkMap[s->seg2->pointA].crosses.removeAll(s); //in case they differ- very necessary
    _linkMap[s->seg1->pointA].crosses.removeAll(s);  //in case they're backwards
    _linkMap[s->seg2->pointB].crosses.removeAll(s);
    link_to_shear[s->seg1].removeAll(s);
    link_to_shear[s->seg2].removeAll(s);

    links.erase(std::remove(links.begin(), links.end(), s), links.end());

    delete s;
}

//********************************TEARING*************************************//
//Returns whether point 'a' needs to be torn
bool TriangleMesh::checkTorn(int a){
    QList<Link*> list = _linkMap[a].links;
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

//Replaces l (if it has a triangle on each side) w/ l1 and l2, and reassigns these links
//accordingly to linkMap and shears
void TriangleMesh::tearLink(Link* l){
    QList<Tri*> tri = link_to_tri[l];
    if(tri.size()!=2)
        return;

    //1. Duplicate l into l1 and l2
    //Note: to keep ordering intact for shears, default ordering is set to false
    int a = l->pointA;
    int b = l->pointB;
    Link* l1 = createLink(a,b,false);
    Link* l2 = createLink(a,b,false);

    //2. Reassign 1 to each triangle (t1,t2)
    Tri* t1 = tri[0];
    Tri* t2 = tri[1];
    t1->replaceLink(l,l1);
    t2->replaceLink(l,l2);
    QList<Tri*> test1 = QList<Tri*>();
    test1.push_back(t1);

    QList<Tri*> test2 = QList<Tri*>();
    test2.push_back(t2);
    link_to_tri[l1]=test1;
    link_to_tri[l2]=test2;

    //3. Information about tearing
    bool tearBefore = checkTorn(a);  //whether a will be duplicated
    bool tearAfter = checkTorn(b);   //whether b will be duplicated
    Connected c1_before = findConnecting(a,t1,l1);  //connected to a and l1
    Connected c2_before = findConnecting(a,t2,l2);  //connected to a and l2
    Connected c1_after  = findConnecting(b,t1,l1);  //connected to b and l1
    Connected c2_after  = findConnecting(b,t2,l2);  //connected to b and l2

    //4. Sort shears into 'before' (shares pointB w/ l) or 'after' (shares ptA w/ l)
    QList<Bend*> shears = link_to_shear[l];
    QList<Bend*> before;
    QList<Bend*> after;
    foreach(Bend* s, shears){
        if(s->pointB==l->pointB)
            before+=s;
        else if(s->pointA==l->pointA)
            after+=s;
        else if(s->pointA==l->pointB)
            before+=s;
            //std::cout<<"before backwards:"<<s->pointA<<","<<s->pointB;
        else if(s->pointB==l->pointA)
            after+=s;
            //std::cout<<"after backwards:"<<s->pointA<<","<<s->pointB;
        else{
            std::cout<<"tearLink(): link not sorted"<<std::endl;
            std::cout<<s->pointA<<","<<s->pointB<<std::endl;
        }
    }
    //std::cout<<"accumulative:"<<before.size()+after.size()<<std::endl;
    //std::cout<<"total:"<<shears.size()<<std::endl;

    //5. Assigns l1 and l2 to the segment that was formerly l.
    //If it's a 'single' shear, it's duplicated
    handleShears(tearBefore,before,l,c1_before.connectedLink,c2_before.connectedLink);
    handleShears(tearAfter,after,l,c1_after.connectedLink,c2_after.connectedLink);

    //6. Erase old link
    removeLink(l);

    if(tearBefore)
        insertPoint(a,c1_before,c2_before);
    if(tearAfter)
        insertPoint(b,c1_after,c2_after);
}

//If 'index' needs to be torn, insert another point (index2) and reassign triangles/ links as necessary
//@param c1: contains triangles/links to be reassigned to index2
int TriangleMesh::insertPoint(int index, Connected c1, Connected c2){
    //1.Duplicate point
    int index2 = duplicatePoint(index);

    QList<int> points1a;
    foreach(Tri* t, c1.connectedTri){
        points1a.push_back(t->a);
        points1a.push_back(t->b);
        points1a.push_back(t->c);
    }
    QList<int> points2a;
    foreach(Tri* t, c2.connectedTri){
        points2a.push_back(t->a);
        points2a.push_back(t->b);
        points2a.push_back(t->c);
    }

    LinkMap& map1 = _linkMap[index];
    LinkMap& map2 = _linkMap[index2];

    //2.Assign index2 to triangles
    foreach(Tri* t, c1.connectedTri)
        t->replaceIndex(index,index2);
    points1a.removeAll(index);
    points1a.push_back(index2);

    //3.Assign index2 to structural edges, and update LinkMap.links
    foreach(Link* l, c1.connectedLink){
        (l->pointA==index) ? l->pointA=index2 : l->pointB=index2;
        map2.links+=l;
        map1.links.removeAll(l);
    }

    //4.Shears: remove invalid shears, assign index2, and upate LinkMap.shears
    QList<Bend*> shear = map1.shears;
    foreach(Bend* s, shear){
        if(s->seg1==NULL||s->seg2==NULL)
            std::cout<<"null in shear"<<std::endl;
        else if(s->seg1->pointB!=s->seg2->pointA){
            removeShear(s);
        }
        else{
            foreach(Link* l, c1.connectedLink){
                if(s->seg1==l||s->seg2==l){
                    (s->pointA==index) ? s->pointA=index2 : s->pointB=index2;
                    map2.shears+=s;
                    map1.shears.removeOne(s);
                }
            }
        }
    }

    //5.Cross: remove invalid, assign index2, and upate LinkMap.cross
    QList<Bend*> cross = map1.crosses;
    foreach(Bend* c, cross){
        int a = c->pointA;
        int b = c->pointB;
        bool onSide = ((points2a.contains(a)&&points2a.contains(b))||(points1a.contains(a)&&points1a.contains(b)));
        if(c->seg1==NULL||c->seg2==NULL)
            std::cout<<"null in cross"<<std::endl;
        else if(!onSide)
            removeShear(c);
        else if(c->seg1->pointB!=c->seg2->pointA)
            removeShear(c);
        else if(c->seg1->pointB==index2){  //update index
            map2.crosses+=c;
            map1.crosses.removeOne(c);
        }
    }

    //recalculate scalar + numTri for index + i (new index)
    numTri[index2] = c1.connectedTri.size();
    _scalar[index2]=1/(float)numTri[index2];

    numTri[index] = numTri[index]-c1.connectedTri.size();
    _scalar[index]=1/(float)numTri[index];

    return index2;
}


//If one of shear b's segments is within 'connected', its l (orig) segment is assigned to 'l1/l2'
bool TriangleMesh::assignLink(QList<Link*> connected, Bend* b, Link* orig){
    Link* l = connected[0];
    Link* bseg = (b->seg1==orig) ? b->seg2 : b->seg1;
    if(connected.contains(bseg)){
        (b->seg1==orig) ? b->seg1=l : b->seg2=l;
        return true;
    }
    return false;
}

//Replaces l in each shear w/ l1/l2, depending on which 'connected links' contains shear's other segment
void TriangleMesh::handleShears(bool tear, QList<Bend*> duplicates, Link* l, QList<Link*> l1_links, QList<Link*> l2_links){
    //The pair of links replacing the original link 'l'
    Link* l1 = l1_links[0];
    Link* l2 = l2_links[0];

    //For 'duplicates' (pairs of shears), l is reassigned
    if(duplicates.size()==2){
        Bend* b1 = duplicates[0];
        Bend* b2 = duplicates[1];
        //l1 and l2 and be assigned arbitrarily as segments to replace l if there's 'tear' (new point formed)
        if(!tear){
            (b1->seg1==l) ? b1->seg1=l1 : b1->seg2=l1;
            (b2->seg1==l) ? b2->seg1=l2 : b2->seg2=l2;
        }
        //If there will be a new point formed, insure each shear in 'duplicates' has valid segments
        else{
            if(!assignLink(l1_links, b1, l)&&!assignLink(l2_links, b1, l))
                std::cout<<"handleDuplicates(): no replacement found for l w/in duplicate shear"<<std::endl;
            if(!assignLink(l1_links, b2, l)&&!assignLink(l2_links, b2, l))
                std::cout<<"handleDuplicates(): no replacement found for l w/in duplicate shear"<<std::endl;
        }
    }
    //For 'singles', the shear is duplicated
    else if(duplicates.size()==1){
        Bend* b = duplicates[0];
        Link* other = (b->seg1==l) ? b->seg2 : b->seg1;
        createShear(b->pointA,b->pointB,b->seg1->pointB,other,l1);
        createShear(b->pointA,b->pointB,b->seg1->pointB,other,l2);
        _linkMap[b->seg1->pointB].crosses.removeOne(b);
        removeShear(b);
    }
}

//Finds all links + triangles adjoining l1 and attached to index
Connected TriangleMesh::findConnecting(int index, Tri*& t1, Link*& l1){
    Connected c;
    c.connectedLink.push_back(l1);
    c.connectedTri.push_back(t1);

    Tri* currentTriangle = t1;
    Link* oppositeEdge = NULL; //other link in triangle connected to index

    while(currentTriangle!=NULL){
        c.points.push_back(currentTriangle->a);
        c.points.push_back(currentTriangle->b);
        c.points.push_back(currentTriangle->c);

        //find opposite edge
        for(int i = 0; i<3; i++){
            Link* l = currentTriangle->edges[i];
            if((l->pointA==index || l->pointB==index)&&!c.connectedLink.contains(l)){
                oppositeEdge=l;
                c.connectedLink.push_back(oppositeEdge);
            }
        }
        //find the triangle attached to it, set it to currentTriangle
        QList<Tri*> list = link_to_tri[oppositeEdge];
        if(list.size()==2){  //triangle that isn't this one
            currentTriangle = (list[0]==currentTriangle) ? list[1] : list[0];
            c.connectedTri.push_back(currentTriangle);
        }
        else
            currentTriangle = NULL;
    }
    return c;
}

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

