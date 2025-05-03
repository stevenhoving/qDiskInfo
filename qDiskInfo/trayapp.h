#pragma once

#include "temperature_provider.h"
#include "trayhandler.h"
#include <QObject>
#include <QSystemTrayIcon>
#include <QTimer>
#include <QAction>

class TrayApp : public QObject
{
    Q_OBJECT

public:
    TrayApp(ITemperature* temperature, TrayHandler& trayHandler, QObject* parent = nullptr);
    void start();

private slots:
    void updateTemperatureIcon();

private:
    QMenu*CreateTrayMenu();

    QAction* quitAction;
    QMenu* trayIconMenu;
    QSystemTrayIcon trayIcon;

    QTimer timer;
    ITemperature* temperatureProvider;
    int lastTemperature = 0;

    QIcon createIconWithTemperature(int temp);

    TrayHandler& trayHandler;
};
