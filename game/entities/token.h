#ifndef TOKEN_H
#define TOKEN_H

#include "marker.h"

class Token : public Marker
{
public:
    Token(OBJ* obj, glm::vec2 center, glm::vec2 size, QString path);
};

#endif // TOKEN_H
