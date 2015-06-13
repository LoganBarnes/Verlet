QT += core gui opengl

TARGET = cs1972final
TEMPLATE = app

# project build directories
DESTDIR     = $$system(pwd) # target dir
BUILDDIR    = $$DESTDIR/build

MOC_DIR     = $$BUILDDIR # moc_...
RCC_DIR     = $$BUILDDIR # qrc_resources.cpp
UI_DIR      = $$BUILDDIR # ui_mainwindow.cpp

OBJECTS_DIR = $$BUILDDIR/bin # .o files

QMAKE_CXXFLAGS += -std=c++11
unix:!macx {
    NON_CUDA_LIBS += -lGLU -lalut
    #glew
    NON_CUDA_LIBS += -L/usr/local/Cellar/glew/1.11.0/lib/ -lGLEW
    LIBS += $$NON_CUDA_LIBS
    DEFINES += LINUX
}
macx {
    NON_CUDA_LIBS += -stdlib=libc++ -framework OpenAL
    #glew
    #If you are setting up local development on Mac, fill in the correct path to your glew and uncomment the following lines:
    NON_CUDA_LIBS += -L/usr/local/Cellar/glew/1.11.0/lib/ -lGLEW
    INCLUDEPATH+=/usr/local/Cellar/glew/1.11.0/include
    DEPENDPATH+=/usr/local/Cellar/glew/1.11.0/include

    LIBS += $$NON_CUDA_LIBS

    QMAKE_CXXFLAGS += -Wno-c++11-extensions
    QMAKE_CXXFLAGS += -stdlib=libc++
    QMAKE_CXXFLAGS += -mmacosx-version-min=10.7
    QMAKE_LFLAGS += -mmacosx-version-min=10.7
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.7

    MAC_SDK  = /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.9.sdk/
    if( exists( $$MAC_SDK) ) {
        QMAKE_MAC_SDK = macosx10.9
    }
}
win32 {
    #Note: using the 32 bit version of minGW
    QMAKE_CXXFLAGS += -Wno-c++11-extensions

    #OpenAL
    OPENAL_DIR = $$PWD/lib/openal
    NON_CUDA_LIBS += -L$$OPENAL_DIR/libs/Win32/ -lOpenAL32
    INCLUDEPATH +=  $$OPENAL_DIR/include

    #glew
    #If you are setting up local development on Windows, fill in the correct path to your glew and uncomment the following lines:
    #Hacky solution: put the libglew32.dll where the executable is (build.../debug/)
    INCLUDEPATH+=C:/GameEngine/glew-gcc-1.10.0-win32/include
    DEPENDPATH+=C:/GameEngine/glew-gcc-1.10.0-win32/bin
    DEPENDPATH+=C:/GameEngine/glew-gcc-1.10.0-win32/lib
    NON_CUDA_LIBS += -L"C:/GameEngine/glew-gcc-1.10.0-win32/lib" -lglew32
    LIBS += $$NON_CUDA_LIBS
}

QMAKE_CXXFLAGS += -isystem "lib/glm"

INCLUDEPATH +=  lib/glm engine game shaders \
                engine/common \
                engine/ui \
                engine/graphics \
                engine/shapes \
                engine/world \
                engine/collisions \
                engine/objects \
                engine/verlet \
                engine/sound \
                engine/particles_cuda \
                game/gamescreens \
                game/world \
                game/entities \
                res/images \
                res/images/cubemap \
                res/levels

DEPENDPATH +=   lib/glm engine game shaders \
                engine/common \
                engine/ui \
                engine/graphics \
                engine/shapes \
                engine/world \
                engine/collisions \
                engine/objects \
                engine/verlet \
                engine/sound \
                engine/particles_cuda \
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
    engine/shapes/mesh.cpp \
    engine/verlet/trianglemesh.cpp \
    engine/common/ray.cpp \
    engine/sound/audio.cpp \
    engine/common/wavreader.cpp \
    game/entities/soundtester.cpp \
    engine/objects/half.cpp \
    engine/verlet/grass.cpp \
    engine/objects/lightparser.cpp \
    game/entities/marker.cpp \
    game/gamescreens/leveltwo.cpp \
    engine/verlet/bend.cpp \
    game/entities/token.cpp \
    game/entities/spiralsensor.cpp \
    game/world/level1.cpp

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
    engine/shapes/mesh.h \
    engine/verlet/trianglemesh.h \
    engine/common/ray.h \
    engine/sound/audio.h \
    engine/common/wavreader.h \
    game/entities/soundtester.h \
    engine/objects/half.h \
    engine/verlet/grass.h \
    engine/objects/lightparser.h \
    game/entities/marker.h \
    game/gamescreens/leveltwo.h \
    engine/verlet/bend.h \
    game/entities/token.h \
    game/entities/spiralsensor.h \
    game/world/level1.h


FORMS += engine/ui/mainwindow.ui

RESOURCES += \
    resources.qrc

################################ LEAP #################################
macx {
    LEAP_DIR = $$PWD/lib/leap
    LIBS += -L$$LEAP_DIR/ -lLeap

    DEFINES += LEAP
    message("configuring for leap");

    INCLUDEPATH +=  $$LEAP_DIR/include \
                    engine/leap

    DEPENDPATH +=   $$LEAP_DIR/include \
                    engine/leap
}

################################ CUDA #################################

unix:!macx {
    # Path to cuda stuff
    CUDA_DIR = /contrib/projects/cuda5-toolkit
    CUDA_LIB = $$CUDA_DIR/lib64

    # GPU architecture
    CUDA_ARCH     = sm_21 # should be able to detect this somehow instead of hardcoding

    SED_STUFF = 2>&1 | sed -r \"s/\\(([0-9]+)\\)/:\\1/g\" 1>&2
}
macx {
    # Path to cuda stuff
    CUDA_DIR = /usr/local/cuda
    CUDA_LIB = $$CUDA_DIR/lib

    # GPU architecture
    CUDA_ARCH     = sm_30 # should be able to detect this somehow instead of hardcoding

    SED_STUFF = 2>&1 | sed -E \"s/\\(([0-9]+)\\)/:\\1/g\" 1>&2

    NVCCFLAGS += --std=c++11
}
win32 {
    # Path to cuda stuff
    CUDA_DIR = /usr/local/cuda
    CUDA_LIB = $$CUDA_DIR/lib
    # GPU architecture
    CUDA_ARCH     = sm_21 # should be able to detect this somehow instead of hardcoding
    # Path to cuda stuff
    SED_STUFF = 2>&1 | sed -E \"s/\\(([0-9]+)\\)/:\\1/g\" 1>&2
}

if ( exists( $$CUDA_DIR/ ) ) {
    message( "Configuring for cuda..." );
    DEFINES += CUDA

    SOURCES += \
               engine/particles_cuda/particlesystemmanager.cpp \
               engine/particles_cuda/particlerenderer.cpp \
               engine/particles_cuda/particlesystem.cpp

    HEADERS += \
               engine/particles_cuda/particlesystemmanager.h \
               engine/particles_cuda/particlerenderer.h \
               engine/particles_cuda/particlesystem.h

    # Cuda sources
    CUDA_SOURCES += \
                    engine/particles_cuda/shared_variables.cu \
                    engine/particles_cuda/integration.cu \
                    engine/particles_cuda/solver.cu \
                    engine/particles_cuda/util.cu

    OTHER_FILES +=  \
                    engine/particles_cuda/wrappers.cuh \
                    engine/particles_cuda/integration_kernel.cuh \
                    engine/particles_cuda/solver_kernel.cuh \
                    engine/particles_cuda/kernel.cuh \
                    engine/particles_cuda/util.cuh \
                    engine/particles_cuda/util.cu \
                    engine/particles_cuda/integration.cu \
                    engine/particles_cuda/solver.cu \
                    engine/particles_cuda/shared_variables.cu \
                    engine/particles_cuda/shared_variables.cuh \
                    engine/particles_cuda/helper_cuda.h \
                    engine/particles_cuda/helper_math.h

    # Pather to header and lib files
    INCLUDEPATH += $$CUDA_DIR/include
    QMAKE_LIBDIR += $$CUDA_LIB

    # prevents warnings from code we didn't write
    QMAKE_CXXFLAGS += -isystem $$CUDA_DIR/include

    LIBS += -lcudart -lcurand -lcublas -lcusparse
    QMAKE_LFLAGS += -Wl,-rpath,$$CUDA_LIB
    NVCCFLAGS = -Xlinker -rpath,$$CUDA_LIB

    # libs used in the code
    CUDA_LIBS = $$LIBS
    CUDA_LIBS -= $$NON_CUDA_LIBS
    # Some default NVCC flags?
    NVCCFLAGS     += --compiler-options -fno-strict-aliasing -use_fast_math --ptxas-options=-v

    # Prepare the extra compiler configuration (taken from the nvidia forum)
    CUDA_INC = $$join(INCLUDEPATH,' -I','-I',' ')

    cuda.commands = $$CUDA_DIR/bin/nvcc -m64 -O3 -arch=$$CUDA_ARCH -c $$NVCCFLAGS \
                    $$CUDA_INC $$CUDA_LIBS  ${QMAKE_FILE_NAME} -o ${QMAKE_FILE_OUT} \
                    $$SED_STUFF
    # nvcc error printout format ever so slightly different from gcc
    # http://forums.nvidia.com/index.php?showtopic=171651

    cuda.dependency_type = TYPE_C
    cuda.depend_command = $$CUDA_DIR/bin/nvcc -O3 -M $$CUDA_INC $$NVCCFLAGS   ${QMAKE_FILE_NAME}

    cuda.input = CUDA_SOURCES
    cuda.output = ${OBJECTS_DIR}${QMAKE_FILE_BASE}_cuda.o

    # Tell Qt that we want add more stuff to the Makefile
    QMAKE_EXTRA_COMPILERS += cuda
}
