#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "trayapp.h"
#include "temperature_provider.h"


int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QApplication::setQuitOnLastWindowClosed(false);

    QQmlApplicationEngine engine;

    TrayHandler trayHandler;
    engine.rootContext()->setContextProperty("trayHandler", &trayHandler);

    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    DummyTemperature tempProvider;
    //SmartTemperature tempProvider(0);
    TrayApp trayApp(&tempProvider, trayHandler);
    trayApp.start();

    return app.exec();
}
