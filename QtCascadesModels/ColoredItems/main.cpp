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

#include <QApplication>
#include <QDeclarativeView>
#include <QDeclarativeContext>

#include "mydatamodel.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QDeclarativeView view;

    // Create the model
    MyDataModel *myDataModel = new MyDataModel(view.rootContext());

    // The dataModel needs to added before loading the QML file
    view.rootContext()->setContextProperty("dataModel", QVariant::fromValue(myDataModel->data()) );
    view.setResizeMode(QDeclarativeView::SizeRootObjectToView);

    view.setSource(QUrl("qrc:///main.qml"));

    view.show();

    return a.exec();
}
