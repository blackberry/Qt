/*
* Copyright (c) 2011-2013 Research In Motion Limited.
* Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/ 

#ifndef MYDATAOBJECT_H
#define MYDATAOBJECT_H

#include <QObject>

class MyDataObject : public QObject
{
    Q_OBJECT
    // Two properties, name and color, that allow access from QML side (through Meta-Object)
    Q_PROPERTY( QString name READ name WRITE setName NOTIFY nameChanged )
    Q_PROPERTY( QString color READ color WRITE setColor NOTIFY colorChanged )

public:
    MyDataObject(QString name, QString color, QObject* parent = 0);

    QString name() const;
    QString color() const;

public slots:
    void setName( QString name );
    void setColor( QString color );

signals:
    void nameChanged();
    void colorChanged();

private:
    QString m_name;
    QString m_color;

};

#endif // MYDATAOBJECT_H
