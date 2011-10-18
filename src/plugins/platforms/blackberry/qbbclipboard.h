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

#ifndef QBBCLIPBOARD_H
#define QBBCLIPBOARD_H

#ifndef QT_NO_CLIPBOARD
#include <QtGui/QPlatformClipboard>
#include <QMimeData>

QT_BEGIN_NAMESPACE

class QBBClipboard : public QPlatformClipboard
{
public:
    QBBClipboard();
    virtual ~QBBClipboard();
    virtual QMimeData *mimeData(QClipboard::Mode mode = QClipboard::Clipboard);
    virtual void setMimeData(QMimeData *data, QClipboard::Mode mode = QClipboard::Clipboard);
private:
    QMimeData *mMimeData;
    void readClipboardBuff(const char *type);
};

QT_END_NAMESPACE

#endif //QT_NO_CLIPBOAR
#endif //QBBCLIPBOARD_H
