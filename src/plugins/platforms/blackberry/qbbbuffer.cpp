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

//#define QBBBUFFER_DEBUG


#include "qbbbuffer.h"

#include <QDebug>

#include <errno.h>
#include <sys/mman.h>

QT_BEGIN_NAMESPACE

QBBBuffer::QBBBuffer()
    : mBuffer(NULL)
{
#if defined(QBBBUFFER_DEBUG)
    qDebug() << "QBBBuffer::QBBBuffer - empty";
#endif
}

QBBBuffer::QBBBuffer(screen_buffer_t buffer)
    : mBuffer(buffer)
{
#if defined(QBBBUFFER_DEBUG)
    qDebug() << "QBBBuffer::QBBBuffer - normal";
#endif

    // get size of buffer
    errno = 0;
    int size[2];
    int result = screen_get_buffer_property_iv(buffer, SCREEN_PROPERTY_BUFFER_SIZE, size );
    if (result != 0) {
        qFatal("QBB: failed to query buffer size, errno=%d", errno);
    }

    // get stride of buffer
    errno = 0;
    int stride;
    result = screen_get_buffer_property_iv(buffer, SCREEN_PROPERTY_STRIDE, &stride );
    if (result != 0) {
        qFatal("QBB: failed to query buffer stride, errno=%d", errno);
    }

    // get access to buffer's data
    errno = 0;
    uchar* dataPtr;
    result = screen_get_buffer_property_pv(buffer, SCREEN_PROPERTY_POINTER, (void **)&dataPtr);
    if (result != 0) {
        qFatal("QBB: failed to query buffer pointer, errno=%d", errno);
    }

    // wrap buffer in an image
    mImage = QImage(dataPtr, size[0], size[1], stride, QImage::Format_ARGB32_Premultiplied);
}

QBBBuffer::QBBBuffer(const QBBBuffer &other)
    : mBuffer(other.mBuffer),
      mImage(other.mImage)
{
#if defined(QBBBUFFER_DEBUG)
    qDebug() << "QBBBuffer::QBBBuffer - copy";
#endif
}

QBBBuffer::~QBBBuffer()
{
#if defined(QBBBUFFER_DEBUG)
    qDebug() << "QBBBuffer::~QBBBuffer";
#endif
}

void QBBBuffer::invalidateInCache()
{
#if defined(QBBBUFFER_DEBUG)
    qDebug() << "QBBBuffer::invalidateInCache";
#endif

    // verify native buffer exists
    if (mBuffer == NULL) {
        qFatal("QBB: can't invalidate cache for null buffer");
    }

    // evict buffer's data from cache
    errno = 0;
    int result = msync(mImage.bits(), mImage.height() * mImage.bytesPerLine(), MS_INVALIDATE | MS_CACHE_ONLY);
    if (result != 0) {
        qFatal("QBB: failed to invalidate cache, errno=%d", errno);
    }
}

QT_END_NAMESPACE
