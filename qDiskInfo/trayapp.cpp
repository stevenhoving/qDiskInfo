#include "trayapp.h"
#include <QCoreApplication>
#include <QPainter>
#include <QPixmap>
#include <QFont>
#include <QString>
#include <QAction>
#include <QMenu>
#include <qpainterpath>

TrayApp::TrayApp(ITemperature* temperature, TrayHandler& trayHandler, QObject* parent)
    : QObject(parent)
    , trayIconMenu(CreateTrayMenu())
    , temperatureProvider(temperature)
    , trayHandler(trayHandler)
{
    trayIcon.setContextMenu(trayIconMenu);
    trayIcon.setVisible(true);

    connect(&timer, &QTimer::timeout, this, &TrayApp::updateTemperatureIcon);
    timer.setInterval(1000); // 1 second
    updateTemperatureIcon();

    QObject::connect(&trayIcon, &QSystemTrayIcon::activated,
                     [&](QSystemTrayIcon::ActivationReason reason)
                     {
                         if (reason == QSystemTrayIcon::Trigger)
                         {
                             qDebug() << "Tray icon clicked";
                             emit trayHandler.trayClicked();
                         }
                     });
}

void TrayApp::start()
{
    timer.start();
    updateTemperatureIcon();
}

void TrayApp::updateTemperatureIcon()
{
    int temp = temperatureProvider->GetTemperature();
    if (temp == lastTemperature)
    {
        return; // No change in temperature
    }

    lastTemperature = temp;

    trayIcon.setIcon(createIconWithTemperature(temp));
    trayIcon.setToolTip(QString("Current Temperature: %1°C").arg(temp));
}

QMenu* TrayApp::CreateTrayMenu()
{
    auto menu = new QMenu();
    quitAction = new QAction(tr("&Quit"), this);
    connect(quitAction, &QAction::triggered, [this] { qApp->quit(); });
    menu->addAction(quitAction);

    return menu;
}

QIcon TrayApp::createIconWithTemperature(int temp)
{
    // 48px seems to be the default tray icon size for windows 11.
    // - can we lookup the correct size somehow?
    // 
    //const int iconSize = 64;
    const int iconSize = 48;
    
    QPixmap pixmap(iconSize, iconSize);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    QFont font = painter.font();
    font.setBold(true);
    font.setPointSize(42);

    painter.setFont(font);
    //painter.setPen(Qt::white);
    painter.setPen(Qt::black);

    painter.drawText(pixmap.rect(), Qt::AlignCenter, QString::number(temp));
    painter.end();

    return QIcon(pixmap);
}
