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

#ifndef PPSFILE_H
#define PPSFILE_H

#include <QObject>
#include <QString>
#include <QByteArray>
#include "ppsobject.h"

QT_BEGIN_NAMESPACE
namespace BlackBerry
{

class PPSFile : public QObject
{
    Q_OBJECT

public:
    enum OpenOptions
    {
        Monitor   = 0x0000, // Monitor the pps object for changes and send a notification.
        OneShot   = 0x0001, // Read/write once to the object and then close.

        NoCreate  = 0x0000, // Fail if the file does not exist.
        Create    = 0x0002, // Create the file and directory structure if it does not exist
        Truncate  = 0x0004, // Truncate the file when opening.

        ReadFull  = 0x0000, // Read the entire pps object on all changes.
        ReadDelta = 0x0010, // Only receive delta of changes to the pps object.

        Client    = 0x0000, // Opens the pps object as a client
        Server    = 0x0100, // Opens the pps object as a publisher
        Critical  = 0x0200, // Designate the publisher as critical to the object. If this option is used when opening a file descriptor for a write, when the file descriptor is closed PPS deletes all non-persistent attributes.
        Persist   = 0x0000, // Object is persestent and will be reloaded on restart.
        NoPersist = 0x0400, // Make the object non-persistent.
    };

public:
    // Create a pps files with the specified path. The ppsMountPoint() is prepended to the
    // specified path automatically.
    PPSFile(QObject* parent, QString const& path);

    // Create a pps files with the specified path. The ppsMountPoint() is prepended to the
    // specified path automatically.v
    PPSFile(QString const& path);

    virtual ~PPSFile();

    // Returns the path of this pps object.
    QString const& path() const { return mPath; }

    // Returns the full path to the pps object.
    QString fullPath() const;

    // Open the object. If there was data in the file, the dataChanged method is called immediately.
    // In OneShot mode, the file is automatically closed after the pps object is read (if there
    // was any data available to read). In Monitor mode, the dataChanged method is called every
    // time the object changes until the object is closed.
    bool open(int openOptions = Monitor | ReadFull | Client | Create );

    // Closes the pps object. If the object has changed and there are still queued
    // changes waiting they will be discarded.
    void close();

    // Returns true of the pps object is currently opened.
    bool isOpen() const;

    // Write a byte array string to the pps object. Note that the maximum length of a PPS object is
    // limited. The default limit is 16384 bytes. If you need to share more data, you must
    // configure the pps system service differently or break your messages apart. You must also
    // be aware of this restriction when reading from a pps object.
    bool write(QByteArray const& rawData);

    // Converts the string to utf-8 and then writes the string to the pps object. Keep in mind the
    // size limit.
    bool write(QString const& rawData);

    // Encode pps data automatically using the PPSData object. Set the json flag to true
    // if the entire object should be json encoded, otherwise pps will choose automatically.
    bool write(PPSObject const& ppsObject, bool json = false);

    // Default is 16384 bytes. If your system is configured for larger pps reads/writes you can change
    // this value.
    static int ppsSizeMax();

    // This value MUST be at least 512 bytes and the maximum is 65536 bytes
    // See http://www.qnx.com/developers/docs/6.5.0/index.jsp?topic=/com.qnx.doc.neutrino_pps/pps.html
    static void setPpsSizeMax(int value);

    // Retrieve the current PPS Mount point.
    static QString const& ppsMountPoint();

protected:
    // Called whenever the data changes. The QByteArray object contains the content of the file
    // which may be parsed automatically. See the documentation of PPSData
    virtual void dataChanged(QByteArray &ppsData);

    // Callen whenever this object has been closed.
    virtual void fileClosed();

private slots:
    // Called whenever the notify object has signalled we have changed
    void onObjectChanged(QString objectUniqueId);

    // Called whenever we have been disconnected from the notify object.
    void onObjectDisconnected(QString objectUniqueId);

private:
    PPSFile(PPSFile const&);   // Don't Implement.
    void operator=(PPSFile const&); // Don't implement

    bool tryRead();

    QString mUniqueId;
    QString mPath;
    int mFd;
    static int smSizeMax;
    int mOptions;
};

} //namespace BlackBerry
QT_END_NAMESPACE


#endif //PPSFILE_H


