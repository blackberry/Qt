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

#ifndef QBBINPUTCONTEXT_H
#define QBBINPUTCONTEXT_H

#include <QtGui/QPlatformIntegration>
#include <qinputcontext.h>

QT_BEGIN_NAMESPACE

class QBBInputContext : public QInputContext
{
public:
    explicit QBBInputContext(QObject* parent = 0);
    ~QBBInputContext();
    QString identifierName() { return QString("BlackBerry IMF"); }
    QString language();

    void reset();
    bool isComposing() const { return false; }
    virtual bool filterEvent( const QEvent *event );
private:
    bool hasPhysicalKeyboard();

};

QT_END_NAMESPACE

#endif // QBBINPUTCONTEXT_H
