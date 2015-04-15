QT += core gui opengl

TARGET = cs1972final
TEMPLATE = app

# project build directories
DESTDIR     = $$system(pwd)
OBJECTS_DIR = $$DESTDIR/bin

unix:!macx {
    LIBS += -lGLU
    QMAKE_CXXFLAGS += -std=c++11
}
macx {
    QMAKE_CFLAGS_X86_64 += -mmacosx-version-min=10.7
    QMAKE_CXXFLAGS_X86_64 = $$QMAKE_CFLAGS_X86_64
    QMAKE_MAC_SDK = macosx10.9
}

INCLUDEPATH +=  glm engine game shaders \
                engine/common \
                engine/ui \
                engine/graphics \
                engine/shapes \
                engine/world \
                engine/collisions \
                engine/objects \
                engine/verlet \
                game/gamescreens \
                game/world \
                game/entities \
                res/images \
                res/images/cubemap \
                res/levels

DEPENDPATH +=   glm engine game shaders \
                engine/common \
                engine/ui \
                engine/graphics \
                engine/shapes \
                engine/world \
                engine/collisions \
                engine/objects \
                engine/verlet \
                game/gamescreens \
                game/world \
                game/entities \
                res/images \
                res/images/cubemap \
                res/levels

DEFINES += TIXML_USE_STL

SOURCES += \
    engine/main.cpp \
    engine/ui/mainwindow.cpp \
    engine/ui/view.cpp \
    engine/ui/application.cpp \
    engine/ui/button.cpp \
    engine/common/point3d.cpp \
    engine/common/perlinnoise.cpp \
    engine/objects/obj.cpp \
    engine/objects/objecthandler.cpp \
    engine/objects/navmeshhandler.cpp \
    engine/graphics/graphics.cpp \
    engine/graphics/cubemap.cpp \
    engine/graphics/camera.cpp \
    engine/graphics/actioncamera.cpp \
    engine/graphics/particleemitter.cpp \
    engine/graphics/meshbuffer.cpp \
    engine/world/world.cpp \
    engine/world/entity.cpp \
    engine/world/movableentity.cpp \
    engine/world/staticentity.cpp \
    engine/world/player.cpp \
    engine/shapes/shape.cpp \
    engine/shapes/cube.cpp \
    engine/shapes/cylinder.cpp \
    engine/shapes/sphere.cpp \
    engine/shapes/cone.cpp \
    engine/shapes/facecube.cpp \
    engine/collisions/collisioncylinder.cpp \
    engine/collisions/geometriccollisionmanager.cpp \
    engine/collisions/ellipsoid.cpp \
    engine/collisions/collisionmanager.cpp \
    engine/collisions/triangle.cpp \
    engine/collisions/collisionsphere.cpp \
    engine/verlet/link.cpp \
    engine/verlet/net.cpp \
    engine/verlet/rope.cpp \
    engine/verlet/verlet.cpp \
    engine/verlet/verletcube.cpp \
    engine/verlet/verletmanager.cpp \
### game
    game/gamescreens/gamescreen.cpp \
    game/entities/gameplayer.cpp \
    game/gamescreens/gamemenu.cpp \
    game/world/gameworld.cpp \
    game/gamescreens/testlevelscreen.cpp \
    engine/common/raytracer.cpp \
    engine/shapes/mesh.cpp \
    engine/verlet/trianglemesh.cpp \
    engine/common/ray.cpp

HEADERS += \
    engine/ui/mainwindow.h \
    engine/ui/view.h \
    engine/ui/application.h \
    engine/ui/screen.h \
    engine/ui/button.h \
    engine/common/point3d.h \
    engine/common/perlinnoise.h \
    engine/objects/obj.h \
    engine/objects/objecthandler.h \
    engine/objects/navmeshhandler.h \
    engine/graphics/graphics.h \
    engine/graphics/cubemap.h \
    engine/graphics/camera.h \
    engine/graphics/actioncamera.h \
    engine/graphics/particleemitter.h \
    engine/graphics/meshbuffer.h \
    engine/world/world.h \
    engine/world/entity.h \
    engine/world/movableentity.h \
    engine/world/staticentity.h \
    engine/world/manager.h \
    engine/world/player.h \
    engine/shapes/shape.h \
    engine/shapes/cube.h \
    engine/shapes/cylinder.h \
    engine/shapes/sphere.h \
    engine/shapes/cone.h \
    engine/shapes/facecube.h \
    engine/collisions/collisionshape.h \
    engine/collisions/collisioncylinder.h \
    engine/collisions/triangle.h \
    engine/collisions/geometriccollisionmanager.h \
    engine/collisions/ellipsoid.h \
    engine/collisions/collisionmanager.h \
    engine/collisions/collisionsphere.h \
    engine/verlet/link.h \
    engine/verlet/net.h \
    engine/verlet/rope.h \
    engine/verlet/verlet.h \
    engine/verlet/verletcube.h \
    engine/verlet/verletmanager.h \
### game
    game/gamescreens/gamescreen.h \
    game/gamescreens/gamemenu.h \
    game/world/gameworld.h \
    game/entities/gameplayer.h \
    engine/common/debugprinting.h \
    game/gamescreens/testlevelscreen.h \
    engine/common/raytracer.h \
    engine/shapes/mesh.h \
    engine/verlet/trianglemesh.h \
    engine/common/ray.h


FORMS += engine/ui/mainwindow.ui

#TODO (Windows): If you are setting up local development on Windows (NOT Mac), comment out the following lines
win32:CONFIG(release, debug|release): LIBS += -L/course/cs123/lib/glew/glew-1.10.0/lib/release/ -lGLEW
else:win32:CONFIG(debug, debug|release): LIBS += -L/course/cs123/lib/glew/glew-1.10.0/lib/debug/ -lGLEW
else:unix: LIBS += -L/usr/local/Cellar/glew/1.11.0/lib/ -lGLEW

#TODO (Windows or Mac): If you are setting up local development on Windows OR Mac, fill in the correct path to your glew and uncomment the following lines:
INCLUDEPATH+=/usr/local/Cellar/glew/1.11.0/include
DEPENDPATH+=/usr/local/Cellar/glew/1.11.0/include

RESOURCES += \
    resources.qrc

################################ LEAP #################################
macx {
    LIBS += -L$$PWD/leap/ -lLeap

    INCLUDEPATH +=  $$PWD/leap/include \
                    engine/leap

    DEPENDPATH +=   $$PWD/leap/include \
                    engine/leap
}

################################ CUDA #################################
