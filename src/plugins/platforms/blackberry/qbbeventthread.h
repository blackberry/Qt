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

#ifndef QBBEVENTTHREAD_H
#define QBBEVENTTHREAD_H

#include <QtGui/QPlatformScreen>
#include <QtGui/QWindowSystemInterface>
#include <QThread>

#include <screen/screen.h>

QT_BEGIN_NAMESPACE

// an arbitrary number
#define MAX_TOUCH_POINTS    10

class QBBEventThread : public QThread
{
public:
    QBBEventThread(screen_context_t context, QPlatformScreen& screen);
    virtual ~QBBEventThread();

    static void injectKeyboardEvent(int flags, int sym, int mod, int scan, int cap);
    void injectPointerMoveEvent(int x, int y);

protected:
    virtual void run();

private:
    screen_context_t mContext;
    QPlatformScreen& mScreen;
    bool mQuit;
    QPoint mLastGlobalMousePoint;
    QPoint mLastLocalMousePoint;
    Qt::MouseButtons mLastButtonState;
    void* mLastMouseWindow;

    QWindowSystemInterface::TouchPoint mTouchPoints[MAX_TOUCH_POINTS];

    void shutdown();
    void dispatchEvent(screen_event_t event);
    void handleKeyboardEvent(screen_event_t event);
    void handlePointerEvent(screen_event_t event);
    void handleTouchEvent(screen_event_t event, int type);
    void handleCloseEvent(screen_event_t event);
};

QT_END_NAMESPACE

#endif // QBBEVENTTHREAD_H
