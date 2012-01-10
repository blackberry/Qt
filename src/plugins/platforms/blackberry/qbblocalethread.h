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

#ifndef QBBLOCALETHREAD_H
#define QBBLOCALETHREAD_H

#include <QThread>

QT_BEGIN_NAMESPACE

class QBBLocaleThread : public QThread
{
public:
    QBBLocaleThread();
    virtual ~QBBLocaleThread();

protected:
    virtual void run();

private:
    int mFd;
    bool mQuit;

    void shutdown();
};

QT_END_NAMESPACE

#endif // QBBLOCALETHREAD_H
