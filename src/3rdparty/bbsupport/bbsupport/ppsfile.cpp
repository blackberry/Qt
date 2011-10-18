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

// #define PPSFile_DEBUG

#include "ppsfile.h"
#include "ppsthread.h"

#include <QDebug>
#include <QFileInfo>
#include <QDir>

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

QT_BEGIN_NAMESPACE
namespace BlackBerry
{

/**
 * PPS holds its objects in memory, and therefore these objects are small:
 *
 * the default size for a PPS object is 16 kilobytes
 * the maximum size for a PPS object is 64 kilobytes
 *
 * http://www.qnx.com/developers/docs/6.5.0/index.jsp?topic=/com.qnx.doc.neutrino_pps/pps.html
 */
int PPSFile::smSizeMax = 16384;

PPSFile::PPSFile(QObject* parent, QString const& path) :
      QObject(parent),
      mPath(path),
      mFd(-1),
      mOptions(0)
{
#if defined(PPSFile_DEBUG)
    qDebug() << "PPSObject: Creating PPSObject with parent " << parent << " on " << path;
#endif
}

PPSFile::PPSFile(QString const& path) :
    mPath(path),
    mFd(-1)
{
#if defined(PPSFile_DEBUG)
    qDebug() << "PPSObject: Creating PPSObject with no parent on " << path;
#endif
}

PPSFile::~PPSFile()
{
#if defined(PPSFile_DEBUG)
    qDebug() << "PPSObject: Destroying PPSObject on " << mPath;
#endif

    close();
}

bool PPSFile::open(int options)
{
    if (isOpen()) {
        qWarning() << "PPSObject: Already opened " << mPath;
        return false;
    }

    mOptions = options;
    QList<QString> optionsList;

    // If we want to monitor this file, then we need to generate a unique ID (and start the
    // monitor thread). Do so now.
    if (!(options & OneShot)) {
        mUniqueId = PPSThread::instance().createUniqueId();
        optionsList += QString::fromAscii("notify=%1:%2").arg(PPSThread::instance().groupId()).arg(mUniqueId);

        // Connect this object now.
        connect(&PPSThread::instance(), SIGNAL(notifyObjectChanged(QString)),
                SLOT(onObjectChanged(QString)));
        connect(&PPSThread::instance(), SIGNAL(notifyObjectDisconnected(QString)),
                SLOT(onObjectDisconnected(QString)));
    }

    if (options & ReadDelta)
        optionsList += QString::fromAscii("delta");

    if (options & Server)
        optionsList += QString::fromAscii("server");

    if (options & Critical)
        optionsList += QString::fromAscii("critical");

    if (options & NoPersist)
        optionsList += QString::fromAscii("nopersist");

    // Build the full path string with options.
    QList<QString>::iterator it;
    QString pathWithOptions = fullPath();

    // ... but first check to make sure that the file exists, because we'll need to create it
    // later if it does not.
    QFileInfo fileInfo(pathWithOptions);
    if (!(options & Create) && !fileInfo.exists())
        return false;

    for (it = optionsList.begin(); it != optionsList.end(); it++) {
        if (it != optionsList.begin())
            pathWithOptions += QString::fromAscii(",");
        else
            pathWithOptions += QString::fromAscii("?");

        pathWithOptions += *it;
    }

    // Filesystem supports utf8 names, so be sure to convert it first.
    QByteArray pathWithOptionsUtf8 = pathWithOptions.toUtf8();
#if defined(PPSFile_DEBUG)
    qDebug() << "PPSObject: Opening PPSObject on " << pathWithOptionsUtf8;
#endif

    if ((options & Truncate) || !fileInfo.exists())
    {
        // First ensure that the directory structure exists.
        QDir dir = fileInfo.dir();
        if (!dir.exists())
            dir.mkpath(dir.path());

        // Create and/or truncate the file.
        mFd = ::open(pathWithOptionsUtf8.data(), O_RDWR | O_CREAT);
    } else {
        mFd = ::open(pathWithOptionsUtf8.data(), O_RDWR);
    }

    if (mFd == -1) {
        qWarning() << "PPSObject:Could not open " << pathWithOptionsUtf8;
        close();
        return false;
    }

    // Try to read from the pps object
    tryRead();

    // If are oneshotting, then close now.
    if (options & OneShot)
        close();

    return true;
}

void PPSFile::close()
{
#if defined(PPSFile_DEBUG)
    qDebug() << "PPSObject: Closing PPSObject on " << mPath;
#endif

    // Disconnect from the PPS Thread only (we don't want to disconnect anyone else)
    QObject::disconnect(&PPSThread::instance(), 0, this, 0);

    if (mFd != -1) {
        ::close(mFd);
        mFd = -1;
    }

    mUniqueId = QString();
}

bool PPSFile::isOpen() const
{
    return mFd != -1;
}

bool PPSFile::write(QByteArray const& rawData)
{
    if (!isOpen()) {
        qWarning() << "PPSObject: Attempt to write to \"" << path() << "\" when not opened.";
        return false;
    }

    if (rawData.length() > ppsSizeMax())
        qWarning() << "PPSObject: Write to \"" << mPath << "\" may be truncated. Max size set to " << ppsSizeMax() << " and object length was " << rawData.length();

#if defined(PPSFile_DEBUG)
    qDebug() << "PPSObject: writing '" << rawData.data() << "' to" << mPath;
#endif

    errno = 0;
    int count = ::write(mFd, rawData.data(), rawData.length());

    if (count == -1) {
        qWarning() << "PPSObject: Write to \"" << mPath << "\" failed. errno = " << errno;
        return false;
    }

    if (count != rawData.length()) {
        qWarning() << "PPSObject: Write to \"" << mPath << "\" failed. "
                   << count << " of " << rawData.length() << " bytes written.";
        return false;
    }

    return true;
}

bool PPSFile::write(QString const& rawData)
{
#if defined(PPSFile_DEBUG)
    qDebug() << "PPSObject: converting string and writing to " << mPath;
#endif

    QByteArray utf8String = rawData.toUtf8();
    return write(utf8String);
}

bool PPSFile::write(PPSObject const& ppsObject, bool json)
{
#if defined(PPSFile_DEBUG)
    qDebug() << "PPSObject: converting PPSObject and writing to " << mPath;
#endif

    return write(ppsObject.serializeToPps(json));
}

void PPSFile::dataChanged(QByteArray &ppsData)
{
    Q_UNUSED(ppsData);
    qWarning() << "PPSFile: dataChanged not implemented.";
}

void PPSFile::fileClosed()
{
    qWarning() << "PPSFile: fileClosed not implemented.";
}

void PPSFile::onObjectChanged(QString objectUniqueId)
{
    if(objectUniqueId != mUniqueId)
        return;

#if defined(PPSFile_DEBUG)
    qDebug() << "PPSObject: PPS object changed:" << mPath;
#endif

    // When reading from an object that is bound to a notification group, a subscriber
    // should do multiple reads for each change indicated. There may be more than one
    // change on an item, but there is no guarantee that every change will be indicated
    // on the notification group's file descriptor.
    while(tryRead());
}

void PPSFile::onObjectDisconnected(QString objectUniqueId)
{
    if(objectUniqueId != mUniqueId)
        return;

    fileClosed();
}

bool PPSFile::tryRead()
{
    if (!isOpen()) {
        qWarning() << "PPSObject: Attempted to tryRead after object was closed";
        return false;
    }

    QByteArray ppsData(ppsSizeMax() + 1, 0);

    errno = 0;
    int readBytes = read(mFd, ppsData.data(), ppsSizeMax());

    // Ensure null termination.
    ppsData[readBytes] = 0;

#if defined(PPSFile_DEBUG)
    qDebug() << "PPSObject: tryRead readBytes ==" << readBytes << "on" << mPath;
#endif

    if (readBytes == -1) {
        qWarning() << "PPSObject: Could not read from" << mPath << "errno=" << errno;
        close();
        fileClosed();
        return false;
    }

    if (readBytes == 0) {
        // There was nothing in the file, this is okay.
        return false;
    }

    // Ensure null termination.
    ppsData[readBytes] = 0;

#if defined(PPSFile_DEBUG)
    qDebug() << "PPSObject: content='" << ppsData.data()<<"'";
#endif

    // We read something from the file. Wrap it with the PPSData object and call the
    // dataChanged method. pps encodes data in utf8 format, so be sure to decode it first.
    dataChanged(ppsData);

    return true;
}

int PPSFile::ppsSizeMax()
{
    return smSizeMax;
}

void PPSFile::setPpsSizeMax(int value)
{
    if (value >= 512 && value <= 65536)
        smSizeMax = value;
}

QString const& PPSFile::ppsMountPoint()
{
    // TODO: Find the pps root correctly - it's not an environment variable right now
    //       but perhaps it should be.
    static QString ppsMountPoint = QString::fromAscii("/pps/");

    return ppsMountPoint;
}

QString PPSFile::fullPath() const
{
    return ppsMountPoint() + mPath;
}

} // namespace BlackBerry
QT_END_NAMESPACE

