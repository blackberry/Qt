/****************************************************************************
**
** Copyright (C) 2011 Research In Motion Limited
**
** This file is part of the plugins of the Qt Toolkit.
**
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
****************************************************************************/

// #define QBBINTEGRATION_DEBUG

#include "qapplication.h"
#include "qbbintegration.h"
#include "qbbinputcontext.h"
#include "qbbeventthread.h"
#include "qbbscreen.h"
#include "qbbwindow.h"
#include "qbbwindowsurface.h"
#include "qbbvirtualkeyboard.h"
#include "qgenericunixfontdatabase.h"
#include "qbbclipboard.h"
#include "qbbglcontext.h"

#include <QtGui/private/qpixmap_raster_p.h>
#include <QtGui/QPlatformWindow>
#include <QtGui/QWindowSystemInterface>
#include <QDebug>

#include <bbsupport/Navigator>
#include <bbsupport/Keyboard>

#include <errno.h>

QT_BEGIN_NAMESPACE

QBBIntegration::QBBIntegration() :
    QObject(0),
    mFontDb(new QGenericUnixFontDatabase())
{
    // initialize global OpenGL resources
    QBBGLContext::initialize();

    // open connection to QNX composition manager
    errno = 0;
    int result = screen_create_context(&mContext, SCREEN_APPLICATION_CONTEXT);
    if (result != 0) {
        qFatal("QBB: failed to connect to composition manager, errno=%d", errno);
    }

    // Create displays for all possible screens (which may not be attached)
    QBBScreen::createDisplays(mContext);

    // create/start event thread
    mEventThread = new QBBEventThread(mContext, *QBBScreen::primaryDisplay());
    mEventThread->start();

    // Start the navigator connection.
    QString primaryId = QBBScreen::primaryDisplay()->windowGroupName();
    QString secondaryId;
    if (QBBScreen::screens().length() > 1)
        secondaryId = ((QBBScreen*)(QBBScreen::screens().at(1)))->windowGroupName();

    // Connect the navigator signals.
    connect(&BlackBerry::Navigator::instance(), SIGNAL(notifyStartOrientation(int, QString const&)),
            this, SLOT(onStartOrientation(int, QString const&)));
    connect(&BlackBerry::Navigator::instance(), SIGNAL(notifyEndOrientation(QString const&)),
            this, SLOT(onEndOrientation(QString const&)));
    connect(&BlackBerry::Navigator::instance(), SIGNAL(notifySwipeDown()),
            this, SLOT(onSwipeDown()));
    connect(&BlackBerry::Navigator::instance(), SIGNAL(notifyExit()),
            this, SLOT(onExit()));

    // Connect the keyboard signals.
    connect(&BlackBerry::Keyboard::instance(), SIGNAL(notifyVisibilityChanged(bool)),
            this, SLOT(onKeyboardVisibilityChanged(bool)));
    connect(&BlackBerry::Keyboard::instance(), SIGNAL(notifyHeightChanged(int)),
            this, SLOT(onKeyboardHeightChanged(int)));

    // Set up the group ids.
    BlackBerry::Navigator::instance().setGroupIds(primaryId, secondaryId);

    // Set up the input context
    qApp->setInputContext(new QBBInputContext(qApp));
}

QBBIntegration::~QBBIntegration()
{
#if defined(QBBINTEGRATION_DEBUG)
    qDebug() << "QBB: platform plugin shutdown begin";
#endif

    // Disconnect all signals.
    disconnect(&BlackBerry::Navigator::instance(), 0, this, 0);
    disconnect(&BlackBerry::Keyboard::instance(), 0, this, 0);

    // stop/destroy event thread
    delete mEventThread;

    // destroy all displays
    QBBScreen::destroyDisplays();

    // close connection to QNX composition manager
    screen_destroy_context(mContext);

    // cleanup global OpenGL resources
    QBBGLContext::shutdown();

#if defined(QBBINTEGRATION_DEBUG)
    qDebug() << "QBB: platform plugin shutdown end";
#endif
}

bool QBBIntegration::hasCapability(QPlatformIntegration::Capability cap) const
{
    switch (cap) {
    case ThreadedPixmaps: return true;
#if defined(QT_OPENGL_ES_2)
    case OpenGL: return true;
#endif
    default: return QPlatformIntegration::hasCapability(cap);
    }
}

QPixmapData *QBBIntegration::createPixmapData(QPixmapData::PixelType type) const
{
    return new QRasterPixmapData(type);
}

QPlatformWindow *QBBIntegration::createPlatformWindow(QWidget *widget, WId winId) const
{
    Q_UNUSED(winId);

    // New windows are created on the primary display.
    return new QBBWindow(widget, mContext);
}

QWindowSurface *QBBIntegration::createWindowSurface(QWidget *widget, WId winId) const
{
    Q_UNUSED(winId);
    return new QBBWindowSurface(widget);
}

void QBBIntegration::moveToScreen(QWidget *window, int screen)
{
#if defined(QBBINTEGRATION_DEBUG)
    qDebug() << "QBBIntegration::moveToScreen - w=" << window << ", s=" << screen;
#endif

    // get platform window used by widget
    QBBWindow* platformWindow = static_cast<QBBWindow*>(window->platformWindow());

    // lookup platform screen by index
    QBBScreen* platformScreen = static_cast<QBBScreen*>(QBBScreen::screens().at(screen));

    // move the platform window to the platform screen
    platformWindow->setScreen(platformScreen);
}

QList<QPlatformScreen *> QBBIntegration::screens() const
{
    return QBBScreen::screens();
}

#ifndef QT_NO_CLIPBOARD
QPlatformClipboard *QBBIntegration::clipboard() const
{
    static QPlatformClipboard *clipboard = 0;
    if (!clipboard) {
        clipboard = static_cast<QPlatformClipboard *>(new QBBClipboard);
    }
    return clipboard;
}
#endif

void QBBIntegration::onStartOrientation(int angle, QString const& id)
{
    // TODO: We should rotate, paint once, and then send the buffer resized message.
    //       We then need to wait for onEndOrientation
#if defined(QBBINTEGRATION_DEBUG)
    qDebug() << "QBBIntegration: start orientation (" << angle << ") id=" << id;
#endif

    QBBScreen::primaryDisplay()->setRotation( angle );
    QWindowSystemInterface::handleScreenGeometryChange(0);
    BlackBerry::Navigator::instance().sendBufferResized(id);
}

void QBBIntegration::onEndOrientation(QString const& id)
{
    Q_UNUSED(id)

    // TODO: Resume painting.
#if defined(QBBINTEGRATION_DEBUG)
    qDebug() << "QBBIntegration: end orientation";
#endif
}

void QBBIntegration::onSwipeDown()
{
    // simulate menu key press
#if defined(QBBINTEGRATION_DEBUG)
    qDebug() << "QBBIntegration: menu";
#endif

    QWidget *w = QApplication::activeWindow();
    QWindowSystemInterface::handleKeyEvent(w, QEvent::KeyPress, Qt::Key_Menu, Qt::NoModifier);
    QWindowSystemInterface::handleKeyEvent(w, QEvent::KeyRelease, Qt::Key_Menu, Qt::NoModifier);
}

void QBBIntegration::onExit()
{
    // shutdown everything
#if defined(QBBINTEGRATION_DEBUG)
    qDebug() << "QBBIntegration: exit";
#endif

    QApplication::quit();
}

void QBBIntegration::onKeyboardVisibilityChanged(bool visibility)
{
#if defined(QBBINTEGRATION_DEBUG)
    qDebug() << "QBBIntegration: onKeyboardVisibilityChanged(" << visibility << ")";
#else
    Q_UNUSED(visibility)
#endif

    // Virtual keyboard is always on the primary display, so notify
    // that its geometry has changed.
    QWindowSystemInterface::handleScreenAvailableGeometryChange(0);
}

void QBBIntegration::onKeyboardHeightChanged(int height)
{
#if defined(QBBINTEGRATION_DEBUG)
    qDebug() << "QBBIntegration: onKeyboardHeightChanged(" << height << ")";
#else
    Q_UNUSED(height)
#endif

    // Virtual keyboard is always on the primary display, so notify
    // that its geometry has changed.
    QWindowSystemInterface::handleScreenAvailableGeometryChange(0);
}


QT_END_NAMESPACE
