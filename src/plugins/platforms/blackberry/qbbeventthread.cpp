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

//#define QBBEVENTTHREAD_DEBUG

#include "qbbeventthread.h"
#include "qbbkeytranslator.h"

#include <QtGui/QWidget>
#include <QtGui/QPlatformScreen>
#include <QtGui/QApplication>
#include <QDebug>

#include <errno.h>
#include <unistd.h>
#include <sys/keycodes.h>

#include <cctype>

QBBEventThread::QBBEventThread(screen_context_t context, QPlatformScreen& screen)
    : mContext(context),
      mScreen(screen),
      mQuit(false),
      mLastButtonState(Qt::NoButton),
      mLastMouseWindow(0)
{
    // initialize array of touch points
    for (int i = 0; i < MAX_TOUCH_POINTS; i++) {

        // map array index to id
        mTouchPoints[i].id = i;

        // first finger is primary
        mTouchPoints[i].isPrimary = (i == 0);

        // pressure is not supported - use default
        mTouchPoints[i].pressure = 1.0;

        // nothing touching
        mTouchPoints[i].state = Qt::TouchPointReleased;
    }
}

QBBEventThread::~QBBEventThread()
{
    // block until thread terminates
    shutdown();
}

void QBBEventThread::run()
{
    screen_event_t event;

    // create screen event
    errno = 0;
    int result = screen_create_event(&event);
    if (result) {
        qFatal("QBB: failed to create event, errno=%d", errno);
    }

#if defined(QBBEVENTTHREAD_DEBUG)
    qDebug() << "QBB: event loop started";
#endif

    // loop indefinitely
    while (!mQuit) {

        // block until screen event is available
        errno = 0;
        result = screen_get_event(mContext, event, -1);
        if (result) {
            qFatal("QBB: failed to get event, errno=%d", errno);
        }

        // process received event
        dispatchEvent(event);
    }

#if defined(QBBEVENTTHREAD_DEBUG)
    qDebug() << "QBB: event loop stopped";
#endif

    // cleanup
    screen_destroy_event(event);
}

void QBBEventThread::shutdown()
{
    screen_event_t event;

    // create screen event
    errno = 0;
    int result = screen_create_event(&event);
    if (result) {
        qFatal("QBB: failed to create event, errno=%d", errno);
    }

    // set the event type as user
    errno = 0;
    int type = SCREEN_EVENT_USER;
    result = screen_set_event_property_iv(event, SCREEN_PROPERTY_TYPE, &type);
    if (result) {
        qFatal("QBB: failed to set event type, errno=%d", errno);
    }

    // NOTE: ignore SCREEN_PROPERTY_USER_DATA; treat all user events as shutdown events

    // post event to event loop so it will wake up and die
    errno = 0;
    result = screen_send_event(mContext, event, getpid());
    if (result) {
        qFatal("QBB: failed to set event type, errno=%d", errno);
    }

    // cleanup
    screen_destroy_event(event);

#if defined(QBBEVENTTHREAD_DEBUG)
    qDebug() << "QBB: event loop shutdown begin";
#endif

    // block until thread terminates
    wait();

#if defined(QBBEVENTTHREAD_DEBUG)
    qDebug() << "QBB: event loop shutdown end";
#endif
}

void QBBEventThread::dispatchEvent(screen_event_t event)
{
    // get the event type
    errno = 0;
    int qnxType;
    int result = screen_get_event_property_iv(event, SCREEN_PROPERTY_TYPE, &qnxType);
    if (result) {
        qFatal("QBB: failed to query event type, errno=%d", errno);
    }

    switch (qnxType) {
    case SCREEN_EVENT_MTOUCH_TOUCH:
    case SCREEN_EVENT_MTOUCH_MOVE:
    case SCREEN_EVENT_MTOUCH_RELEASE:
        handleTouchEvent(event, qnxType);
        break;

    case SCREEN_EVENT_KEYBOARD:
        handleKeyboardEvent(event);
        break;

    case SCREEN_EVENT_POINTER:
        handlePointerEvent(event);
        break;

    case SCREEN_EVENT_CLOSE:
        handleCloseEvent(event);
        break;

    case SCREEN_EVENT_USER:
        // treat all user events as shutdown requests
#if defined(QBBEVENTTHREAD_DEBUG)
        qDebug() << "QBB: QNX user event";
#endif
        mQuit = true;
        break;

    default:
        // event ignored
#if defined(QBBEVENTTHREAD_DEBUG)
        qDebug() << "QBB: QNX unknown event";
#endif
        break;
    }
}

void QBBEventThread::handleKeyboardEvent(screen_event_t event)
{
    // get flags of key event
    errno = 0;
    int flags;
    int result = screen_get_event_property_iv(event, SCREEN_PROPERTY_KEY_FLAGS, &flags);
    if (result) {
        qFatal("QBB: failed to query event flags, errno=%d", errno);
    }

    // get key code
    errno = 0;
    int sym;
    result = screen_get_event_property_iv(event, SCREEN_PROPERTY_KEY_SYM, &sym);
    if (result) {
        qFatal("QBB: failed to query event sym, errno=%d", errno);
    }

    int modifiers;
    result = screen_get_event_property_iv(event, SCREEN_PROPERTY_KEY_MODIFIERS, &modifiers);
    if (result) {
        qFatal("QBB: failed to query event modifiers, errno=%d", errno);
    }

    int scan;
    result = screen_get_event_property_iv(event, SCREEN_PROPERTY_KEY_SCAN, &scan);
    if (result) {
        qFatal("QBB: failed to query event modifiers, errno=%d", errno);
    }

    int cap;
    result = screen_get_event_property_iv(event, SCREEN_PROPERTY_KEY_CAP, &cap);
    if (result) {
        qFatal("QBB: failed to query event cap, errno=%d", errno);
    }

    Qt::KeyboardModifiers qtMod = Qt::NoModifier;
    if (modifiers & KEYMOD_SHIFT)
        qtMod |= Qt::ShiftModifier;
    if (modifiers & KEYMOD_CTRL)
        qtMod |= Qt::ControlModifier;
    if (modifiers & KEYMOD_ALT)
        qtMod |= Qt::AltModifier;

    // determine event type
    QEvent::Type type = (flags & KEY_DOWN) ? QEvent::KeyPress : QEvent::KeyRelease;

    // Check if the key cap is valid
    if (flags & KEY_CAP_VALID) {
        Qt::Key key;
        QChar keyStr;

        if (cap >= 0x20 && cap <= 0x0ff) { // 8 bit printable Latin1
            key = Qt::Key(std::toupper(cap));   // Qt expects the CAP to be upper case.
            if (flags & KEY_SYM_VALID) {
                keyStr = QChar(sym);
            }
        } else {
            if (isKeypadKey(cap))
                qtMod |= Qt::KeypadModifier; // Is this right?
            key = keyTranslator(cap);

            // Some special keys do have a string
            switch( key ) {
                case Qt::Key_Backspace:
                    keyStr = QChar('\b');
                    break;
                case Qt::Key_Return:
                    keyStr = QChar('\n');
                    break;
                default:
                    // No special charcode
                    break;
            }
        }

        QWindowSystemInterface::handleKeyEvent(QApplication::activeWindow(), type, key, qtMod, keyStr);
#if defined(QBBEVENTTHREAD_DEBUG)
        qDebug() << "QBB: Qt key t=" << type << ", k=" << key << ", s=" << keyStr;
#endif
    }
}

void QBBEventThread::handlePointerEvent(screen_event_t event)
{
    errno = 0;

    // Query the window that was clicked
    void *qnxWindow;
    int result = screen_get_event_property_pv(event, SCREEN_PROPERTY_WINDOW, &qnxWindow);
    if (result) {
        qFatal("QBB: failed to query event window, errno=%d", errno);
    }

    // Query the button states
    int buttonState = 0;
    result = screen_get_event_property_iv(event, SCREEN_PROPERTY_BUTTONS, &buttonState);
    if (result) {
        qFatal("QBB: failed to query event button state, errno=%d", errno);
    }

    // Query the window position
    int windowPos[2];
    result = screen_get_event_property_iv(event, SCREEN_PROPERTY_SOURCE_POSITION, windowPos);
    if (result) {
        qFatal("QBB: failed to query event window position, errno=%d", errno);
    }

    // Query the screen position
    int pos[2];
    result = screen_get_event_property_iv(event, SCREEN_PROPERTY_POSITION, pos);
    if (result) {
        qFatal("QBB: failed to query event position, errno=%d", errno);
    }

    // Query the wheel delta
    int wheelDelta = 0;
    result = screen_get_event_property_iv(event, SCREEN_PROPERTY_MOUSE_WHEEL, &wheelDelta);
    if (result) {
        qFatal("QBB: failed to query event wheel delta, errno=%d", errno);
    }

    // map window to top-level widget
    QWidget* w = QWidget::find( (WId)qnxWindow );

    // Generate enter and leave events as needed.
    if (qnxWindow != mLastMouseWindow) {
        QWidget* wOld = QWidget::find( (WId)mLastMouseWindow );

        if (wOld) {
            QWindowSystemInterface::handleLeaveEvent(wOld);
#if defined(QBBEVENTTHREAD_DEBUG)
            qDebug() << "QBB: Qt leave, w=" << wOld;
#endif
        }

        if (w) {
            QWindowSystemInterface::handleEnterEvent(w);
#if defined(QBBEVENTTHREAD_DEBUG)
            qDebug() << "QBB: Qt enter, w=" << w;
#endif
        }
    }

    // Apply scaling to wheel delta and invert value for Qt. We'll probably want to scale
    // this via a system preference at some point. But for now this is a sane value and makes
    // the wheel usable.
    wheelDelta *= -10;

    // convert point to local coordinates
    QPoint globalPoint(pos[0], pos[1]);
    QPoint localPoint(windowPos[0], windowPos[1]);

    // Convert buttons.
    Qt::MouseButtons buttons = Qt::NoButton;
    if (buttonState & 1)
        buttons |= Qt::LeftButton;
    if (buttonState & 2)
        buttons |= Qt::MidButton;
    if (buttonState & 4)
        buttons |= Qt::RightButton;

    if (w) {
        // Inject mouse event into Qt only if something has changed.
        if (mLastGlobalMousePoint != globalPoint ||
            mLastLocalMousePoint != localPoint ||
            mLastButtonState != buttons) {
            QWindowSystemInterface::handleMouseEvent(w, localPoint, globalPoint, buttons);
#if defined(QBBEVENTTHREAD_DEBUG)
            qDebug() << "QBB: Qt mouse, w=" << w << ", (" << localPoint.x() << "," << localPoint.y() << "), b=" << (int)buttons;
#endif
        }

        if (wheelDelta) {
            // Screen only supports a single wheel, so we will assume Vertical orientation for
            // now since that is pretty much standard.
            QWindowSystemInterface::handleWheelEvent(w, localPoint, globalPoint, wheelDelta, Qt::Vertical);
#if defined(QBBEVENTTHREAD_DEBUG)
            qDebug() << "QBB: Qt wheel, w=" << w << ", (" << localPoint.x() << "," << localPoint.y() << "), d=" << (int)wheelDelta;
#endif
        }
    }

    mLastGlobalMousePoint = globalPoint;
    mLastLocalMousePoint = localPoint;
    mLastButtonState = buttons;
}

void QBBEventThread::handleTouchEvent(screen_event_t event, int qnxType)
{
    // get display coordinates of touch
    errno = 0;
    int pos[2];
    int result = screen_get_event_property_iv(event, SCREEN_PROPERTY_POSITION, pos);
    if (result) {
        qFatal("QBB: failed to query event position, errno=%d", errno);
    }

    // get window coordinates of touch
    errno = 0;
    int windowPos[2];
    result = screen_get_event_property_iv(event, SCREEN_PROPERTY_SOURCE_POSITION, windowPos);
    if (result) {
        qFatal("QBB: failed to query event window position, errno=%d", errno);
    }

    // determine which finger touched
    errno = 0;
    int touchId;
    result = screen_get_event_property_iv(event, SCREEN_PROPERTY_TOUCH_ID, &touchId);
    if (result) {
        qFatal("QBB: failed to query event touch id, errno=%d", errno);
    }

    // determine which window was touched
    errno = 0;
    void *qnxWindow;
    result = screen_get_event_property_pv(event, SCREEN_PROPERTY_WINDOW, &qnxWindow);
    if (result) {
        qFatal("QBB: failed to query event window, errno=%d", errno);
    }

    // check if finger is valid
    if (touchId < MAX_TOUCH_POINTS) {

        // map window to top-level widget
        QWidget* w = QWidget::find( (WId)qnxWindow );

        // Generate enter and leave events as needed.
        if (qnxWindow != mLastMouseWindow) {
            QWidget* wOld = QWidget::find( (WId)mLastMouseWindow );

            if (wOld) {
                QWindowSystemInterface::handleLeaveEvent(wOld);
    #if defined(QBBEVENTTHREAD_DEBUG)
                qDebug() << "QBB: Qt leave, w=" << wOld;
    #endif
            }

            if (w) {
                QWindowSystemInterface::handleEnterEvent(w);
    #if defined(QBBEVENTTHREAD_DEBUG)
                qDebug() << "QBB: Qt enter, w=" << w;
    #endif
            }
        }

        if (w) {
            // convert primary touch to mouse event
            if (touchId == 0) {

                // convert point to local coordinates
                QPoint globalPoint(pos[0], pos[1]);
                QPoint localPoint(windowPos[0], windowPos[1]);

                // map touch state to button state
                Qt::MouseButtons buttons = (qnxType == SCREEN_EVENT_MTOUCH_RELEASE) ? Qt::NoButton : Qt::LeftButton;

                // inject event into Qt
                QWindowSystemInterface::handleMouseEvent(w, localPoint, globalPoint, buttons);
#if defined(QBBEVENTTHREAD_DEBUG)
                qDebug() << "QBB: Qt mouse, w=" << w << ", (" << localPoint.x() << "," << localPoint.y() << "), b=" << buttons;
#endif
            }

            // get size of screen which contains window
            QPlatformScreen* platformScreen = QPlatformScreen::platformScreenForWidget(w);
            QSize screenSize = platformScreen->physicalSize();

            // update cached position of current touch point
            mTouchPoints[touchId].normalPosition = QPointF( ((qreal)pos[0]) / screenSize.width(), ((qreal)pos[1]) / screenSize.height() );
            mTouchPoints[touchId].area = QRectF( pos[0], pos[1], 0.0, 0.0 );

            // determine event type and update state of current touch point
            QEvent::Type type = QEvent::None;
            switch (qnxType) {
            case SCREEN_EVENT_MTOUCH_TOUCH:
                mTouchPoints[touchId].state = Qt::TouchPointPressed;
                type = QEvent::TouchBegin;
                break;
            case SCREEN_EVENT_MTOUCH_MOVE:
                mTouchPoints[touchId].state = Qt::TouchPointMoved;
                type = QEvent::TouchUpdate;
                break;
            case SCREEN_EVENT_MTOUCH_RELEASE:
                mTouchPoints[touchId].state = Qt::TouchPointReleased;
                type = QEvent::TouchEnd;
                break;
            }

            // build list of active touch points
            QList<QWindowSystemInterface::TouchPoint> pointList;
            for (int i = 0; i < MAX_TOUCH_POINTS; i++) {
                if (i == touchId) {
                    // current touch point is always active
                    pointList.append(mTouchPoints[i]);
                } else if (mTouchPoints[i].state != Qt::TouchPointReleased) {
                    // finger is down but did not move
                    mTouchPoints[i].state = Qt::TouchPointStationary;
                    pointList.append(mTouchPoints[i]);
                }
            }

            // inject event into Qt
            QWindowSystemInterface::handleTouchEvent(w, type, QTouchEvent::TouchScreen, pointList);
#if defined(QBBEVENTTHREAD_DEBUG)
            qDebug() << "QBB: Qt touch, w=" << w << ", p=(" << pos[0] << "," << pos[1] << "), t=" << type;
#endif
        }
    }
}

void QBBEventThread::handleCloseEvent(screen_event_t event)
{
    // Query the window that was closed
    void *qnxWindow;
    int result = screen_get_event_property_pv(event, SCREEN_PROPERTY_WINDOW, &qnxWindow);
    if (result != 0) {
        qFatal("QBB: failed to query event window, errno=%d", errno);
    }

    // map window to top-level widget
    QWidget* w = QWidget::find( (WId)qnxWindow );
    if (w != NULL) {
        QWindowSystemInterface::handleCloseEvent(w);
    }
}

