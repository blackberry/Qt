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

// #define PPS_KEYBOARD_DEBUG

#include <keyboard.h>
#include <navigator.h>

#include <QDebug>
#include <QList>
#include <QStringList>

QT_BEGIN_NAMESPACE

namespace BlackBerry
{

namespace
{
    QString enterTypeToString(Keyboard::EnterType type)
    {
        switch(type) {
        case Keyboard::DefaultEnter:
            return QString::fromLatin1("default");
        case Keyboard::Go:
            return QString::fromLatin1("go");
        case Keyboard::Join:
            return QString::fromLatin1("join");
        case Keyboard::Next:
            return QString::fromLatin1("next");
        case Keyboard::Search:
            return QString::fromLatin1("search");
        case Keyboard::Send:
            return QString::fromLatin1("send");
        case Keyboard::Submit:
            return QString::fromLatin1("submit");
        case Keyboard::Done:
            return QString::fromLatin1("done");
        case Keyboard::Connect:
            return QString::fromLatin1("connect");
        default:
            qFatal("Unexpected EnterType in Blackberry::Keyboard");
        }

        return QString();
    }

    QString keyboardTypeToString(Keyboard::KeyboardType type)
    {
        switch(type) {
        case Keyboard::DefaultKeyboard:
            return QString::fromLatin1("DefaultKeyboard");
        case Keyboard::URL:
            return QString::fromLatin1("URL");
        case Keyboard::Email:
            return QString::fromLatin1("Email");
        default:
            qFatal("Unexpected KeyboardType in Blackberry::Keyboard");
        }

        return QString();
    }
}

Keyboard& Keyboard::instance()
{
    static Keyboard instance;
    return instance;
}

Keyboard::Keyboard() :
    PPSFile(0, QString::fromLatin1("services/input/control")),
    mKeyboardType(DefaultKeyboard),
    mEnterType(DefaultEnter),
    mIsVisible(false),
    mKeyboardHeight(-1),
    mLanguageId(QString::fromLatin1("en")),
    mCountryId(QString::fromLatin1("US"))

{
    if (!open())
        qCritical("Could not open a connection to the keyboard - application may not function correctly.");

    QVariantHash msg;
    msg[QString::fromLatin1("msg")] = QString::fromLatin1("info");
    msg[QString::fromLatin1("id")] = QString::fromLatin1("QtKeyboard");
    write(PPSObject(QString(), msg));

    connect(&Navigator::instance(), SIGNAL(notifyWindowActivated(bool, Navigator::Display)),
            this, SLOT(onWindowActivated(bool, Navigator::Display)));
}

Keyboard::~Keyboard()
{
    disconnect(&Navigator::instance(), 0, this, 0);
}

void Keyboard::dataChanged(QByteArray &ppsData)
{
    QList<PPSObject> parsed = PPSObject::parse(ppsData);
    QList<PPSObject>::iterator it;
    for (it = parsed.begin(); it != parsed.end(); it++) {
        handlePPSMessage(*it);
    }
}

void Keyboard::handlePPSMessage(PPSObject const& message)
{
#ifdef PPS_KEYBOARD_DEBUG
    qDebug() << "Handling pps message with attributes: " << message.attributes();
#endif

    if (message.objectStatus() != PPSObject::Undefined &&
        message.objectStatus() != PPSObject::Created) {
        // In all other cases the keyboard service is probably gone.
        handleHide();
        return;
    }

    QVariantHash attributes = message.attributes().toHash();

    QString msg = attributes[QString::fromLatin1("msg")].toString();
    // We don't care if it's a response or message right now.
    if(msg.isEmpty())
        msg = attributes[QString::fromLatin1("res")].toString();

    if (msg.compare(QString::fromLatin1("show"), Qt::CaseInsensitive) == 0)
        handleShow();
    else if (msg.compare(QString::fromLatin1("hide"), Qt::CaseInsensitive) == 0)
        handleHide();
    else if (msg.compare(QString::fromLatin1("info"), Qt::CaseInsensitive) == 0)
        handleInfo(attributes[QString::fromLatin1("dat")]);
    else if (msg.compare(QString::fromLatin1("connect"), Qt::CaseInsensitive) == 0)
        return; // We don't care, but no need to log a warning.
    else
        qWarning() << "Unsupported message type from keyboard: " << msg;
}

void Keyboard::disconnected()
{
}

void Keyboard::handleShow()
{
    mIsVisible = true;
    trySendKeyboardType();
    emit notifyVisibilityChanged(true);
}

void Keyboard::handleHide()
{
    mIsVisible = false;
    emit notifyVisibilityChanged(false);
}

void Keyboard::handleInfo(QVariant const& properties)
{
    bool ok = false;
    QVariantHash items = properties.toHash();
    QString newSize = items[QString::fromLatin1("size")].toString();
    QVariantHash locale = items[QString::fromLatin1("locale")].toHash();

    int intSize = newSize.toInt(&ok);

    if (!locale.isEmpty())
    {
        QString languageId = locale[QString::fromLatin1("languageId")].toString();
        QString countryId = locale[QString::fromLatin1("countryId")].toString();

        if (!languageId.isEmpty())
            mLanguageId = languageId;

        if (!countryId.isEmpty())
            mCountryId = countryId;
    }

    if (ok && intSize != mKeyboardHeight) {
        mKeyboardHeight = intSize;
        emit notifyHeightChanged(mKeyboardHeight);
    }

#ifdef PPS_KEYBOARD_DEBUG
    qDebug() << "Keyboard: height:" << mKeyboardHeight << " languageId:" << mLanguageId
             << " countryId:" << mCountryId;
#endif

}

void Keyboard::show()
{
    QVariantHash msg;
    msg[QString::fromLatin1("msg")] = QString::fromLatin1("show");
    msg[QString::fromLatin1("id")] = QString::fromLatin1("QtKeyboard");
    write(PPSObject(QString(), msg));
}

void Keyboard::hide()
{
    QVariantHash msg;
    msg[QString::fromLatin1("msg")] = QString::fromLatin1("hide");
    msg[QString::fromLatin1("id")] = QString::fromLatin1("QtKeyboard");
    write(PPSObject(QString(), msg));
}

bool Keyboard::isVisible() const
{
    return mIsVisible;
}

int Keyboard::keyboardHeight() const
{
    // They keyboard has a shadow, lets clip it out automatically
    return mKeyboardHeight - 8;
}

void Keyboard::setKeyboardType(KeyboardType keyboardType, EnterType enterType)
{
    mKeyboardType = keyboardType;
    mEnterType = enterType;
    trySendKeyboardType();
}

void Keyboard::trySendKeyboardType()
{
    if (!isVisible() || Navigator::instance().isActivated(Navigator::Primary))
        return;

    // Build the message and send it off.
    QVariantHash options;
    options[QString::fromLatin1("enter")] = enterTypeToString(enterType());
    options[QString::fromLatin1("keyboard")] = keyboardTypeToString(keyboardType());

    QVariantHash msg;
    msg[QString::fromLatin1("msg")] = QString::fromLatin1("options");
    msg[QString::fromLatin1("id")] = QString::fromLatin1("QtKeyboard");
    msg[QString::fromLatin1("dat")] = options;

    write(PPSObject(QString(), msg));
}

Keyboard::KeyboardType Keyboard::keyboardType() const
{
    return mKeyboardType;
}

Keyboard::EnterType Keyboard::enterType() const
{
    return mEnterType;
}

void Keyboard::onWindowActivated(bool activated, Navigator::Display display)
{
    // IF we are activated on the primary display AND the keyboard is currently visible,
    // ensure the right keyboard type is being displayed.
    if (activated && display == Navigator::Primary) {
        trySendKeyboardType();
    } else if (!activated) {
        // We need to hide the keyboard if we are no longer active.
        // Note: I almost feel like this is a problem for the integration class.
        hide();
    }
}

QString Keyboard::languageId() const
{
    return mLanguageId;
}

QString Keyboard::countryId() const
{
    return mCountryId;
}

} // namespace BlackBerry

QT_END_NAMESPACE

