TARGET      = bbsupport
QPRO_PWD    = $$PWD
QT          = core

include(../qbase.pri)

BLACKBERRY_SUPPORT_MAJOR_VERSION = $${QT_MAJOR_VERSION}
BLACKBERRY_SUPPORT_MINOR_VERSION = 0
BLACKBERRY_SUPPORT_PATCH_VERSION = 0
VERSION=$${BLACKBERRY_SUPPORT_MAJOR_VERSION}.$${BLACKBERRY_SUPPORT_MINOR_VERSION}.$${BLACKBERRY_SUPPORT_PATCH_VERSION}

DEPENDPATH += .
DEFINES += MAKE_BLACKBERRY_SUPPORT_LIB

BBSUPPORT_DIR = $$QT_SOURCE_TREE/src/3rdparty/bbsupport/bbsupport

blackberry*:QMAKE_PKGCONFIG_REQUIRES = QtCore

# Input
SOURCES +=  $$BBSUPPORT_DIR/keyboard.cpp \
            $$BBSUPPORT_DIR/notification.cpp \
            $$BBSUPPORT_DIR/notificationmanager.cpp \
            $$BBSUPPORT_DIR/ppsfile.cpp \
            $$BBSUPPORT_DIR/ppsobject.cpp \
            $$BBSUPPORT_DIR/ppsthread.cpp \
            $$BBSUPPORT_DIR/navigator.cpp

HEADERS +=  $$BBSUPPORT_DIR/keyboard.h \
            $$BBSUPPORT_DIR/notification.h \
            $$BBSUPPORT_DIR/notificationmanager.h \
            $$BBSUPPORT_DIR/ppsfile.h \
            $$BBSUPPORT_DIR/ppsobject.h \
            $$BBSUPPORT_DIR/ppsthread.h \
            $$BBSUPPORT_DIR/navigator.h

INCLUDEPATH += $$PWD
LIBS += -lpps

contains(QT_CONFIG, reduce_exports): CONFIG += hide_symbols

