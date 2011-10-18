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


#include <QtGui/QPlatformIntegrationPlugin>
#include "qbbintegration.h"

QT_BEGIN_NAMESPACE

class QBBIntegrationPlugin : public QPlatformIntegrationPlugin
{
public:
    QStringList keys() const;
    QPlatformIntegration *create(const QString&, const QStringList&);
};

QStringList QBBIntegrationPlugin::keys() const
{
    QStringList list;
    list << "blackberry";
    return list;
}

QPlatformIntegration *QBBIntegrationPlugin::create(const QString& system, const QStringList& paramList)
{
    Q_UNUSED(paramList);
    if (system.toLower() == "blackberry")
        return new QBBIntegration;

    return 0;
}

Q_EXPORT_PLUGIN2(blackberry, QBBIntegrationPlugin)

QT_END_NAMESPACE
