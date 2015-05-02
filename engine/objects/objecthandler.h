#ifndef OBJECTHANDLER_H
#define OBJECTHANDLER_H

#include <QHash>
#include <QString>
#include "obj.h"

class ObjectHandler
{
public:
    ObjectHandler();
    virtual ~ObjectHandler();

    OBJ *getObject(const QString &path, GLuint shader, QList<Triangle *> *tris, const glm::vec3& offset);

private:
//    QHash<QString, OBJ *> m_objects;
    QList<OBJ*> m_objects;
};

#endif // OBJECTHANDLER_H
