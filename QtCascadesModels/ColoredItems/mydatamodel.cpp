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

#include <QList>
#include <QStringList>
// needed for the model data with all named colors in Qt
#include <QColor>

#include "mydatamodel.h"
#include "mydataobject.h"

MyDataModel::MyDataModel(QObject *parent) :
    QObject(parent)
{
// Creates a data set of all named colors in Qt
// This colors are not avaialble in Cascades UI
// We make them available via this C++ backend
    foreach (QString currentColorName, QColor::colorNames()) {
        QColor currentColor (currentColorName);
        m_data.append(new MyDataObject ("\"" + currentColorName + "\" is in hex " + currentColor.name(), currentColor.name()));
    }

// An alternative, simple data set with three manually created items
// Boring, but clear... :-)
//   m_data.append(new MyDataObject( "White Item", "#FFFFFF"));
//   m_data.append(new MyDataObject( "Blue Item", "#0000FF"));
//   m_data.append(new MyDataObject( "Green Item", "#00FF00"));
//   m_data.append(new MyDataObject( "Red Item", "#FF0000"));
}

MyDataModel::~MyDataModel() {

}

QList<QObject*> MyDataModel::data() {
    return m_data;
}
