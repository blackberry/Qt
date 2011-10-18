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

#ifndef PPSTHREAD_H
#define PPSTHREAD_H

#include <QThread>
#include <QHash>
#include <QString>
#include "ppsobject.h"

QT_BEGIN_NAMESPACE

namespace BlackBerry
{
/**
 * The PPSThread singleton encapsulates a single connection to the PPS .notify object and
 * is shared by all PPSFile objects.
 */
class PPSThread : public QThread
{
    Q_OBJECT

friend class PPSFile;

protected:
    static PPSThread& instance();
    QString   groupId() const { return mGroupId; }
    QString   createUniqueId();
    virtual void run();

signals:
    void notifyObjectChanged(QString objectUniqueId);
    void notifyObjectDisconnected(QString objectUniqueId);

private:
    PPSThread();
    virtual ~PPSThread();
    PPSThread(PPSThread const&);   // Don't implement
    void operator=(PPSThread const&); // Don't implement

    QString mGroupId;
    unsigned int mUniqueId;
    int mNotifyFd;
};

} // namespace BlackBerry

QT_END_NAMESPACE


#endif //PPSTHREAD_H


