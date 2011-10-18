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

//#define QBBWINDOW_DEBUG

#include "qbbwindow.h"
#include "qbbglcontext.h"
#include "qbbscreen.h"

#include <QtGui/QWidget>
#include <QDebug>
#include <QtGui/QWindowSystemInterface>

#include <errno.h>

QT_BEGIN_NAMESPACE

QBBWindow::QBBWindow(QWidget *window, screen_context_t context)
    : QPlatformWindow(window),
      mContext(context),
      mCurrentBufferIndex(-1),
      mPreviousBufferIndex(-1),
      mPlatformGlContext(NULL),
      mScreen(NULL),
      mParent(NULL),
      mVisible(true)
{
#if defined(QBBWINDOW_DEBUG)
    qDebug() << "QBBWindow::QBBWindow - w=" << window << ", s=" << window->size();
#endif
    int result;

    // create child QNX window
    errno = 0;
    result = screen_create_window_type(&mWindow, mContext, SCREEN_CHILD_WINDOW);
    if (result != 0) {
        qFatal("QBB: failed to create window, errno=%d", errno);
    }

    // set window buffer usage based on rendering API
    int val;
    QPlatformWindowFormat format = widget()->platformWindowFormat();
    switch (format.windowApi()) {
    case QPlatformWindowFormat::Raster:
        val = SCREEN_USAGE_NATIVE;
        break;
    case QPlatformWindowFormat::OpenGL:
        val = SCREEN_USAGE_OPENGL_ES2;
        break;
    default:
        qFatal("QBB: unsupported window API");
        break;
    }

    errno = 0;
    result = screen_set_window_property_iv(mWindow, SCREEN_PROPERTY_USAGE, &val);
    if (result != 0) {
        qFatal("QBB: failed to set window buffer usage, errno=%d", errno);
    }

    // setup pixel format for 32-bit ARGB PMA
    errno = 0;
    val = SCREEN_FORMAT_RGBA8888;
    result = screen_set_window_property_iv(mWindow, SCREEN_PROPERTY_FORMAT, &val);
    if (result != 0) {
        qFatal("QBB: failed to set window pixel format, errno=%d", errno);
    }

    errno = 0;
    val = SCREEN_PRE_MULTIPLIED_ALPHA;
    result = screen_set_window_property_iv(mWindow, SCREEN_PROPERTY_ALPHA_MODE, &val);
    if (result != 0) {
        qFatal("QBB: failed to set window alpha mode, errno=%d", errno);
    }

    // create OpenGL context if necessary
    if (format.windowApi() != QPlatformWindowFormat::Raster) {
        mPlatformGlContext = new QBBGLContext(this);
    }

    // Set the screen to the primary display (this is the default specified by screen).
    setScreen(QBBScreen::primaryDisplay());

    // Add the window to the root of the Hierarchy.
    QBBScreen::addWindow(this);
}

QBBWindow::~QBBWindow()
{
#if defined(QBBWINDOW_DEBUG)
    qDebug() << "QBBWindow::~QBBWindow - w=" << widget();
#endif

    // Remove from parent's Hierarchy.
    removeFromParent();
    QBBScreen::updateHierarchy();

    // We shouldn't allow this case unless QT allows it. Does it? Or should we send the
    // handleCloseEvent on all children when this window is deleted?
    if (mChildren.size() > 0)
        qFatal("QBB: window destroyed before children!");

    // cleanup OpenGL/OpenVG context if it exists
    if (mPlatformGlContext != NULL) {
        delete mPlatformGlContext;
    }

    // cleanup QNX window and its buffers
    screen_destroy_window(mWindow);
}

void QBBWindow::setGeometry(const QRect &rect)
{
    int val[2];

#if defined(QBBWINDOW_DEBUG)
    qDebug() << "QBBWindow::setGeometry - w=" << widget() << ", (" << rect.x() << "," << rect.y() << "," << rect.width() << "," << rect.height() << ")";
#endif

    QRect oldGeometry = geometry();

    // call parent method
    QPlatformWindow::setGeometry(rect);

    errno = 0;
    val[0] = rect.width();
    val[1] = rect.height();
    int result = screen_set_window_property_iv(mWindow, SCREEN_PROPERTY_SIZE, val);
    if (result != 0) {
        qFatal("QBB: failed to set window size, errno=%d", errno);
    }

    // set viewport size equal to window size
    errno = 0;
    result = screen_set_window_property_iv(mWindow, SCREEN_PROPERTY_SOURCE_SIZE, val);
    if (result != 0) {
        qFatal("QBB: failed to set window source size, errno=%d", errno);
    }

    sys_updatePosition();
}

void QBBWindow::sys_updatePosition() const
{
    QPoint offset(0,0);
    QBBWindow* current = (QBBWindow*)this;

    // Calculate our position in screen coordinates.
    while ((current->widget()->windowType() & Qt::SubWindow) == Qt::SubWindow && current->mParent)
    {
        offset += current->mParent->geometry().topLeft();
        current = current->mParent;
    }

    sys_updatePosition(offset);
}

void QBBWindow::sys_updatePosition(QPoint const&parentScreenOffset) const
{
    // Move self and then children.
    QRect screenGeometry = geometry();
    screenGeometry.translate(parentScreenOffset);

    int val[2];

    errno = 0;
    val[0] = screenGeometry.x();
    val[1] = screenGeometry.y();
    int result = screen_set_window_property_iv(mWindow, SCREEN_PROPERTY_POSITION, val);
    if (result != 0) {
        qFatal("QBB: failed to set window position, errno=%d", errno);
    }

    // Only subwindows will move - so skip everything else.
    QList<QBBWindow*>::const_iterator it;
    for (it = mChildren.begin(); it != mChildren.end(); it++) {
        if(((*it)->widget()->windowType() & Qt::SubWindow) == Qt::SubWindow)
            (*it)->sys_updatePosition(screenGeometry.topLeft());
    }
}
void QBBWindow::setVisible(bool visible)
{
#if defined(QBBWINDOW_DEBUG)
    qDebug() << "QBBWindow::setVisible - w=" << widget() << ", v=" << visible;
#endif

    mVisible = visible;

    QBBWindow* root = this;
    while(root->mParent)
        root = root->mParent;

    root->updateVisibility(root->mVisible);
}

void QBBWindow::updateVisibility(bool parentVisible)
{
    // set window visibility
    errno = 0;
    int val = (mVisible && parentVisible) ? 1 : 0;
    int result = screen_set_window_property_iv(mWindow, SCREEN_PROPERTY_VISIBLE, &val);
    if (result != 0) {
        qFatal("QBB: failed to set window visibility, errno=%d", errno);
    }

    QList<QBBWindow*>::iterator it;
    for (it = mChildren.begin(); it != mChildren.end(); it++) {
        (*it)->updateVisibility(mVisible && parentVisible);
    }
}

void QBBWindow::setOpacity(qreal level)
{
#if defined(QBBWINDOW_DEBUG)
    qDebug() << "QBBWindow::setOpacity - w=" << widget() << ", o=" << level;
#endif

    // set window global alpha
    errno = 0;
    int val = (int)(level * 255);
    int result = screen_set_window_property_iv(mWindow, SCREEN_PROPERTY_GLOBAL_ALPHA, &val);
    if (result != 0) {
        qFatal("QBB: failed to set window global alpha, errno=%d", errno);
    }

    // TODO: How to handle children of this window? If we change all the visibilities, then
    //       the transparency will look wrong...
}

void QBBWindow::setBufferSize(const QSize &size)
{
#if defined(QBBWINDOW_DEBUG)
    qDebug() << "QBBWindow::setBufferSize - w=" << widget() << ", s=" << size;
#endif

    // set window buffer size
    errno = 0;
    int val[2] = { size.width(), size.height() };
    int result = screen_set_window_property_iv(mWindow, SCREEN_PROPERTY_BUFFER_SIZE, val);
    if (result != 0) {
        qFatal("QBB: failed to set window buffer size, errno=%d", errno);
    }

    // create window buffers if they do not exist
    if (!hasBuffers()) {
#if defined(QBBWINDOW_DEBUG)
        qDebug() << "QBBWindow::setBufferSize - create buffers";
#endif
        errno = 0;
        result = screen_create_window_buffers(mWindow, MAX_BUFFER_COUNT);
        if (result != 0) {
            qFatal("QBB: failed to create window buffers, errno=%d", errno);
        }
    }

    // cache new buffer size
    mBufferSize = size;

    // buffers were destroyed; reacquire them
    mCurrentBufferIndex = -1;
    mPreviousDirty = QRegion();
    mScrolled = QRegion();
}

QBBBuffer &QBBWindow::renderBuffer()
{
#if defined(QBBWINDOW_DEBUG)
    qDebug() << "QBBWindow::renderBuffer - w=" << widget();
#endif

    // check if render buffer is invalid
    if (mCurrentBufferIndex == -1) {

        // get all buffers available for rendering
        errno = 0;
        screen_buffer_t buffers[MAX_BUFFER_COUNT];
        int result = screen_get_window_property_pv(mWindow, SCREEN_PROPERTY_RENDER_BUFFERS, (void **)buffers);
        if (result != 0) {
            qFatal("QBB: failed to query window buffers, errno=%d", errno);
        }

        // wrap each buffer
        for (int i = 0; i < MAX_BUFFER_COUNT; i++) {
            mBuffers[i] = QBBBuffer(buffers[i]);
        }

        // use the first available render buffer
        mCurrentBufferIndex = 0;
        mPreviousBufferIndex = -1;
    }

    return mBuffers[mCurrentBufferIndex];
}

void QBBWindow::scroll(const QRegion &region, int dx, int dy, bool flush)
{
    copyBack(region, dx, dy, flush);
    mScrolled += region;
}

void QBBWindow::post(const QRegion &dirty)
{
    // check if render buffer exists and something was rendered
    if (mCurrentBufferIndex != -1 && !dirty.isEmpty()) {

#if defined(QBBWINDOW_DEBUG)
        qDebug() << "QBBWindow::post - w=" << widget();
#endif
        QBBBuffer &currentBuffer = mBuffers[mCurrentBufferIndex];

        // copy unmodified region from old render buffer to new render buffer;
        // required to allow partial updates
        QRegion preserve = mPreviousDirty - dirty - mScrolled;
        copyBack(preserve, 0, 0);

        // calculate region that changed
        QRegion modified = preserve + dirty + mScrolled;
        QRect rect = modified.boundingRect();
        int dirtyRect[4] = { rect.x(), rect.y(), rect.x() + rect.width(), rect.y() + rect.height() };

        // update the display with contents of render buffer
        errno = 0;
        int result = screen_post_window(mWindow, currentBuffer.nativeBuffer(), 1, dirtyRect, 0);
        if (result != 0) {
            qFatal("QBB: failed to post window buffer, errno=%d", errno);
        }

        // advance to next nender buffer
        mPreviousBufferIndex = mCurrentBufferIndex++;
        if (mCurrentBufferIndex >= MAX_BUFFER_COUNT) {
            mCurrentBufferIndex = 0;
        }

        // save modified region and clear scrolled region
        mPreviousDirty = dirty;
        mScrolled = QRegion();
    }
}

QPlatformGLContext *QBBWindow::glContext() const
{
#if defined(QBBWINDOW_DEBUG)
    qDebug() << "QBBWindow::glContext - w=" << widget();
#endif
    return mPlatformGlContext;
}

void QBBWindow::setScreen(QBBScreen *platformScreen)
{
#if defined(QBBWINDOW_DEBUG)
    qDebug() << "QBBWindow::setScreen - w=" << widget();
#endif

    if (mScreen == platformScreen)
        return;

    mScreen = platformScreen;

    // move window to proper display
    errno = 0;
    screen_display_t display = platformScreen->nativeDisplay();
    int result = screen_set_window_property_pv(mWindow, SCREEN_PROPERTY_DISPLAY, (void **)&display);
    if (result != 0) {
        qFatal("QBB: failed to set window display, errno=%d", errno);
    }

    // add window to display's window group
    errno = 0;
    result = screen_join_window_group(mWindow, platformScreen->windowGroupName());
    if (result != 0) {
        qFatal("QBB: failed to join window group, errno=%d", errno);
    }

    QList<QBBWindow*>::iterator it;
    for (it = mChildren.begin(); it != mChildren.end(); it++) {
        // Only subwindows and tooltips need necessarily be moved to another display with
        // the window.
        if ((widget()->windowType() & Qt::WindowType_Mask) == Qt::SubWindow ||
            (widget()->windowType() & Qt::WindowType_Mask) == Qt::ToolTip)
            (*it)->setScreen(platformScreen);
    }

    QBBScreen::updateHierarchy();
}

void QBBWindow::removeFromParent()
{
    // Remove from old Hierarchy position
    if (mParent) {
        if (mParent->mChildren.removeOne(this))
            mParent = 0;
        else
            qFatal("QBB: Window Hierarchy broken; window has parent, but parent hasn't got child.");
    } else {
        QBBScreen::removeWindow(this);
    }
}

void QBBWindow::setParent(const QPlatformWindow *window)
{
#if defined(QBBWINDOW_DEBUG)
    qDebug() << "QBBWindow::setParent - w=" << widget() << " p=" << window;
#endif

    // Cast away the const, we need to modify the Hierarchy.
    QBBWindow* newParent = 0;

    if (window)
        newParent = static_cast<QBBWindow*>((QPlatformWindow *)window);

    if (newParent == mParent)
        return;

    removeFromParent();
    mParent = newParent;

    // Add to new Hierarchy position.
    if (mParent) {
        if (mParent->mScreen != mScreen)
            setScreen(mParent->mScreen);

        mParent->mChildren.push_back(this);
    } else {
        QBBScreen::addWindow(this);
    }

    QBBScreen::updateHierarchy();
}

void QBBWindow::raise()
{
#if defined(QBBWINDOW_DEBUG)
    qDebug() << "QBBWindow::raise - w=" << widget();
#endif

    QBBWindow* oldParent = mParent;
    if (oldParent) {
        removeFromParent();
        oldParent->mChildren.push_back(this);
    } else {
        QBBScreen::raiseWindow(this);
    }

    QBBScreen::updateHierarchy();
}

void QBBWindow::lower()
{
#if defined(QBBWINDOW_DEBUG)
    qDebug() << "QBBWindow::lower - w=" << widget();
#endif

    QBBWindow* oldParent = mParent;
    if (oldParent) {
        removeFromParent();
        oldParent->mChildren.push_front(this);
    } else {
        QBBScreen::lowerWindow(this);
    }

    QBBScreen::updateHierarchy();
}

void QBBWindow::requestActivateWindow()
{
#if defined(QBBWINDOW_DEBUG)
    qDebug() << "QBBWindow::requestActivateWindow - w=" << widget();
#endif

    // TODO: Tell screen to set keyboard focus to this window.

    // Notify that we gained focus.
    gainedFocus();
}

void QBBWindow::gainedFocus()
{
#if defined(QBBWINDOW_DEBUG)
    qDebug() << "QBBWindow::gainedFocus - w=" << widget();
#endif

    // Got focus
    QWindowSystemInterface::handleWindowActivated(widget());
}

void QBBWindow::updateZorder(int &topZorder)
{
    errno = 0;
    int result = screen_set_window_property_iv(mWindow, SCREEN_PROPERTY_ZORDER, &topZorder);
    topZorder++;

    if (result != 0)
        qFatal("QBB: failed to set window z-order=%d, errno=%d", topZorder, errno);

    QList<QBBWindow*>::const_iterator it;

    for (it = mChildren.begin(); it != mChildren.end(); it++)
        (*it)->updateZorder(topZorder);
}

void QBBWindow::copyBack(const QRegion &region, int dx, int dy, bool flush)
{
    int result;

    // abort if previous buffer is invalid
    if (mPreviousBufferIndex == -1) {
        return;
    }

    // abort if nothing to copy
    if (region.isEmpty()) {
        return;
    }

    QBBBuffer &currentBuffer = mBuffers[mCurrentBufferIndex];
    QBBBuffer &previousBuffer = mBuffers[mPreviousBufferIndex];

    // break down region into non-overlapping rectangles
    QVector<QRect> rects = region.rects();
    for (int i = rects.size() - 1; i >= 0; i--) {

        // clip rectangle to bounds of target
        QRect rect = rects[i].intersected( currentBuffer.rect() );

        // setup blit operation
        int attribs[] = { SCREEN_BLIT_SOURCE_X, rect.x(),
                          SCREEN_BLIT_SOURCE_Y, rect.y(),
                          SCREEN_BLIT_SOURCE_WIDTH, rect.width(),
                          SCREEN_BLIT_SOURCE_HEIGHT, rect.height(),
                          SCREEN_BLIT_DESTINATION_X, rect.x() + dx,
                          SCREEN_BLIT_DESTINATION_Y, rect.y() + dy,
                          SCREEN_BLIT_DESTINATION_WIDTH, rect.width(),
                          SCREEN_BLIT_DESTINATION_HEIGHT, rect.height(),
                          SCREEN_BLIT_END };

        // queue blit operation
        errno = 0;
        result = screen_blit(mContext, currentBuffer.nativeBuffer(), previousBuffer.nativeBuffer(), attribs);
        if (result != 0) {
            qFatal("QBB: failed to blit buffers, errno=%d", errno);
        }
    }

    // check if flush requested
    if (flush) {

        // wait for all blits to complete
        errno = 0;
        result = screen_flush_blits(mContext, SCREEN_WAIT_IDLE);
        if (result != 0) {
            qFatal("QBB: failed to flush blits, errno=%d", errno);
        }

        // buffer was modified outside the CPU
        currentBuffer.invalidateInCache();
    }
}

QT_END_NAMESPACE
