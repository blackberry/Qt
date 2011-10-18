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

#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <bbsupport/Navigator>
#include <bbsupport/PPSObject>
#include <bbsupport/PPSFile>

#include <QFileInfo>
#include <QObject>
#include <QString>

QT_BEGIN_NAMESPACE

namespace BlackBerry
{

class Keyboard : public PPSFile
{
    Q_OBJECT

public:
    enum KeyboardType
    {
        DefaultKeyboard,    // Use the default keyboard.
        URL,                // Use the URL keyboard layout.
        Email               // Use the Email keyboard layout.
    };

    enum EnterType  // The label on the Enter key.
    {
        DefaultEnter,
        Go,
        Join,
        Next,
        Search,
        Send,
        Submit,
        Done,
        Connect
    };

public:
    /**
     * instance
     *
     * Return the one and only instance of the Keyboard singleton.
     */
    static Keyboard &instance();

    /**
     * show
     *
     * Shows the keyboard using the current keyboard type and enter type.
     * When the keyboard has been shown, notifyVisibilityChanged will be emitted.
     *
     * Note: If you are using the BlackBerry lighthouse plugin, this function will
     *       be called for you for most keyboard enabled controls. You can override
     *       this behavior by filtering QEvent::RequestSoftwareInputPanel
     */
    void show();

    /**
     * hide
     *
     * Hides the keyboard.
     * When the keyboard has been hidden, notifyVisibilityChanged will be emitted.
     *
     * Note: If you are using the BlackBerry lighthouse plugin, this function will
     *       be called for you for most keyboard enabled controls. You can override
     *       this behavior by filtering QEvent::CloseSoftwareInputPanel
     */
    void hide();

    /**
     * isVisible
     *
     * Returns the current visibility state of the keyboard.
     */
    bool isVisible() const;

    /**
     * keyboardHeight
     *
     * Returns the current height of the keyboard as if it were visible (or
     * if it is currently visible).
     */
    int keyboardHeight() const;

    /**
     * setKeyboardType
     *
     * Sets the keyboard type that should be displayed next time the keyboard becomes
     * visible, or change the current type if it is already visible.
     */
    void setKeyboardType(KeyboardType keyboardType, EnterType enterType);

    /**
     * keyboardType
     *
     * Get the current keyboard type.
     */
    KeyboardType keyboardType() const;

    /**
     * enterType
     *
     * Get the current enter type.
     */
    EnterType enterType() const;

    /**
     * lauguageId
     *
     * Get the current keyboard language id.
     */
    QString languageId() const;

    /**
     * countryId
     *
     * Get the current keyboard country id.
     */
    QString countryId() const;

signals:
    /**
     * Emitted when the keyboard has become on/off screen. This can be the result of calling
     * show() or from the user manually showing the keyboard. If the KeyboardType or EnterType
     * are not set to default, the keyboard will be notified to change to that type (if the
     * application is currently visible).
     *
     * Note: If you are using the BlackBerry lighthouse plugin, it will automatically handle
     *       this signal to change the screen bounds.
     */
    void notifyVisibilityChanged(bool visible);

    /**
     * Emitted when the height of the keyboard has changed (usually due to
     * a device orientation change.
     *
     * Note: If you are using the BlackBerry lighthouse plugin, it will automatically handle
     *       this signal to change the screen bounds.
     */
    void notifyHeightChanged(int height);

private slots:
    void onWindowActivated(bool activated, Navigator::Display display);

private:
    Keyboard();
    virtual ~Keyboard();

    Keyboard(Keyboard const&);   // Don't implement
    void operator=(Keyboard const&); // Don't implement

    virtual void dataChanged(QByteArray &ppsData);
    void handlePPSMessage(PPSObject const& message);
    virtual void disconnected();
    void trySendKeyboardType();

    void handleShow();
    void handleHide();
    void handleInfo(QVariant const& properties);

    KeyboardType mKeyboardType;
    EnterType mEnterType;
    bool mIsVisible;
    int mKeyboardHeight;
    QString mLanguageId;
    QString mCountryId;
};

} //namespace BlackBerry

QT_END_NAMESPACE

#endif // KEYBOARD_H

