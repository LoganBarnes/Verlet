#ifndef LINK_H
#define LINK_H

#include <iostream>

struct Link
{
public:
    //indices of points and length between them
    Link(int a, int b, float length);
    Link(int a, int b, int c, float length);
    ~Link();

    int pointA, pointB;
    float restLength;
    //precalculated for squareroot approximation
    float restLengthSq;

    int numTri; //for calculating averages
    int pointC; //in between A and B, for shear constraint

    friend std::ostream& operator<<(std::ostream& os, const Link& t)
    {
        os << "Link:" << t.pointA << "," << t.pointB;
        return os;
    }

    bool operator == (const Link &l)
    const {return pointA==l.pointA&&pointB==l.pointB;}
};

#endif // LINK_H
