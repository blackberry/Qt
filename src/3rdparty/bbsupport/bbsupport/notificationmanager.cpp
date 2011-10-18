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

// #define NOTIFICATION_MANAGER_DEBUG

#define STRX(x) #x
#define STR(x) STRX(x)
#define TAG __FILE__ "(" STR(__LINE__) "): "

#include <notificationmanager.h>

#include <QDebug>

QT_BEGIN_NAMESPACE

namespace BlackBerry
{

NotificationManager& NotificationManager::instance()
{
    static NotificationManager instance;
    return instance;
}

NotificationManager::NotificationManager() :
    PPSFile(0, QString::fromLatin1("services/notification/control"))
{
#ifdef NOTIFICATION_MANAGER_DEBUG
    qDebug() << TAG "Created NotificationManager.";
#endif

    if (!open())
        qCritical(TAG "Could not open a connection to the notification manager - application may not function correctly.");
}

NotificationManager::~NotificationManager()
{
}

void NotificationManager::show(Notification const& notification)
{
#ifdef NOTIFICATION_MANAGER_DEBUG
    qDebug() << TAG "Showing notification " << notification.id();
#endif

    QVariantHash msg;
    msg[QString::fromLatin1("msg")] = QString::fromLatin1("Notify");
    msg[QString::fromLatin1("dat")] = notification.toHash();

    write(PPSObject(QString(), msg));

}

void NotificationManager::cancel(Notification const& notification)
{
#ifdef NOTIFICATION_MANAGER_DEBUG
    qDebug() << TAG "Cancelling notification " << notification.id();
#endif

    QVariantHash dat;
    dat[QString::fromLatin1("id")] = notification.id();

    QVariantHash msg;
    msg[QString::fromLatin1("msg")] = QString::fromLatin1("Cancel");
    msg[QString::fromLatin1("dat")] = dat;

    write(PPSObject(QString(), msg));
}

void NotificationManager::dataChanged(QByteArray &ppsData)
{
    QList<PPSObject> parsed = PPSObject::parse(ppsData);
    QList<PPSObject>::iterator it;
    for (it = parsed.begin(); it != parsed.end(); it++) {
        handlePPSMessage(*it);
    }
}

void NotificationManager::handlePPSMessage(PPSObject const& message)
{
#ifdef NOTIFICATION_MANAGER_DEBUG
    qDebug() << TAG "Handling " << message.attributes();
#endif

    if (message.objectStatus() != PPSObject::Undefined &&
        message.objectStatus() != PPSObject::Created) {
        // In all other cases the notification service is probably gone.
        qCritical() << TAG "The notification service has closed, notifications will no longer function.";
        return;
    }

    QVariantHash attributes = message.attributes().toHash();

    QString msg = attributes[QString::fromLatin1("msg")].toString();

    QVariantHash dat = attributes[QString::fromLatin1("dat")].toHash();
    QString id = dat[QString::fromLatin1("id")].toString();

    if (msg.compare(QString::fromLatin1("Acknowledge"), Qt::CaseInsensitive) == 0) {
        QString promptIndexStr = dat[QString::fromLatin1("promptResponse")].toString();
        int promptIndex = -1;
        if (!promptIndexStr.isEmpty()) {
            bool ok = false;
            promptIndex = promptIndexStr.toInt(&ok);
            if (!ok) {
                promptIndex = -1;
                qWarning() << TAG "promptResponse is not an integer.";
            }
        }
        emit notifyAcknowledge(id, promptIndex);
    }
    else if (msg.compare(QString::fromLatin1("Expire"), Qt::CaseInsensitive) == 0)
        emit notifyExpire(id);
    else if (msg.compare(QString::fromLatin1("Invoke"), Qt::CaseInsensitive) == 0)
        emit notifyInvoke(id);
    else
        qWarning() << TAG "Unsupported message type: " << msg;
}

void NotificationManager::disconnected()
{
#ifdef NOTIFICATION_MANAGER_DEBUG
    qDebug() << TAG "Disconnected from the notification manager service.";
#endif
}

} // namespace BlackBerry

QT_END_NAMESPACE

