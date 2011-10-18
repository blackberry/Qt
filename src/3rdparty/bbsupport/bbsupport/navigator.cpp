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

// #define PPS_NAVIGATOR_DEBUG

#include <navigator.h>

#include <QDebug>
#include <QList>
#include <QStringList>

QT_BEGIN_NAMESPACE

namespace BlackBerry
{

namespace
{

#define dp() NavigatorPrivate *p = (NavigatorPrivate *)mPrivate;

class NavigatorDisplayInfo
{
public:
    NavigatorDisplayInfo() :
        mWindowState(Navigator::FullScreen),
        mIsActivated(true) {}
    ~NavigatorDisplayInfo() {};

    QString mGroupId;
    Navigator::WindowState mWindowState;
    bool mIsActivated;
};

class NavigatorPrivate
{
    public:
    NavigatorPrivate() :
        mIsOrientationLocked(false),
        mIsReorienting(false) {}

    ~NavigatorPrivate() {}

    NavigatorDisplayInfo& getDisplayInfo(Navigator::Display display)
    {
        if (display == Navigator::Primary)
            return mPrimary;
        else
            return mSecondary;
    }

    Navigator::Display findDisplay(QString const& groupId)
    {
        // Navigator will send an empty groupId for the primary display.
        if (groupId.compare(mPrimary.mGroupId) == 0)
            return Navigator::Primary;
        else if (groupId.compare(mPrimary.mGroupId) == 0)
            return Navigator::Secondary;

        qWarning() << "Unrecognized group id from navigator (using primary): " << groupId;
        return Navigator::Primary;
    }

    bool mIsOrientationLocked;
    bool mIsReorienting;
    NavigatorDisplayInfo mPrimary;
    NavigatorDisplayInfo mSecondary;
};

}   // Private namespace for private data.

Navigator& Navigator::instance()
{
    static Navigator instance;
    return instance;
}

Navigator::Navigator() :
    PPSFile(0, QString::fromLatin1("services/navigator/control")),
    mPrivate(new NavigatorPrivate())
{
    if (!open())
        qCritical("Could not open a connection to the navigator - application may not function correctly.");
}

Navigator::~Navigator()
{
    dp()

    delete p;
    mPrivate = 0;
}

void Navigator::setGroupIds(QString const& primaryGroupId, QString const& secondaryGroupId)
{
    dp()
    p->getDisplayInfo(Primary).mGroupId = primaryGroupId;
    p->getDisplayInfo(Secondary).mGroupId = secondaryGroupId;
}

Navigator::WindowState Navigator::windowState(Display display) const
{
    dp()
    return p->getDisplayInfo(display).mWindowState;
}

bool Navigator::isActivated(Display display) const
{
    dp()
    return p->getDisplayInfo(display).mIsActivated;
}

void Navigator::dataChanged(QByteArray &ppsData)
{
    QList<PPSObject> parsed = PPSObject::parse(ppsData);
    QList<PPSObject>::iterator it;
    for (it = parsed.begin(); it != parsed.end(); it++) {
        handlePPSMessage(*it);
    }
}

void Navigator::handlePPSMessage(PPSObject const& message)
{
#ifdef PPS_NAVIGATOR_DEBUG
    qDebug() << "Handling pps message with attributes: " << message.attributes();
#endif

    QVariantHash attributes = message.attributes().toHash();
    if (!attributes.contains(QString::fromLatin1("msg"))) {
        qWarning() << "PPS message improperly formatted. No msg attribute found.";
        return;
    }

    QString msg = attributes[QString::fromLatin1("msg")].toString();
    QString dat = attributes[QString::fromLatin1("dat")].toString();
    QString id = attributes[QString::fromLatin1("id")].toString();

    if (msg.compare(QString::fromLatin1("orientationCheck"), Qt::CaseInsensitive) == 0)
        handleOrientationCheck(dat, id);
    else if (msg.compare(QString::fromLatin1("orientation"), Qt::CaseInsensitive) == 0)
        handleStartOrientation(dat, id);
    else if (msg.compare(QString::fromLatin1("orientationDone"), Qt::CaseInsensitive) == 0)
        handleEndOrientation(id);
    else if (msg.compare(QString::fromLatin1("windowState"), Qt::CaseInsensitive) == 0)
        handleWindowStateChanged(dat);
    else if (msg.compare(QString::fromLatin1("windowActive"), Qt::CaseInsensitive) == 0)
        handleWindowActivated(dat, true);
    else if (msg.compare(QString::fromLatin1("windowInactive"), Qt::CaseInsensitive) == 0)
        handleWindowActivated(dat, false);
    else if (msg.compare(QString::fromLatin1("SWIPE_DOWN"), Qt::CaseInsensitive) == 0)
        emit notifySwipeDown();
    else if (msg.compare(QString::fromLatin1("SWIPE_START"), Qt::CaseInsensitive) == 0)
        emit notifySwipeStart();
    else if (msg.compare(QString::fromLatin1("invoke"), Qt::CaseInsensitive) == 0)
        emit notifyInvoke(dat);
    else if (msg.compare(QString::fromLatin1("back"), Qt::CaseInsensitive) == 0)
        emit notifyBack();
    else if (msg.compare(QString::fromLatin1("exit"), Qt::CaseInsensitive) == 0)
        emit notifyExit();
    else if (msg.compare(QString::fromLatin1("LOW_MEMORY"), Qt::CaseInsensitive) == 0)
        emit notifyLowMemory();
    else
        qWarning() << "Unsupported message type from navigator: " << msg;
}

void Navigator::disconnected()
{
    emit notifyNavigatorDisconnected();
}

void Navigator::handleOrientationCheck(QString const& angle, QString const& id)
{
    bool ok = false;
    int iAngle = angle.toInt(&ok);

    if (!ok) {
        qWarning() << "Navigator requested orientation of " << iAngle << " which is not supported.";
        return;
    }

    // If orientation is locked, then we will reply false.
    ok &= !isOrientationLocked();

    QVariantHash msg;
    msg[QString::fromLatin1("res")] = QString::fromLatin1("orientationCheck");
    msg[QString::fromLatin1("id")] = id;
    msg[QString::fromLatin1("dat")] = ok?QString::fromLatin1("true"):QString::fromLatin1("false");
    write(PPSObject(QString(), msg));
}

void Navigator::handleStartOrientation(QString const& angle, QString const& id)
{
    bool ok = false;
    int iAngle = angle.toInt(&ok);

    if (!ok) {
        qWarning() << "Navigator requested orientation of " << iAngle << " which is not supported.";
        return;
    }

    emit notifyStartOrientation(iAngle, id);
}

void Navigator::handleEndOrientation(QString const& id)
{
    emit notifyEndOrientation(id);
}

void Navigator::handleWindowStateChanged(QString const& newState)
{
    dp()
    WindowState state;
    Display display;

    QStringList params = newState.split(QChar::fromLatin1(','));

    if (params.length() != 2) {
        qWarning() << "Expected 2 parameters for windowState, got " << params;
        return;
    }

    if (params[0].compare(QString::fromLatin1("fullscreen"), Qt::CaseInsensitive) == 0)
        state = FullScreen;
    else if (params[0].compare(QString::fromLatin1("thumbnail"), Qt::CaseInsensitive) == 0)
        state = Thumbnail;
    else if (params[0].compare(QString::fromLatin1("invisible"), Qt::CaseInsensitive) == 0)
        state = Invisible;
    else {
        qWarning() << "Unrecognized window state from navigator: " << params[0];
        state = Invisible;
    }

    display = p->findDisplay(params[1]);
    p->getDisplayInfo(display).mWindowState = state;
    emit notifyWindowStateChanged(state, display);
}

void Navigator::handleWindowActivated(QString const& groupId, bool activated)
{
    dp()
    Display display;

    display = p->findDisplay(groupId);
    p->getDisplayInfo(display).mIsActivated = activated;
    emit notifyWindowActivated(activated, display);
}

void Navigator::launchUrl(QString const& url)
{
    QVariantHash msg;
    msg[QString::fromLatin1("msg")] = QString::fromLatin1("invoke");
    msg[QString::fromLatin1("dat")] = url;

    write(PPSObject(QString(), msg));
}

void Navigator::openFile(QFileInfo const& file)
{
    QVariantHash msg;
    msg[QString::fromLatin1("msg")] = QString::fromLatin1("openFile");
    msg[QString::fromLatin1("dat")] = file.path();

    write(PPSObject(QString(), msg));
}

void Navigator::enableSwipeStart()
{
    QVariantHash msg;
    msg[QString::fromLatin1("msg")] = QString::fromLatin1("SWIPE_START");

    write(PPSObject(QString(), msg));
}

void Navigator::addURI(QString const& URL, QFileInfo const& iconPath, QString const& label, QString const& defaultCategory)
{
    QString args = QString(QString::fromLatin1("%1,%2,%3,%4")).arg(iconPath.path(), label, defaultCategory, URL);

    QVariantHash msg;
    msg[QString::fromLatin1("msg")] = QString::fromLatin1("addURI");
    msg[QString::fromLatin1("dat")] = args;

    write(PPSObject(QString(), msg));
}

void Navigator::setOrientation(int orientation)
{
    if (orientation % 90 != 0 || orientation < 0 || orientation > 270)
        return;

    QVariantHash msg;
    msg[QString::fromLatin1("msg")] = QString::fromLatin1("orientation");
    msg[QString::fromLatin1("dat")] = QString(QString::fromLatin1("%1")).arg(orientation);

    write(PPSObject(QString(), msg));
}

void Navigator::lockOrientation()
{
    dp()
    QVariantHash msg;
    msg[QString::fromLatin1("msg")] = QString::fromLatin1("lockOrientation");

    write(PPSObject(QString(), msg));

    p->mIsOrientationLocked = true;
}

void Navigator::unlockOrientation()
{
    dp()
    QVariantHash msg;
    msg[QString::fromLatin1("msg")] = QString::fromLatin1("unlockOrientation");

    write(PPSObject(QString(), msg));

    p->mIsOrientationLocked = false;
}

bool Navigator::isOrientationLocked() const
{
    dp()
    return p->mIsOrientationLocked;
}

bool Navigator::isReorienting() const
{
    dp()
    return p->mIsReorienting;
}

void Navigator::addSplat()
{
    QVariantHash msg;
    msg[QString::fromLatin1("msg")] = QString::fromLatin1("addBadge");
    msg[QString::fromLatin1("dat")] = QString::fromLatin1("splat");

    write(PPSObject(QString(), msg));
}

void Navigator::removeBadge()
{
    QVariantHash msg;
    msg[QString::fromLatin1("msg")] = QString::fromLatin1("removeBadge");

    write(PPSObject(QString(), msg));
}

void Navigator::extendTimeout(unsigned int milliseconds)
{
    QVariantHash msg;
    msg[QString::fromLatin1("msg")] = QString::fromLatin1("extendTimeout");
    msg[QString::fromLatin1("dat")] = QString(QString::fromLatin1("%1")).arg(milliseconds);

    write(PPSObject(QString(), msg));
}

void Navigator::setWindowAngle(int angle)
{
    QVariantHash msg;
    msg[QString::fromLatin1("msg")] = QString::fromLatin1("windowAngle");
    msg[QString::fromLatin1("dat")] = QString(QString::fromLatin1("%1")).arg(angle);

    write(PPSObject(QString(), msg));
}

void Navigator::setWindowLabel(QString const& label)
{
    QVariantHash msg;
    msg[QString::fromLatin1("msg")] = QString::fromLatin1("windowLabel");
    msg[QString::fromLatin1("dat")] = label;

    write(PPSObject(QString(), msg));
}

void Navigator::setClosePrompt(QString const& title, QString const& message)
{
    QVariantHash msg;
    msg[QString::fromLatin1("msg")] = QString::fromLatin1("closePrompt");
    msg[QString::fromLatin1("dat")] = QString(QString::fromLatin1("%1,%2")).arg(title, message);

    write(PPSObject(QString(), msg));
}

void Navigator::clearClosePrompt()
{
    setClosePrompt(QString(), QString());
}

void Navigator::sendBufferResized(QString const& id)
{
    QVariantHash msg;
    msg[QString::fromLatin1("res")] = QString::fromLatin1("orientation");
    msg[QString::fromLatin1("id")] = id;

    write(PPSObject(QString(), msg));
}

} // namespace BlackBerry

QT_END_NAMESPACE

