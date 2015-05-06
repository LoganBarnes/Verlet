#ifndef SPIRALSENSOR_H
#define SPIRALSENSOR_H

#include "marker.h"

class SpiralSensor: public Marker
{
public:
    SpiralSensor(OBJ* obj, glm::vec2 center, glm::vec2 size, QString path);
    ~SpiralSensor();
};

#endif // SPIRALSENSOR_H
