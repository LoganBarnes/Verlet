#include "link.h"

Link::Link(int a, int b, float length)
{
    pointA = a;
    pointB = b;
    restLength = length;
    restLengthSq = restLength*restLength;
}

Link::~Link()
{

}
