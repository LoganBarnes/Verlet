#include "objecthandler.h"

ObjectHandler::ObjectHandler()
{
}


ObjectHandler::~ObjectHandler()
{
    QList<OBJ*> objects = m_objects.values();
    foreach(OBJ *obj, objects)
        delete obj;
}


OBJ *ObjectHandler::getObject(const QString &path, GLuint shader, QList<Triangle *> *tris)
{
    OBJ* obj;
    obj = m_objects.value(path, NULL);

    if (obj)
        return obj;

    obj = new OBJ(shader);
    if (obj->read(path, tris))
        m_objects.insert(path, obj);
    else
    {
        delete obj;
        obj = NULL;
    }
    return obj;
}

