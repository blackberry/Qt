#include <QApplication>
#include "qmlapplicationviewer.h"
#include <QDeclarativeContext>
#include "clientbackend.h"

Q_DECL_EXPORT int main(int argc, char *argv[])
{
    QScopedPointer<QApplication> app(createApplication(argc, argv));

    QmlApplicationViewer viewer;
    viewer.addImportPath(QLatin1String("modules"));
    ClientBackend client;
    viewer.rootContext()->setContextProperty("socket", &client);
    viewer.setOrientation(QmlApplicationViewer::ScreenOrientationAuto);
    viewer.setMainQmlFile(QLatin1String("qml/SocketClient/main.qml"));
    viewer.showExpanded();

    return app->exec();
}
