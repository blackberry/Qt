TARGET = blackberry
include(../../qpluginbase.pri)

QTDIR_build:DESTDIR = $$QT_BUILD_TREE/plugins/platforms

SOURCES =   main.cpp \
            qbbbuffer.cpp \
            qbbeventthread.cpp \
            qbbinputcontext.cpp \
            qbbintegration.cpp \
            qbbglcontext.cpp \
            qbbscreen.cpp \
            qbbwindow.cpp \
            qbbwindowsurface.cpp \
            qbbclipboard.cpp

HEADERS =   qbbbuffer.h \
            qbbeventthread.h \
            qbbinputcontext.h \
            qbbintegration.h \
            qbbglcontext.h \
            qbbscreen.h \
            qbbwindow.h \
            qbbwindowsurface.h \
            qbbclipboard.h

LIBS += -lpps -lscreen -lEGL -lclipboard -lbbsupport

include (../eglconvenience/eglconvenience.pri)
include (../fontdatabases/genericunix/genericunix.pri)

target.path += $$[QT_INSTALL_PLUGINS]/platforms
INSTALLS += target
