#include "mainwindow.h"
#include <QSystemTrayIcon>
#include <QMenu>
#include <QApplication>

//https://amin-ahmadi.com/2019/06/20/implementing-minimize-to-tray-functionality-using-qt-c/

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    //w.move(10,10);
    //w.show();
    // Create a system tray icon with a default icon
        QSystemTrayIcon trayIcon(QIcon(":/icons/icon.png"));  // Using a default smiley face icon
        trayIcon.setVisible(true);
        trayIcon.show();

        // Create a context menu for the system tray icon
        QMenu menu;
        QAction actionSettings("Settings");
        QAction actionExit("Exit");
        menu.addAction(&actionSettings);
        menu.addAction(&actionExit);

        // Connect the exit action to the application exit slot
        QObject::connect(&actionExit, &QAction::triggered, [&]() {
            QApplication::quit();
        });

        // Connect the exit action to the application exit slot
        QObject::connect(&actionSettings, &QAction::triggered, [&]() {
            qDebug() << "Clicked settings";
        });

        // Set the context menu for the system tray icon
        trayIcon.setContextMenu(&menu);
    return a.exec();
}
