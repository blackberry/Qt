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

// #define PPSTHREAD_DEBUG

#include "ppsthread.h"
#include "ppsfile.h"

#include <QtGui/QApplication>
#include <QList>
#include <QDebug>

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

QT_BEGIN_NAMESPACE

namespace BlackBerry
{

inline QDebug qFatalSs() { return QDebug(QtFatalMsg); }

const QString NotifyObjectName = QString::fromAscii(".notify");

// See qnx pps documentation (+2 for null & initial space)
const int GroupIdLength = 18;

// Only needs to be long enough for id+value, which should never be longer than 32 bytes, but
// we'll use 512 to be safe.
const int PpsMaxBuffer = 512;

PPSThread& PPSThread::instance()
{
    static PPSThread instance;
    return instance;
}

PPSThread::PPSThread() :
    mNotifyFd(-1)
{
#if defined(PPSTHREAD_DEBUG)
    qDebug() << "PPS: Staring PPS thread.";
#endif

    // Open a connection to the notification object. According to the pps documentation as of
    // 6.5.0, the notification id will be /less than/ 16 characters.
    // See http://www.qnx.com/developers/docs/6.5.0/index.jsp?topic=/com.qnx.doc.neutrino_pps/pps.html
    QString notifyPath = QString::fromAscii("%1/%2?wait").arg(PPSFile::ppsMountPoint()).arg(NotifyObjectName);
    QByteArray utf8NotifyPath = notifyPath.toUtf8();

    QByteArray groupIdData;
    groupIdData.resize(GroupIdLength);

    mNotifyFd = open(utf8NotifyPath.data(), O_RDONLY);

    if (mNotifyFd < 0)
        qFatalSs() << "PPS: Cannot open pps mount point at " << notifyPath;

    // The first read to the notify object always gives the group identifier string. (see documentation).
    int count = read(mNotifyFd, (void*)groupIdData.data(), groupIdData.length());

    if (count >= GroupIdLength)
        qFatalSs() << "PPS: Group id length is larger than " << GroupIdLength - 1 << " bytes. Cannot continue.";

    if (count <= 0)
        qFatalSs() << "PPS: Could not read the notification group id.";

    // String is not automatically null terminated.
    groupIdData[count] = 0;

    mGroupId = QString::fromAscii(groupIdData).trimmed();

#if defined(PPSTHREAD_DEBUG)
    qDebug() << "PPS: Connected to PPS notification object at " << notifyPath;
    qDebug() << "PPS: Group id " << mGroupId;
#endif

    // Start the monitor thread.
    start();
}

PPSThread::~PPSThread()
{
    // Close the file, which will cause the thread to exit.
    close(mNotifyFd);
    wait(1000);
    mNotifyFd = 0;
}

QString PPSThread::createUniqueId()
{
    // Just return a unique id that pps object can use to open a connection to the notify object
    return QString::fromAscii("%1").arg(mUniqueId++, 8, 16, QChar::fromAscii('0'));
}

void PPSThread::run()
{
#if defined(PPSTHREAD_DEBUG)
    qDebug() << "PPS: Thread started.";
#endif

    char buf[PpsMaxBuffer+1];
    char substr[2] = ":";
    int count;
    while((count = read(mNotifyFd, &buf, sizeof(buf)-1)) > 0) {
        // Ensure null termination
        buf[count]=0;

#if defined(PPSTHREAD_DEBUG)
        qDebug() << "PPS: .notify read =" << buf;
#endif

        // Format of string is <->ggg:vvv
        //  <-> - Optional - if this is a disconnect
        //  ggg = Group id
        //  vvv = Value (unique id of object connection).
        char* objectId = strstr(buf, substr);
        bool disconnect = buf[0] == '-';

        // Move past the separator (if we found it)
        if (!objectId) {
            qFatal("PPS: Unable to parse contents of .notify object.");
            continue;
        }
            objectId++;

        QString notifyId = QString::fromUtf8(objectId).trimmed();

        if (objectId < buf + PpsMaxBuffer-1) {
            if (disconnect) {
#if defined(PPSTHREAD_DEBUG)
                qDebug() << "PPS: Emitting notifyObjectDisconnected for " << notifyId;
#endif
                emit notifyObjectDisconnected(notifyId);
            } else {
#if defined(PPSTHREAD_DEBUG)
                qDebug() << "PPS: Emitting notifyObjectChanged for " << notifyId;
#endif
                emit notifyObjectChanged(notifyId);
            }
        }
    }

#if defined(PPSTHREAD_DEBUG)
    qDebug() << "PPS: Read " << count << " bytes. PPS thread is now exiting.";
#endif
}

} // namespace BlackBerry

QT_END_NAMESPACE
