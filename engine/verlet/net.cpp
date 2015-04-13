#include "net.h"
#include "mesh.h"

Net::Net(glm::vec2 dimension, const glm::vec3 &start,
         const glm::vec3 &interval1, const glm::vec3 &interval2): Verlet()
{
    int width = dimension.x;
    int height = dimension.y;
    int count = 0;

    for(int i = 0; i<width; i++){
        for(int j = 0; j<height; j++){
            createPoint(start+(float)j*interval1+(float)i*interval2);
            if(j!=0)  //attach to the left
                createLink(count-1,count);
            if(i!=0)  //attach to top
                createLink(count-width, count);
            count++;
        }
    }

    m_mesh = new Mesh();
}

Net::~Net()
{
    delete m_mesh;
}


//void Net::onDraw(Graphics *g)
//{
//    for(int i=0; i<links.size(); i++){
//        Link l = links.at(i);
//        g->drawLineSeg(_pos[l.pointA],_pos[l.pointB], .1f);
//    }
//}

