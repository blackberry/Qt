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

#ifndef QBBBUFFER_H
#define QBBBUFFER_H

#include <QImage>

#include <screen/screen.h>

QT_BEGIN_NAMESPACE

class QBBBuffer
{
public:
    QBBBuffer();
    QBBBuffer(screen_buffer_t buffer);
    QBBBuffer(const QBBBuffer &other);
    virtual ~QBBBuffer();

    screen_buffer_t nativeBuffer() const { return mBuffer; }
    const QImage *image() const { return (mBuffer != NULL) ? &mImage : NULL; }
    QImage *image() { return (mBuffer != NULL) ? &mImage : NULL; }

    QRect rect() const { return mImage.rect(); }

    void invalidateInCache();

private:
    screen_buffer_t mBuffer;
    QImage mImage;
};

QT_END_NAMESPACE

#endif // QBBBUFFER_H
