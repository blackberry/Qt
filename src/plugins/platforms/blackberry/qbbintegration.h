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

#ifndef QBBINTEGRATION_H
#define QBBINTEGRATION_H

#include <QtGui/QPlatformIntegration>

#include <screen/screen.h>

QT_BEGIN_NAMESPACE

class QBBEventThread;

class QBBIntegration : public QObject,
                       public QPlatformIntegration
{

    Q_OBJECT

public:
    QBBIntegration();
    virtual ~QBBIntegration();

    virtual bool hasCapability(QPlatformIntegration::Capability cap) const;

    virtual QPixmapData *createPixmapData(QPixmapData::PixelType type) const;
    virtual QPlatformWindow *createPlatformWindow(QWidget *widget, WId winId) const;
    virtual QWindowSurface *createWindowSurface(QWidget *widget, WId winId) const;

    virtual QList<QPlatformScreen *> screens() const;
    virtual void moveToScreen(QWidget *window, int screen);

    virtual QPlatformFontDatabase *fontDatabase() const { return mFontDb; }

#ifndef QT_NO_CLIPBOARD
    virtual QPlatformClipboard *clipboard() const;
#endif

private slots:
    void onStartOrientation(int angle, QString const& id);
    void onEndOrientation(QString const& id);
    void onSwipeDown();
    void onExit();
    void onKeyboardVisibilityChanged(bool visibility);
    void onKeyboardHeightChanged(int height);

private:
    screen_context_t mContext;
    QBBEventThread *mEventThread;
    QPlatformFontDatabase *mFontDb;
};

QT_END_NAMESPACE

#endif // QBBINTEGRATION_H
