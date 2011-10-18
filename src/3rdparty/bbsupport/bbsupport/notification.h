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

#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include <bbsupport/PPSObject>
#include <bbsupport/PPSFile>

#include <QtGui/QColor>
#include <QDateTime>
#include <QObject>
#include <QString>
#include <QStringList>

QT_BEGIN_NAMESPACE

namespace BlackBerry
{

class Notification : public QObject
{
    Q_OBJECT

public:
    enum Category
    {
        Default,
        Ongoing
    };

public:
    /**
     * Construct a new notification and connect it to the Notification Manager.
     */
    Notification(QObject* parent = 0);
    virtual ~Notification();

    /**
     * showNotification
     *
     * Shows the notification or updates the currently display notification with
     * the current properties.
     */
    void showNotification();
    bool isShown() const;

    /**
     * cancelNotification
     *
     * Cancels the notification if it is currently being shown.
     */
    void cancelNotification();

    /**
     * beginPropertyUpdate
     *
     * Call this function before you update a number of properties if you wish to
     * perform a batch update. Otherwise, a visibile notification will be updated
     * whenever any property is updated.
     */
    void beginPropertyUpdate();

    /**
     * endPropertyUpdate
     *
     * Call this function after you have finished a batch property update to update
     * the notification object.
     */
    void endPropertyUpdate();

// Updating the property of a shown update
public:
    /**
     * id
     *
     * The id of the notification.
     */
    QString id() const { return mId; }

    /**
     * time
     *
     * A timestamp for the notification.
     *
     * The default is QDateTime()
     */
    QDateTime time() const { return mTime; }
    void setTime(QDateTime const& time);

    /**
     * expirationTime
     *
     * Specifies the absolute time for the expiration of the notification.
     * An expiration time of QDateTime() will disable the expiration.
     *
     * The default value is QDateTime()
     */
    QDateTime expirationTime() const { return mExpirationTime; }
    void setExpirationTime(QDateTime const& expirationTime);

    /**
     * iconUrl
     *
     * The URL, specified as a valid Unix path, to the icon representing the
     * notification. This icon is displayed in the notification list if the
     * system status screen when the notification is created.
     *
     * The default value is an empty string.
     */
    QString iconUrl() const { return mIconUrl; }
    void setIconUrl(QString const& iconUrl);

    /**
     * soundUrl
     *
     * The URL, specified as a valid Unix path, to the system sound file to play
     * when the notification is created.
     *
     * The default value is an empty string.
     */
    QString soundUrl() const { return mSoundUrl; }
    void setSoundUrl(QString const& soundUrl);

    /**
     * ledColor
     *
     * RGB color value for the LED that will blink with this notification, or QColor()
     * if not specified.
     *
     * The default color is QColor().
     */
    QColor ledColor() const { return mLedColor; }
    void setLedColor(QColor const& color);

    /**
     * appUrl
     *
     * The URL, specified as a valid Unix path, to the application. The user can
     * run the application by selecting or opening the notifcation.
     *
     * The default value is null
     */
    QString appUrl() const { return mAppUrl; }
    void setAppUrl(QString const& appUrl);

    /**
     * isAcknowledgable
     *
     * Set to true to indicate the notification can be acknowledged/dismissed.
     * This attribute is ignored for notification prompts.
     *
     * The default value is true.
     */
    bool isAcknowledgable() const { return mAcknowledgable; }
    void setAcknowledgable(bool canAcknowledge);

    /**
     * isShownInAll
     *
     * Set to true to indicate the notification should appear under both the
     * client indication, as well as the "all" indication. This property is
     * ignored for clients that do not provide an indication.
     *
     * The default value is true.
     */
    bool isShownInAll() const { return mShowInAll; }
    void setShowInAll(bool showInAll);

    /**
     * category
     *
     * The category of the notification. Notifications that belong in the same
     * category are listed together.
     *
     * The default value is Default
     */
    Category category() const { return mCategory; }
    void setCategory(Category category);

    /**
     * willNotifyOnAcknowledge
     *
     * Set to tru to notify you when the user acknowledges this notification.
     *
     * The default value is true
     */
    bool willNotifyOnAcknowledge() const { return mNotifyOnAcknowledge; }
    void setNotifyOnAcknowledge(bool notifyOnAcknowledge);

    /**
     * willNotifyOnExpire
     *
     * Set to tru to notify you when the notification expires.
     *
     * The default value is true
     */
    bool willNotifyOnExpire() const { return mNotifyOnExpire; }
    void setNotifyOnExpire(bool notifyOnExpire);

    /**
     * willNotifyOnInvoke
     *
     * Set to tru to notify you when the user invokes (opens) this notification.
     *
     * The default value is true
     */
    bool willNotifyOnInvoke() const { return mNotifyOnInvoke; }
    void setNotifyOnInvoke(bool notifyOnInvoke);

    /**
     * text
     *
     * One of either a string or an array that describes the notification. A string
     * specifies a single text element that describes the notification.
     *
     * An array specifies a number of strings that describe the notification.
     *
     * The default is null.
     */
    QVariant text() const { return mText; }
    void setText(QVariant const& text);

    /**
     * htmlText
     *
     * An HTML-formatted text string. If set, this value overrides any text
     * property that was specified by the text property.
     *
     * The default is null.
     */
    QString htmlText() const { return mHtmlText; }
    void setHtmlText(QString const& htmlText);

    /**
     * promptChoices
     *
     * Specifies the array of acknowledgement choices presented to the user for this
     * notification. If this is a normal notification that does not prompt the user,
     * this list can be empty.
     *
     * The default is an empty list.
     */
    QStringList promptChoices() const { return mPromptChoices; }
    void setPromptChoices(QStringList const& promptChoices);

    /**
     * Convert this object to a hash that can be used to communicate with the PPS server.
     */
    QVariantHash toHash() const;

signals:
    void notifyAcknowledged(int promptIndex);
    void notifyExpired();
    void notifyInvoked();

private slots:
    void onAcknowledge(QString id, int promptIndex);
    void onExpire(QString id);
    void onInvoke(QString id);

private:
    void setDirty();
    bool mIsShown;
    bool mIsDirty;
    bool mIsLocked;

    QString mId;
    QDateTime mTime;
    QDateTime mExpirationTime;
    QString mIconUrl;
    QString mSoundUrl;
    QColor mLedColor;
    QString mAppUrl;
    bool mAcknowledgable;
    bool mShowInAll;
    Category mCategory;
    bool mNotifyOnAcknowledge;
    bool mNotifyOnExpire;
    bool mNotifyOnInvoke;
    QVariant mText;
    QString mHtmlText;
    QStringList mPromptChoices;
};


} //namespace BlackBerry

QT_END_NAMESPACE

#endif // NOTIFICATION_H

