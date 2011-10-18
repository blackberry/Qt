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

#ifndef NOTIFICATION_MANAGER_H
#define NOTIFICATION_MANAGER_H

#include <bbsupport/Navigator>
#include <bbsupport/Notification>
#include <bbsupport/PPSObject>
#include <bbsupport/PPSFile>

#include <QFileInfo>
#include <QObject>
#include <QString>

QT_BEGIN_NAMESPACE

namespace BlackBerry
{

class NotificationManager : public PPSFile
{
    Q_OBJECT

public:
    static NotificationManager& instance();

    /**
     * Show or update the notification
     */
    void show(Notification const& notification);

    /**
     * Cancel the notification. 
     */
    void cancel(Notification const& notification);

signals:
    void notifyAcknowledge(QString id, int promptIndex);
    void notifyExpire(QString id);
    void notifyInvoke(QString id);

private:
    NotificationManager();
    virtual ~NotificationManager();

    virtual void dataChanged(QByteArray &ppsData);
    void handlePPSMessage(PPSObject const& message);
    virtual void disconnected();

    NotificationManager(NotificationManager const&);   // Don't implement
    void operator=(NotificationManager const&);        // Don't implement
};

} //namespace BlackBerry

QT_END_NAMESPACE

#endif // NOTIFICATION_MANAGER_H

