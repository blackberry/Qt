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

//#define QBBGLCONTEXT_DEBUG


#include "qbbglcontext.h"
#include "qbbwindow.h"
#include "qbbintegration.h"
#include "../eglconvenience/qeglconvenience.h"

#include <QDebug>

QT_BEGIN_NAMESPACE

EGLDisplay QBBGLContext::sEglDisplay = EGL_NO_DISPLAY;

QBBGLContext::QBBGLContext(QBBWindow* platformWindow)
    : QPlatformGLContext(),
      mPlatformWindow(platformWindow),
      mEglSurface(EGL_NO_SURFACE)
{
#if defined(QBBGLCONTEXT_DEBUG)
    qDebug() << "QBBGLContext::QBBGLContext - w=" << mPlatformWindow->widget();
#endif

    // verify rendering API is correct
    QPlatformWindowFormat format = mPlatformWindow->widget()->platformWindowFormat();
    if (format.windowApi() != QPlatformWindowFormat::OpenGL) {
        qFatal("QBB: window API is not OpenGL");
    }

    // choose EGL settings based on OpenGL version
    EGLint renderableType = EGL_OPENGL_ES2_BIT;
    EGLint contextAttrs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };

    // set current rendering API
    EGLBoolean eglResult = eglBindAPI(EGL_OPENGL_ES_API);
    if (eglResult != EGL_TRUE) {
        qFatal("QBB: failed to set EGL API, err=%d", eglGetError());
    }

    // get depth buffer size from window format
    EGLint depthSize = 0;
    if (format.depth()) {
        depthSize = format.depthBufferSize();
        if (depthSize == -1) {
            // don't care so ask for >= 1-bit
            depthSize = 1;
        }
    }

    // get stencil buffer size from window format
    EGLint stencilSize = 0;
    if (format.stencil()) {
        stencilSize = format.stencilBufferSize();
        if (stencilSize == -1) {
            // don't care so ask for >= 1-bit
            stencilSize = 1;
        }
    }

    // get number of multisample buffers and samples per pixel
    // from window format
    EGLint sampleBuffers = 0;
    EGLint samples = 0;
    if (format.sampleBuffers()) {
        sampleBuffers = 1;
        samples = format.samples();
        if (samples == -1) {
            // don't care so ask for >= 1 sample
            samples = 1;
        }
    }

    // get EGL config compatible with window
    EGLint numConfig;
    EGLint configAttrs[] = { EGL_BUFFER_SIZE,       32,
                             EGL_ALPHA_SIZE,        8,
                             EGL_RED_SIZE,          8,
                             EGL_GREEN_SIZE,        8,
                             EGL_BLUE_SIZE,         8,
                             EGL_DEPTH_SIZE,        depthSize,
                             EGL_STENCIL_SIZE,      stencilSize,
                             EGL_SAMPLE_BUFFERS,    sampleBuffers,
                             EGL_SAMPLES,           samples,
                             EGL_SURFACE_TYPE,      EGL_WINDOW_BIT,
                             EGL_RENDERABLE_TYPE,   renderableType,
                             EGL_NONE };
    eglResult = eglChooseConfig(sEglDisplay, configAttrs, &mEglConfig, 1, &numConfig);
    if (eglResult != EGL_TRUE || numConfig == 0) {
        qFatal("QBB: failed to find EGL config, err=%d, numConfig=%d", eglGetError(), numConfig);
    }

    // create EGL context
    mEglContext = eglCreateContext(sEglDisplay, mEglConfig, EGL_NO_CONTEXT, contextAttrs);
    if (mEglContext == EGL_NO_CONTEXT) {
        qFatal("QBB: failed to create EGL context, err=%d", eglGetError());
    }

    // query/cache window format
    mWindowFormat = qt_qPlatformWindowFormatFromConfig(sEglDisplay, mEglConfig);

    // NOTE: create a dummy EGL surface since Qt will call makeCurrent() before
    // setting the geometry on the associated widget
    mSurfaceSize = QSize(1, 1);
    mPlatformWindow->setBufferSize(mSurfaceSize);
    createSurface();
}

QBBGLContext::~QBBGLContext()
{
#if defined(QBBGLCONTEXT_DEBUG)
    qDebug() << "QBBGLContext::~QBBGLContext - w=" << mPlatformWindow->widget();
#endif

    // cleanup EGL context if it exists
    if (mEglContext != EGL_NO_CONTEXT) {
        eglDestroyContext(sEglDisplay, mEglContext);
    }

    // cleanup EGL surface if it exists
    destroySurface();
}

void QBBGLContext::initialize()
{
    // initialize connection to EGL
    sEglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (sEglDisplay == EGL_NO_DISPLAY) {
        qFatal("QBB: failed to obtain EGL display");
    }

    EGLBoolean eglResult = eglInitialize(sEglDisplay, NULL, NULL);
    if (eglResult != EGL_TRUE) {
        qFatal("QBB: failed to initialize EGL display, err=%d", eglGetError());
    }
}

void QBBGLContext::shutdown()
{
    // close connection to EGL
    eglTerminate(sEglDisplay);
}

void QBBGLContext::makeCurrent()
{
#if defined(QBBGLCONTEXT_DEBUG)
    qDebug() << "QBBGLContext::makeCurrent - w=" << mPlatformWindow->widget();
#endif

    // call the parent method
    QPlatformGLContext::makeCurrent();

    // set current rendering API
    EGLBoolean eglResult = eglBindAPI(EGL_OPENGL_ES_API);
    if (eglResult != EGL_TRUE) {
        qFatal("QBB: failed to set EGL API, err=%d", eglGetError());
    }

    // set current EGL context and bind with EGL surface
    eglResult = eglMakeCurrent(sEglDisplay, mEglSurface, mEglSurface, mEglContext);
    if (eglResult != EGL_TRUE) {
        qFatal("QBB: failed to set current EGL context, err=%d", eglGetError());
    }
}

void QBBGLContext::doneCurrent()
{
#if defined(QBBGLCONTEXT_DEBUG)
    qDebug() << "QBBGLContext::doneCurrent - w=" << mPlatformWindow->widget();
#endif

    // call the parent method
    QPlatformGLContext::doneCurrent();

    // set current rendering API
    EGLBoolean eglResult = eglBindAPI(EGL_OPENGL_ES_API);
    if (eglResult != EGL_TRUE) {
        qFatal("QBB: failed to set EGL API, err=%d", eglGetError());
    }

    // clear curent EGL context and unbind EGL surface
    eglResult = eglMakeCurrent(sEglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    if (eglResult != EGL_TRUE) {
        qFatal("QBB: failed to clear current EGL context, err=%d", eglGetError());
    }
}

void QBBGLContext::swapBuffers()
{
#if defined(QBBGLCONTEXT_DEBUG)
    qDebug() << "QBBGLContext::swapBuffers - w=" << mPlatformWindow->widget();
#endif

    // set current rendering API
    EGLBoolean eglResult = eglBindAPI(EGL_OPENGL_ES_API);
    if (eglResult != EGL_TRUE) {
        qFatal("QBB: failed to set EGL API, err=%d", eglGetError());
    }

    // post EGL surface to window
    eglResult = eglSwapBuffers(sEglDisplay, mEglSurface);
    if (eglResult != EGL_TRUE) {
        qFatal("QBB: failed to swap EGL buffers, err=%d", eglGetError());
    }

    // check if window was resized
    QSize s = mPlatformWindow->geometry().size();
    if (s != mSurfaceSize) {

        // need to destroy surface so make it not current
        doneCurrent();

        // destroy old EGL surface
        destroySurface();

        // resize window's buffers
        mPlatformWindow->setBufferSize(s);

        // re-create EGL surface with new size
        mSurfaceSize = s;
        createSurface();

        // make context current again
        makeCurrent();
    }
}

void* QBBGLContext::getProcAddress(const QString& procName)
{
#if defined(QBBGLCONTEXT_DEBUG)
    qDebug() << "QBBGLContext::getProcAddress - w=" << mPlatformWindow->widget();
#endif

    // set current rendering API
    EGLBoolean eglResult = eglBindAPI(EGL_OPENGL_ES_API);
    if (eglResult != EGL_TRUE) {
        qFatal("QBB: failed to set EGL API, err=%d", eglGetError());
    }

    // lookup EGL extension function pointer
    return (void *)eglGetProcAddress( procName.toAscii().constData() );
}

void QBBGLContext::createSurface()
{
#if defined(QBBGLCONTEXT_DEBUG)
    qDebug() << "QBBGLContext::createSurface - w=" << mPlatformWindow->widget();
#endif

    // create EGL surface
    mEglSurface = eglCreateWindowSurface(sEglDisplay, mEglConfig, (EGLNativeWindowType)mPlatformWindow->winId(), NULL);
    if (mEglSurface == EGL_NO_SURFACE) {
        qFatal("QBB: failed to create EGL surface, err=%d", eglGetError());
    }
}

void QBBGLContext::destroySurface()
{
#if defined(QBBGLCONTEXT_DEBUG)
    qDebug() << "QBBGLContext::destroySurface - w=" << mPlatformWindow->widget();
#endif

    // destroy EGL surface if it exists
    if (mEglSurface != EGL_NO_SURFACE) {
        EGLBoolean eglResult = eglDestroySurface(sEglDisplay, mEglSurface);
        if (eglResult != EGL_TRUE) {
            qFatal("QBB: failed to destroy EGL surface, err=%d", eglGetError());
        }
    }
}

QT_END_NAMESPACE
