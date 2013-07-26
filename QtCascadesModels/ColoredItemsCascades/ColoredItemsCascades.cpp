/*
* Copyright (c) 2011-2013 Research In Motion Limited.
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

#include <bb/cascades/Application>
#include <bb/cascades/QmlDocument>
#include <bb/cascades/AbstractPane>
#include <bb/cascades/QListDataModel>

#include "ColoredItemsCascades.hpp"
#include "../ColoredItems/mydatamodel.h"

using namespace bb::cascades;

ColoredItemsCascades::ColoredItemsCascades(bb::cascades::Application *app)
    : QObject(app)
{

    // create a new model in the C++ backend
    MyDataModel *myDataModel = new MyDataModel(app);
    // create a data model for Cascades UI
    QListDataModel<QObject*> *cascadesModel = new QListDataModel<QObject*>(myDataModel->data());

    // load QML
    QmlDocument *qml = QmlDocument::create("asset:///main.qml").parent(this);
    // make the data model availavle via a property in the context of the loaded QML file
    qml->documentContext()->setContextProperty("myCasadesDataModel", cascadesModel);

    AbstractPane *root = qml->createRootObject<AbstractPane>();
    app->setScene(root);
}
