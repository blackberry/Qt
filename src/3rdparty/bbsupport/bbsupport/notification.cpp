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

// #define NOTIFICATION_DEBUG

#define STRX(x) #x
#define STR(x) STRX(x)
#define TAG __FILE__ "(" STR(__LINE__) "): "

#include <notification.h>
#include <notificationmanager.h>

#include <QDebug>
#include <QUuid>

QT_BEGIN_NAMESPACE

namespace BlackBerry
{

namespace
{
QString CategoryToString(Notification::Category category)
{
    switch(category)
    {
        case Notification::Default:
            return QString::fromLatin1("null");
        case Notification::Ongoing:
            QString::fromLatin1("ongoing");
        default:
            qFatal(TAG "Unknown category");
    }
    return QString::fromLatin1("null");
}
}


Notification::Notification(QObject* parent) :
    QObject(parent),
    mIsShown(false),
    mIsDirty(false),
    mIsLocked(false),
    mId(QUuid::createUuid().toString()),
    mAcknowledgable(true),
    mShowInAll(true),
    mCategory(Notification::Default),
    mNotifyOnAcknowledge(true),
    mNotifyOnExpire(true),
    mNotifyOnInvoke(true)
{
#ifdef NOTIFICATION_DEBUG
    qDebug() << TAG "Created new notification: " << mId;
#endif

    connect(&NotificationManager::instance(), SIGNAL(notifyAcknowledge(QString, int)),
            this, SLOT(onAcknowledge(QString, int)));
    connect(&NotificationManager::instance(), SIGNAL(notifyExpire(QString)),
            this, SLOT(onExpire(QString)));
    connect(&NotificationManager::instance(), SIGNAL(notifyInvoke(QString)),
            this, SLOT(onInvoke(QString)));
}

Notification::~Notification()
{
#ifdef NOTIFICATION_DEBUG
    qDebug() << TAG "Destroyed notification: " << mId;
#endif
}

void Notification::showNotification()
{
#ifdef NOTIFICATION_DEBUG
    qDebug() << TAG "Showing/updating notification: " << mId;
#endif

    if (mIsLocked)
        qWarning() << TAG "showNotification called while in batch property update: " << mId;

    mIsShown = true;
    NotificationManager::instance().show(*this);
}

void Notification::cancelNotification()
{
#ifdef NOTIFICATION_DEBUG
    qDebug() << TAG "Cancelling notification: " << mId;
#endif

    mIsShown = false;
    NotificationManager::instance().cancel(*this);
}

void Notification::onAcknowledge(QString id, int promptIndex)
{
    if (id.compare(mId) != 0)
        return;

#ifdef NOTIFICATION_DEBUG
    qDebug() << TAG "Notification acknowledged: " << mId;
#endif

    mIsShown = false;
    emit notifyAcknowledged(promptIndex);
}

void Notification::onExpire(QString id)
{
    if (id.compare(mId) != 0)
        return;

#ifdef NOTIFICATION_DEBUG
    qDebug() << TAG "Notification expired: " << mId;
#endif

    mIsShown = false;
    emit notifyExpired();
}

void Notification::onInvoke(QString id)
{
    if (id.compare(mId) != 0)
        return;

#ifdef NOTIFICATION_DEBUG
    qDebug() << TAG "Notification invoked: " << mId;
#endif

    mIsShown = false;
    emit notifyInvoked();
}

QVariantHash Notification::toHash() const
{
    QVariantHash ret;

    ret[QString::fromLatin1("id")] = QVariant::fromValue(mId);
    ret[QString::fromLatin1("canAcknowledge")] = QVariant::fromValue(mAcknowledgable);
    ret[QString::fromLatin1("showInAll")] = QVariant::fromValue(mShowInAll);
    ret[QString::fromLatin1("category")] = QVariant::fromValue(CategoryToString(mCategory));
    ret[QString::fromLatin1("notifyOnAcknowledge")] = QVariant::fromValue(mNotifyOnAcknowledge);
    ret[QString::fromLatin1("notifyOnExpire")] = QVariant::fromValue(mNotifyOnExpire);
    ret[QString::fromLatin1("notifyOnInvoke")] = QVariant::fromValue(mNotifyOnInvoke);

    if (mTime.isValid())
        ret[QString::fromLatin1("time")] = QVariant::fromValue(mTime.toUTC().toMSecsSinceEpoch());

    if (mExpirationTime.isValid())
        ret[QString::fromLatin1("expirationTime")] = QVariant::fromValue(mExpirationTime.toUTC().toMSecsSinceEpoch());

    if (!mIconUrl.isEmpty())
        ret[QString::fromLatin1("iconUrl")] = QVariant::fromValue(mIconUrl);

    if (!mSoundUrl.isEmpty())
        ret[QString::fromLatin1("soundUrl")] = QVariant::fromValue(mSoundUrl);

    if (mLedColor.isValid())
        ret[QString::fromLatin1("ledColor")] = QVariant::fromValue(mLedColor.red() << 16 | mLedColor.green()<<8 | mLedColor.blue());

    if (!mAppUrl.isEmpty())
        ret[QString::fromLatin1("appUrl")] = QVariant::fromValue(mAppUrl);

    if (mHtmlText.isEmpty() && (mText.canConvert(QVariant::String) || mText.canConvert(QVariant::StringList))) {
        ret[QString::fromLatin1("text")] = mText;
    }

    if (!mHtmlText.isEmpty())
        ret[QString::fromLatin1("htmlText")] = QVariant::fromValue(mHtmlText);

    if (mPromptChoices.length() > 0)
    {
        QVariantList choices;
        for (int i = 0; i < mPromptChoices.length(); i++) {
            QVariantHash choice;
            choice[QString::fromLatin1("label")] = QVariant::fromValue(mPromptChoices[i]);
            choices.push_back(choice);
        }

        ret[QString::fromLatin1("promptChoices")] = choices;
    }

#ifdef NOTIFICATION_DEBUG
    qDebug() << TAG << "Notification hash " << ret;
#endif

    return ret;
}

void Notification::beginPropertyUpdate()
{
    mIsLocked = true;
}

void Notification::endPropertyUpdate()
{
    mIsLocked = false;

    if (mIsShown && mIsDirty)
      showNotification();

    mIsDirty = false;  
}

void Notification::setDirty()
{
    if (mIsLocked) {
        mIsDirty = true;
        return;
    }

    if (!mIsShown)
        return;

    showNotification();
}

void Notification::setTime(QDateTime const& time)
{
    mTime = time;
    setDirty();
}

void Notification::setExpirationTime(QDateTime const& expirationTime)
{
    mExpirationTime = expirationTime;
    setDirty();
}

void Notification::setIconUrl(QString const& iconUrl)
{
    mIconUrl = iconUrl;
    setDirty();
}

void Notification::setSoundUrl(QString const& soundUrl)
{
    mSoundUrl = soundUrl;
    setDirty();
}

void Notification::setLedColor(QColor const& color)
{
    mLedColor = color;
    setDirty();
}

void Notification::setAppUrl(QString const& appUrl)
{
    mAppUrl = appUrl;
    setDirty();
}

void Notification::setAcknowledgable(bool canAcknowledge)
{
    mAcknowledgable = canAcknowledge;
    setDirty();
}

void Notification::setShowInAll(bool showInAll)
{
    mShowInAll = showInAll;
    setDirty();
}

void Notification::setCategory(Category category)
{
    mCategory = category;
    setDirty();
}

void Notification::setNotifyOnAcknowledge(bool notifyOnAcknowledge)
{
    mNotifyOnAcknowledge = notifyOnAcknowledge;
    setDirty();
}

void Notification::setNotifyOnExpire(bool notifyOnExpire)
{
    mNotifyOnExpire = notifyOnExpire;
    setDirty();
}

void Notification::setNotifyOnInvoke(bool notifyOnInvoke)
{
    mNotifyOnInvoke = notifyOnInvoke;
    setDirty();
}

void Notification::setText(QVariant const& text)
{
    mText = text;
    setDirty();
}

void Notification::setHtmlText(QString const& htmlText)
{
    mHtmlText = htmlText;
    setDirty();
}

void Notification::setPromptChoices(QStringList const& promptChoices)
{
    mPromptChoices = promptChoices;
    setDirty();
}

} // namespace BlackBerry

QT_END_NAMESPACE

