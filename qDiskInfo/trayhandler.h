#pragma once

#include <QObject>

// just a hack so we can hookup the qt widgets tray to the qml flyout trigger
class TrayHandler : public QObject
{
    Q_OBJECT
public:
    explicit TrayHandler(QObject* parent = nullptr)
        : QObject(parent)
    {
    }

signals:
    void trayClicked();
};
