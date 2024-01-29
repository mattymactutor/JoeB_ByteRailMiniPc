#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QMouseEvent>
#include <windows.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    //---Arduino Communication----
    QSerialPort *arduino;
    QByteArray serialData;
    QString serialBuffer;
    void initArduino(QString port);
    void parseArduinoCmd(QString in);
    void serialError(QSerialPort::SerialPortError error);
    void loadCOMPorts();
    //----MOUSE CONTROL---
    void moveMouse(int x, int y);
    void simulateMouseClick(DWORD dwFlags,DWORD dwFlag2);
    void mouseLeftClick();
    void mouseRightClick();
    void mouseWheelClick();
    void mouseWheelScroll(QChar dir);
    //Keyboard control
    void simulateKeyPress(WORD key);
    void simulateKeyRelease(WORD key);
    void simulateKeyStroke(WORD key);
    void simulateMouseClick(DWORD dwFlags);

    void displayStandardMessageBox(QString msg);

private slots:
    void readSerial();
};
#endif // MAINWINDOW_H
