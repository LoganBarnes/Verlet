#ifndef LIGHTPARSER_H
#define LIGHTPARSER_H

#include "graphics.h"

class LightParser
{
public:
    LightParser();
    QList<Light*> getLights(const QString &path, glm::vec3 offset);
};

#endif // LIGHTPARSER_H
