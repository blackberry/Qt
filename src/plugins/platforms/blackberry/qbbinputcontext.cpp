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

// #define QBBINPUTCONTEXT_DEBUG

#include <qbbinputcontext.h>

#include <QDebug>
#include <bbsupport/Keyboard>

QBBInputContext::QBBInputContext(QObject* parent):
         QInputContext(parent)
{
}

QBBInputContext::~QBBInputContext()
{
}

QString QBBInputContext::language()
{
    // Once we enable full IMF support, we need to hook that up here.
    return BlackBerry::Keyboard::instance().languageId();
}

bool QBBInputContext::hasPhysicalKeyboard()
{
    // TODO: This should query the system to check if a USB keyboard is connected.
    return false;
}

void QBBInputContext::reset()
{
}

bool QBBInputContext::filterEvent( const QEvent *event )
{
    if(hasPhysicalKeyboard())
        return false;

    if (event->type() == QEvent::CloseSoftwareInputPanel) {
        BlackBerry::Keyboard::instance().hide();
#if defined(QBBINPUTCONTEXT_DEBUG)
        qDebug() << "QBB: hiding virtual keyboard";
#endif
        return false;
    }

    if (event->type() == QEvent::RequestSoftwareInputPanel) {
        BlackBerry::Keyboard::instance().show();
#if defined(QBBINPUTCONTEXT_DEBUG)
        qDebug() << "QBB: requesting virtual keyboard";
#endif
        return false;
    }

    return false;

}

