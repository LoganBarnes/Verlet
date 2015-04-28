#include "lightparser.h"
#include "obj.h"
#include "triangle.h"

#include <iostream>
using namespace std;

LightParser::LightParser()
{
}

// Read in an obj file and make a list of lights from it to return
QList<Light*> LightParser::getLights(const QString &path){

    QList<Light*> lights;
    QList<Triangle*> tris;

    OBJ* obj = new OBJ(0);
    obj->read(path, &tris);

    Light* light;
    int counter = 0;

    foreach(Triangle* t, tris){

        glm::vec3 norm = t->normal;

        // use triangle information to make a light
        light = new Light();
        light->id = counter++;                  // index into array in shader


        if(norm.y>0)
            light->type = POINT;            // can be POINT or DIRECTIONAL for now
        else
            light->type = DIRECTIONAL;
        light->color = glm::vec3(.750, .750, 1.5f);  // rgb color

        light->posDir = t->vertices[0];

        light->radius = 50.f;
        light->function = glm::vec3(1.0, .1, .01);

        lights.append(light);
    }

    for(int i=0; i<1; i++){

        light = new Light();
        light->id = counter++;

        light->type = POINT;            // can be POINT or DIRECTIONAL for now

        light->color = glm::vec3(100, 100, 100);  // rgb color

        light->posDir = glm::vec3(0,0,100);

        light->radius = 60.f;
        light->function = glm::vec3(1.0, 1.5, 1.5);

        lights.append(light);
    }





    return lights;
}

