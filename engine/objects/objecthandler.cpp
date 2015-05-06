#include "objecthandler.h"

ObjectHandler::ObjectHandler()
{
}


ObjectHandler::~ObjectHandler()
{
//    QList<OBJ*> objects = m_objects.values();
//    foreach(OBJ *obj, objects)
//        delete obj;

    foreach(OBJ * obj, m_objects)
        delete obj;
}


OBJ *ObjectHandler::getObject(const QString &path, GLuint shader, QList<Triangle *> *tris, const glm::vec3& offset, glm::vec4 color)
{
    OBJ* obj;
//    obj = m_objects.value(path, NULL);

//    if (obj)
//    {
//        obj->makeTriList(tris);
//        return obj;
//    }

    obj = new OBJ(shader, color);
    if (obj->read(path, tris, offset))
        m_objects.append(obj);
//        m_objects.insert(path, obj);
    else
    {
        delete obj;
        obj = NULL;
    }
    return obj;
}

