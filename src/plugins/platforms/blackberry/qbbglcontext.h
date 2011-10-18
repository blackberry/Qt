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

#ifndef QBBGLCONTEXT_H
#define QBBGLCONTEXT_H

#include <QtGui/QPlatformGLContext>
#include <QtGui/QPlatformWindowFormat>
#include <QSize>

#include <EGL/egl.h>

QT_BEGIN_NAMESPACE

class QBBWindow;

class QBBGLContext : public QPlatformGLContext
{
public:
    QBBGLContext(QBBWindow* platformWindow);
    virtual ~QBBGLContext();

    static void initialize();
    static void shutdown();

    virtual void makeCurrent();
    virtual void doneCurrent();
    virtual void swapBuffers();
    virtual void* getProcAddress(const QString& procName);

    virtual QPlatformWindowFormat platformWindowFormat() const { return mWindowFormat; }

private:
    static EGLDisplay sEglDisplay;

    QBBWindow *mPlatformWindow;
    EGLConfig mEglConfig;
    EGLContext mEglContext;
    EGLSurface mEglSurface;
    QSize mSurfaceSize;

    QPlatformWindowFormat mWindowFormat;

    void createSurface();
    void destroySurface();
};

QT_END_NAMESPACE

#endif // QBBGLCONTEXT_H
