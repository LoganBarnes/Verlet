#include "net.h"

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
}

Net::~Net()
{

}

