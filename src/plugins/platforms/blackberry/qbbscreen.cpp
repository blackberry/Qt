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

// #define QBBSCREEN_DEBUG

#include "qbbscreen.h"
#include "qbbvirtualkeyboard.h"
#include "qbbwindow.h"

#include <QUuid>
#include <QDebug>

#include <errno.h>
#include <unistd.h>

QT_BEGIN_NAMESPACE

#define MAGIC_ZORDER_FOR_NO_NAV     10

QList<QPlatformScreen *> QBBScreen::sScreens;
QList<QBBWindow*> QBBScreen::sChildren;

QBBScreen::QBBScreen(screen_context_t context, screen_display_t display)
    : mContext(context),
      mDisplay(display),
      mPosted(false)
{
#if defined(QBBSCREEN_DEBUG)
    qDebug() << "QBBScreen::QBBScreen";
#endif

    // cache initial orientation of this display
    // TODO: use ORIENTATION environment variable?
    errno = 0;
    int result = screen_get_display_property_iv(mDisplay, SCREEN_PROPERTY_ROTATION, &mStartRotation);
    if (result != 0) {
        qFatal("QBBScreen: failed to query display rotation, errno=%d", errno);
    }

    mCurrentRotation = mStartRotation;

    // cache size of this display in pixels
    // TODO: use WIDTH and HEIGHT environment variables?
    errno = 0;
    int val[2];
    result = screen_get_display_property_iv(mDisplay, SCREEN_PROPERTY_SIZE, val);
    if (result != 0) {
        qFatal("QBBScreen: failed to query display size, errno=%d", errno);
    }

    mCurrentGeometry = mStartGeometry = QRect(0, 0, val[0], val[1]);

    // cache size of this display in millimeters
    errno = 0;
    result = screen_get_display_property_iv(mDisplay, SCREEN_PROPERTY_PHYSICAL_SIZE, val);
    if (result != 0) {
        qFatal("QBBScreen: failed to query display physical size, errno=%d", errno);
    }

    // Peg the DPI to 96 (for now) so fonts are a reasonable size. We'll want to match
    // everything with a QStyle later, and at that point the physical size can be used
    // instead.
    {
        static const int dpi = 96;
        int width = mCurrentGeometry.width() / dpi * qreal(25.4) ;
        int height = mCurrentGeometry.height() / dpi * qreal(25.4) ;

        mCurrentPhysicalSize = mStartPhysicalSize = QSize(width,height);
    }

    // create one top-level QNX window to act as a container for child windows
    // since navigator only supports one application window
    errno = 0;
    result = screen_create_window(&mAppWindow, mContext);
    if (result != 0) {
        qFatal("QBBScreen: failed to create window, errno=%d", errno);
    }

    // move window to proper display
    errno = 0;
    result = screen_set_window_property_pv(mAppWindow, SCREEN_PROPERTY_DISPLAY, (void **)&mDisplay);
    if (result != 0) {
        qFatal("QBBScreen: failed to set window display, errno=%d", errno);
    }

    // make sure window is above navigator but below keyboard if running as root
    // since navigator won't automatically set our z-order in this case
    if (getuid() == 0) {
        errno = 0;
        val[0] = MAGIC_ZORDER_FOR_NO_NAV;
        result = screen_set_window_property_iv(mAppWindow, SCREEN_PROPERTY_ZORDER, val);
        if (result != 0) {
            qFatal("QBBScreen: failed to set window z-order, errno=%d", errno);
        }
    }

    // window won't be visible unless it has some buffers so make one dummy buffer
    // that is 1x1
    errno = 0;

    val[0] = SCREEN_USAGE_NATIVE;

    result = screen_set_window_property_iv(mAppWindow, SCREEN_PROPERTY_USAGE, val);
    if (result != 0) {
        qFatal("QBBScreen: failed to set window buffer usage, errno=%d", errno);
    }

    errno = 0;
    val[0] = nativeFormat();
    result = screen_set_window_property_iv(mAppWindow, SCREEN_PROPERTY_FORMAT, val);
    if (result != 0) {
        qFatal("QBBScreen: failed to set window pixel format, errno=%d", errno);
    }

    errno = 0;
    val[0] = 1;
    val[1] = 1;
    result = screen_set_window_property_iv(mAppWindow, SCREEN_PROPERTY_BUFFER_SIZE, val);
    if (result != 0) {
        qFatal("QBBScreen: failed to set window buffer size, errno=%d", errno);
    }

    errno = 0;
    result = screen_create_window_buffers(mAppWindow, 1);
    if (result != 0) {
        qFatal("QBBScreen: failed to create bufferless window, errno=%d", errno);
    }

    // window is always the size of the display
    errno = 0;
    val[0] = mCurrentGeometry.width();
    val[1] = mCurrentGeometry.height();
    result = screen_set_window_property_iv(mAppWindow, SCREEN_PROPERTY_SIZE, val);
    if (result != 0) {
        qFatal("QBBScreen: failed to set window size, errno=%d", errno);
    }

    // fill the window with solid black
    errno = 0;
    val[0] = 0;
    result = screen_set_window_property_iv(mAppWindow, SCREEN_PROPERTY_COLOR, val);
    if (result != 0) {
        qFatal("QBBScreen: failed to set window colour, errno=%d", errno);
    }

    // make the window opaque
    errno = 0;
    val[0] = SCREEN_TRANSPARENCY_NONE;
    result = screen_set_window_property_iv(mAppWindow, SCREEN_PROPERTY_TRANSPARENCY, val);
    if (result != 0) {
        qFatal("QBBScreen: failed to set window transparency, errno=%d", errno);
    }

    // set the swap interval to 1
    errno = 0;
    val[0] = 1;
    result = screen_set_window_property_iv(mAppWindow, SCREEN_PROPERTY_SWAP_INTERVAL, val);
    if (result != 0) {
        qFatal("QBBScreen: failed to set window swap interval, errno=%d", errno);
    }

    // set viewport size equal to window size but move outside buffer so the fill
    // colour is used exclusively
    errno = 0;
    val[0] = mCurrentGeometry.width();
    val[1] = mCurrentGeometry.height();
    result = screen_set_window_property_iv(mAppWindow, SCREEN_PROPERTY_SOURCE_SIZE, val);
    if (result != 0) {
        qFatal("QBBScreen: failed to set window source size, errno=%d", errno);
    }

    errno = 0;
    val[0] = 1;
    val[1] = 0;
    result = screen_set_window_property_iv(mAppWindow, SCREEN_PROPERTY_SOURCE_POSITION, val);
    if (result != 0) {
        qFatal("QBBScreen: failed to set window source position, errno=%d", errno);
    }

    // generate a random window group name
    QUuid uuid;
    mWindowGroupName = QUuid::createUuid().toString().toAscii();

    // create window group so child windows can be parented by container window
    errno = 0;
    result = screen_create_window_group(mAppWindow, windowGroupName());
    if (result != 0) {
        qFatal("QBBScreen: failed to create app window group, errno=%d", errno);
    }

    errno = 0;
    screen_buffer_t buffer;
    result = screen_get_window_property_pv(mAppWindow, SCREEN_PROPERTY_RENDER_BUFFERS, (void **)&buffer);
    if (result != 0) {
        qFatal("QBBScreen: failed to query window buffer, errno=%d", errno);
    }

    errno = 0;
    int dirtyRect[] = {0, 0, 1, 1};
    result = screen_post_window(mAppWindow, buffer, 1, dirtyRect, 0);
    if (result != 0) {
        qFatal("QBB: failed to post window buffer, errno=%d", errno);
    }
}

QBBScreen::~QBBScreen()
{
#if defined(QBBSCREEN_DEBUG)
    qDebug() << "QBBScreen::~QBBScreen";
#endif

    // cleanup top-level QNX window
    screen_destroy_window(mAppWindow);
}

/* static */
void QBBScreen::createDisplays(screen_context_t context)
{
    // get number of displays
    errno = 0;
    int displayCount;
    int result = screen_get_context_property_iv(context, SCREEN_PROPERTY_DISPLAY_COUNT, &displayCount);
    if (result != 0) {
        qFatal("QBBScreen: failed to query display count, errno=%d", errno);
    }

    // get all displays
    errno = 0;
    screen_display_t *displays = (screen_display_t *)alloca(sizeof(screen_display_t) * displayCount);
    result = screen_get_context_property_pv(context, SCREEN_PROPERTY_DISPLAYS, (void **)displays);
    if (result != 0) {
        qFatal("QBBScreen: failed to query displays, errno=%d", errno);
    }

    for (int i=0; i<displayCount; i++) {
#if defined(QBBSCREEN_DEBUG)
        qDebug() << "QBBScreen::Creating screen for display " << i;
#endif
        QBBScreen *screen = new QBBScreen(context, displays[i]);
        sScreens.push_back(screen);
    }
}

/* static */
void QBBScreen::destroyDisplays()
{
    while (sScreens.length()) {
        delete sScreens.front();
        sScreens.pop_front();
    }

    // We're not managing the child windows anymore so we need to clear the list.
    sChildren.clear();
}

/* static */
int QBBScreen::defaultDepth()
{
    static int defaultDepth = 0;
    if (defaultDepth == 0) {
        // check if display depth was specified in environment variable;
        // use default value if no valid value found
        defaultDepth = qgetenv("QBB_DISPLAY_DEPTH").toInt();
        if (defaultDepth != 16 && defaultDepth != 32) {
            defaultDepth = 32;
        }
    }
    return defaultDepth;
}

QRect QBBScreen::availableGeometry() const
{
    // available geometry = total geometry - keyboard
    int keyboardHeight = QBBVirtualKeyboard::instance().getHeight();
    return QRect(mCurrentGeometry.x(), mCurrentGeometry.y(),
                 mCurrentGeometry.width(), mCurrentGeometry.height() - keyboardHeight);
}

void QBBScreen::setRotation(int rotation)
{
#if defined(QBBSCREEN_DEBUG)
    qDebug() << "QBBScreen::setRotation, o=" << rotation;
#endif

    // check if rotation changed
    if (mCurrentRotation != rotation) {

        // update rotation of app window
        errno = 0;
        int result = screen_set_window_property_iv(mAppWindow, SCREEN_PROPERTY_ROTATION, &rotation);
        if (result != 0) {
            qFatal("QBBScreen: failed to set window rotation, errno=%d", errno);
        }

        // swap dimensions if we've rotated 90 or 270 from initial orientation
        if(orthogonal(mStartRotation, rotation)) {
            mCurrentGeometry = QRect(0, 0, mStartGeometry.height(), mStartGeometry.width());
            mCurrentPhysicalSize = QSize(mStartPhysicalSize.height(), mStartPhysicalSize.width());
        } else {
            mCurrentGeometry = QRect(0, 0, mStartGeometry.width(), mStartGeometry.height());
            mCurrentPhysicalSize = mStartPhysicalSize;
        }

        // resize app window if we've rotated 90 or 270 from previous orientation
        if (orthogonal(mCurrentRotation, rotation)) {

#if defined(QBBSCREEN_DEBUG)
            qDebug() << "QBBScreen::setRotation - resize, s=" << mCurrentGeometry.size();
#endif
            errno = 0;
            int val[] = {mCurrentGeometry.width(), mCurrentGeometry.height()};
            result = screen_set_window_property_iv(mAppWindow, SCREEN_PROPERTY_SIZE, val);
            if (result != 0) {
                qFatal("QBBScreen: failed to set window size, errno=%d", errno);
            }

            errno = 0;
            result = screen_set_window_property_iv(mAppWindow, SCREEN_PROPERTY_SOURCE_SIZE, val);
            if (result != 0) {
                qFatal("QBBScreen: failed to set window source size, errno=%d", errno);
            }

            // NOTE: display will update when child windows relayout and repaint

        } else {

            // TODO: find one global place to flush display updates
#if defined(QBBSCREEN_DEBUG)
            qDebug() << "QBBScreen::setRotation - flush";
#endif
            // force immediate display update if no geometry changes required
            errno = 0;
            int result = screen_flush_context(mContext, 0);
            if (result != 0) {
                qFatal("QBBScreen: failed to flush context, errno=%d", errno);
            }
        }

        // save new rotation
        mCurrentRotation = rotation;
    }
}

/*!
    Check if the supplied angles are perpendicular to each other.
*/
bool QBBScreen::orthogonal(int rotation1, int rotation2)
{
    return ((rotation1 - rotation2) % 180) != 0;
}

void QBBScreen::addWindow(QBBWindow* window)
{
#if defined(QBBSCREEN_DEBUG)
    qDebug() << "QBBScreen::addWindow=" << window;
#endif

    if (sChildren.contains(window))
        return;

    sChildren.push_back(window);
    QBBScreen::updateHierarchy();
}

void QBBScreen::removeWindow(QBBWindow* window)
{
#if defined(QBBSCREEN_DEBUG)
    qDebug() << "QBBScreen::removeWindow=" << window;
#endif

    sChildren.removeAll(window);
    QBBScreen::updateHierarchy();
}

void QBBScreen::raiseWindow(QBBWindow* window)
{
#if defined(QBBSCREEN_DEBUG)
    qDebug() << "QBBScreen::raise window=" << window;
#endif

    removeWindow(window);
    sChildren.push_back(window);
    QBBScreen::updateHierarchy();
}

void QBBScreen::lowerWindow(QBBWindow* window)
{
#if defined(QBBSCREEN_DEBUG)
    qDebug() << "QBBScreen::lower window=" << window;
#endif

    removeWindow(window);
    sChildren.push_front(window);
    QBBScreen::updateHierarchy();
}

void QBBScreen::updateHierarchy()
{
#if defined(QBBSCREEN_DEBUG)
    qDebug() << "QBBScreen::updateHierarchy";
#endif

    QList<QBBWindow*>::iterator it;
    int topZorder = 0;

    for (it = sChildren.begin(); it != sChildren.end(); it++)
        (*it)->updateZorder(topZorder);

    // After a hierarchy update, we need to force a flush on all screens.
    // Right now, all screens share a context.
    screen_flush_context( primaryDisplay()->mContext, 0 );
}

void QBBScreen::onWindowPost(QBBWindow* window)
{
    Q_UNUSED(window)

    // post app window (so navigator will show it) after first child window
    // has posted; this only needs to happen once as the app window's content
    // never changes
    if (!mPosted) {

        errno = 0;
        screen_buffer_t buffer;
        int result = screen_get_window_property_pv(mAppWindow, SCREEN_PROPERTY_RENDER_BUFFERS, (void **)&buffer);
        if (result != 0) {
            qFatal("QBB: failed to query window buffer, errno=%d", errno);
        }

        errno = 0;
        int dirtyRect[] = {0, 0, 1, 1};
        result = screen_post_window(mAppWindow, buffer, 1, dirtyRect, 0);
        if (result != 0) {
            qFatal("QBB: failed to post window buffer, errno=%d", errno);
        }

        mPosted = true;
    }
}

QT_END_NAMESPACE
