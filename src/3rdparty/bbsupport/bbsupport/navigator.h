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

#ifndef NAVIGATOR_H
#define NAVIGATOR_H

#include <bbsupport/PPSObject>
#include <bbsupport/PPSFile>

#include <QFileInfo>
#include <QObject>
#include <QString>

QT_BEGIN_NAMESPACE

namespace BlackBerry
{

/**
 * Singleton class that wraps the connection to the Navigator pps object.
 *
 * The usage of this class is shared by applications and the libblackberry lighthouse
 * plugin. Applications should communicate with the navigator only through this class
 * and MUST NEVER open an additional connection to the navigator.
 */
class Navigator : public PPSFile
{
     Q_OBJECT

public:
    enum WindowState
    {
        FullScreen,
        Thumbnail,
        Invisible
    };

    enum Display
    {
        Primary,
        Secondary
    };

public:
    static Navigator& instance();

    /**
     * launchUrl
     *
     * Applications should send this message to the Navigator when it wants to open
     * another application that can handle the URL data.
     *
     * Note: This message is called invoke by the navigator documentation.
     *
     * url can be encoded in any way that the receiving application expects.
     */
    void launchUrl(QString const& url);

    /**
     * openFile
     *
     * Applications should send this message to the Navigator when it wants to open
     * a file using the default handler.
     */
    void openFile(QFileInfo const& file);

    /**
     * enableSwipeStart
     *
     * Applications can send this message if the standard notifySwipeDown signal does not
     * provide enough functionality for the application. If the app sends this message
     * to the Navigator it will no longer receive a notifySwipeDown signal and instead will
     * receive a notifySwipeStart signal followed by mouse pointer events.
     *
     * Note: This message is called SWIPE_START in the navigator documentation.
     *
     * After this mode has been enabled it will remain enabled until the application
     * exits. There is no way to clear this mode.
     */
    void enableSwipeStart();

    /**
     * addUri
     *
     * This command will create an icon on in the Navigator trays that can be used to launch
     * an application via the URL field. If the default_category exists, the icon will be
     * added to the category, otherwise it will only show in All. No validation of the url
     * is done.
     */
    void addURI(QString const& URL, QFileInfo const& iconPath, QString const& label, QString const& defaultCategory);

    /**
     * setOrientation
     *
     * This message allows the application to request the device be rotated to the
     * specified orientation.
     *
     * Note: The navigator message is documented as "orientation"
     *
     * Valid orientations are 0, 90, 180, 270.
     */
    void setOrientation(int orientation);

    /**
     * lockOrientation
     *
     * This message instructs the Navigator to prevent rotation of the application regardless
     * of it's orientation settings. It only affects the application when it's the foreground
     * full screen application. The state persists until unlocked, so if the app is minimized
     * and then selected again, it will remain locked in whatever the current orientation is
     * set to.
     */
    void lockOrientation();

    /**
     * unlockOrientation
     *
     * This message removes the rotation lock created by the previous message above.
     */
    void unlockOrientation();

    /**
     * isOrientationLocked
     *
     * Returns true if the orientation is currently locked.bool activated, Display display
     **/
    bool isOrientationLocked() const;

    /**
     * isReorienting
     *
     * Returns true if the navigator is currently reorienting the application.
     */
    bool isReorienting() const;

    /**
     * addSplat
     *
     * This message is used to place a splat (red star) on the application's icon and window
     * frame. Generic badges are not supported yet. Use the removeBadge function to clear
     * the splat.
     *
     * Note that this function will send the addBadge message to the navigator with the
     * parameter "splat", which is the only supported parameter for add badge at this time.
     */
    void addSplat();

    /**isOrientationLocked()
     * removeBadge
     *
     * This function will remove a badge from the application's icon and window frame.
     */
    void removeBadge();

    /**
     * extendTimeout
     *
     * Normally, applications get 30 seconds to create their application window. If an
     * application is unable to create windows in this time frame due to initialization
     * or loading issues, it must request and extension to the normal timeout, otherwise
     * it will be terminated. The timeout value is increased from 30 seconds by the
     * specified amount in milliseconds.
     */
    void extendTimeout(unsigned int milliseconds);

    /**
     * setWindowAngle
     *
     * Some applications do their rotation internally and don't use the standard Navigator
     * method. In this case the application should maintain a SCREEN_PROPERTY_ROTATION
     * value of 0, do their rotations internally and report back the angle of rotation to
     * the Navigator so that they remain in sync.
     *
     * Valid orientations are 0, 90, 180, 270.
     */
    void setWindowAngle(int angle);

    /**
     * setWindowLabel
     *
     * This message allows an application to change it's icon/window label. Currently,
     * this message is best for hidden applications to change their window label since
     * the icon label is not seen. The change is not persistent across reboots.
     */
    void setWindowLabel(QString const& labeldispatchEndOrientation);

    /**
     * setClosePrompt
     *
     * This message allows an application to prevent the user from closing the application
     * without warning. If the user tries to close the application, a dialog will be
     * displayed with the title and message specified. The user will have 2 buttons:
     * "Cancel" and "Close". If the user selects Close, the application will receive
     * an exit signal. If they select Cancel, the dialog will close and the application
     * will continue running.
     */
    void setClosePrompt(QString const& title, QString const& message);

    /**
     * clearClosePrompt
     *
     * Removes a close prompt that was set with setClosePrompt.
     *
     * Note: PPS Documentation indicates this calls closePrompt with empty strings for title
     *       and message. You should not do this as it might change in future. Always use
     *       clearClosePrompt to remove the prompt.
     */
    void clearClosePrompt();

    /**
     * sendBufferResized
     *
     * This function *must* be called *after* the buffer has been resized and painted as a
     * response to an orientation change initiated by notifyStartOrientation, even if 
     * the buffer size did not need to be changed (180 degree rotation). This indicates 
     * to the navigator that it can start playing the transition animation. If you do not
     * respond, the navigator will play the animation anyway.
     *
     * NOTE: You should not call this function if you are using the BlackBerry lighthouse
     *       plugin as it will handle this for you.
     */
    void sendBufferResized(QString const& id);

    /**
     * setGroupIds
     *
     * Specify the group IDs that represent the main display and external displays.
     *
     * NOTE: You should not call this function if you are using the BlackBerry lighthouse
     *       plugin as it will handle this for you.
     */
    void setGroupIds(QString const& primaryGroupId, QString const& secondaryGroupId);

    /**
     * windowState
     *
     * Get the current window state of the group on the specified display.
     */
    WindowState windowState(Display display) const;

    /**
     * isActive
     *
     * Get the current active state of the specified display.
     */
    bool isActivated(Display display) const;

signals:
    /**
     * notifyStartOrientation
     *
     * This signal is emitted when the application should rotate. This indicates the
     * application must resize its buffers (e.g. 1024x600 -> 600x1024), paint the
     * application window and THEN MUST call sendBufferResized to indicate that it has
     * been completed. The application should not post updates to the window until the
     * notifyEndOrientation signal is emitted.
     */
    void notifyStartOrientation(int angle, QString const& id);

    /**
     * notifyEndOrientation
     *
     * This signal is emitted when the orientation transition animation has been completed.
     */
    void notifyEndOrientation(QString const& id);

    /**
     * notifyWindowStateChanged
     *
     * This signal is emitted whenever the window state has changed.
     * 
     * The FullScreen state is set when the application is considered fullscreen and should
     * be running normally. The Thumbnail state is sent when the application has been sized
     * smaller than fullscreen. The Invisible state is sent when the application's window
     * is not visible to the user. This could be due to being positioned off the screen or
     * when LCD is turned off. For HDMI windows, Fullscreen indicates that the application's
     * HDMI window is the current owner of the HDMI display and Invisible means that it is
     * currently not displayed on the HDMI monitor. In both Invisible and Thumbnail
     * states, applications should checkpoint or save their state as they could be terminated
     * at any time. The groupid can be used to determine which window is being updated if the
     * application has more than one window (i.e. HDMI). 
     */
    void notifyWindowStateChanged(WindowState state, Navigator::Display display);

    /**
     * notifyWindowActived
     *
     * This signal is emitted to an application when its activation state has changed. An
     * application can be active while in any windowState mode depending on the Application
     * Behaviour setting.
     *
     * When an application is inactive it should not update it's display but could continue
     * any background activities or audio playback.
     */
    void notifyWindowActivated(bool activated, Navigator::Display display);

    /**
     * notifySwipeDown
     *
     * This signal is emitted when the user swipes down from the top edge of the device into
     * the application area.
     *
     * If you are using the BlackBerry lighthouse plugin, a Qt::Key_Menu key press event
     * will also be injected in response to this signal.
     */
    void notifySwipeDown();

    /**
     * notifySwipeStart
     *
     * This signal is emitted if the Application has request more control of the swipe down
     * process (by calling enableSwipeStart). Once the application receives this event it
     * should start listening for mouse events to control the swipe down process live.
     */
    void notifySwipeStart();

    /**
     * notifyInvoke
     *
     * This signal is emitted to the application if it is registered as an URL/EXT handler.
     * The application should use the data to update its view accordingly.
     */
    void notifyInvoke(QString const& url);

    /**
     * notifyBack
     *
     * This signal is emitted to the application when the user has initiated the system "back"
     * gesture.
     */
    void notifyBack();

    /**
     * notifyExit
     *
     * This signal is emitted to the application when the User has closed it via the
     * Navigator. Currently, the application has 2 seconds to exit normally otherwise
     * the Navigator will terminate the application
     */
    void notifyExit();

    /**
     * notifyLowMemory
     *
     * This signal is emitted when the system is running low on memory. Applications should
     * free any memory they are not using. Applications could be closed if memory is
     * not released.
     */
    void notifyLowMemory();

    /**
     * notifyNavigatorDisconnected
     *
     * This signal is emitted whenever the navigator has been disconnected due to the
     * object being closed. If this happens before your application has exited it could
     * signify a problem.
     */
    void notifyNavigatorDisconnected();

private:
    void handleOrientationCheck(QString const& angle, QString const& id);
    void handleStartOrientation(QString const& angle, QString const& id);
    void handleEndOrientation(QString const& id);
    void handleWindowStateChanged(QString const& newState);
    void handleWindowActivated(QString const& groupId, bool activated);

private:
    Navigator();
    virtual ~Navigator();

    virtual void dataChanged(QByteArray &ppsData);
    void handlePPSMessage(PPSObject const& message);
    virtual void disconnected();

    Navigator(Navigator const&);   // Don't implement
    void operator=(Navigator const&); // Don't implement

    void* mPrivate;
};

} //namespace BlackBerry

QT_END_NAMESPACE

#endif // NAVIGATOR_H
