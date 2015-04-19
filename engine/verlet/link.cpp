#include "link.h"

Link::Link(int a, int b, float length)
{
    pointA = a;
    pointB = b;
    restLength = length;
    restLengthSq = restLength*restLength;
    numTri = 0;
}

Link::Link(int a, int b, int c, float length)
{
    pointA = a;
    pointB = b;
    pointC = c;
    restLength = length;
    restLengthSq = restLength*restLength;
    numTri = 0;
}

Link::~Link()
{

}
