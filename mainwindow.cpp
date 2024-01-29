#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QScreen>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QMessageBox>
#include <QThread>
#include <sstream>

QStringList comPORTS;
bool com_available = false;
QString connectCOM = "COM14";
QString serialMonitor = "";

const int PROD_ID_TEENSY = 1155;
const int VENDOR_ID_TEENSY = 5824;

int SCREEN_WIDTH = 0;
int SCREEN_HEIGHT = 0;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Create a desktop widget to get screen information
    //QDesktopWidget *desktop = QApplication::desktop();

    // Get the available screens
    QList<QScreen*> screens = QGuiApplication::screens();

    qDebug() << "Number of Screens: " << screens.size();

    QScreen *screen = QGuiApplication::primaryScreen();
    QRect  screenGeometry = screen->geometry();
    SCREEN_HEIGHT = screenGeometry.height();
    SCREEN_WIDTH = screenGeometry.width();

    qDebug() << "(" << SCREEN_WIDTH << "x" << SCREEN_HEIGHT << ")";

    int x = SCREEN_WIDTH / 2;
    int y = SCREEN_HEIGHT / 2;

    // Move the mouse to a new position
    //QCursor::setPos(x,y);

    //init arduino serial port pointer
    arduino = new QSerialPort(this);
    // connect to the errorOccurred signal
    connect(arduino, &QSerialPort::errorOccurred, this, &MainWindow::serialError);
    loadCOMPorts();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::serialError(QSerialPort::SerialPortError error){
    if (error == QSerialPort::SerialPortError::ResourceError) {
        qDebug() << "Communication error occurred";
        //ui->lblStatus->setText("Not Connected");
        //loadCOMPorts();
        //ui->cmbCOM->clear();
        //ui->cmbCOM->addItem("Select a COM Port...");
        displayStandardMessageBox("Communication with Arduino has been dropped!");
        if (arduino->isOpen()){
            arduino->close();
        }
    }
}


void MainWindow::loadCOMPorts(){
    QString arduino_uno_port_name;
    //while (ui->cmbCOM->count() > 0){
    //ui->cmbCOM->removeItem(0);
    //}
    //

    comPORTS.clear();
    comPORTS.append("Select a COM Port...");

    qDebug() << "---COM PORTS---";
    //  For each available serial port
    QDebug noLine = qDebug();
    foreach(const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts()){
        comPORTS.append(serialPortInfo.portName());
        noLine << serialPortInfo.portName() << " ";
        if(serialPortInfo.hasProductIdentifier() && serialPortInfo.hasVendorIdentifier()){
            noLine << "Descr: " << serialPortInfo.description() << " ";
            noLine << "Manu: " << serialPortInfo.manufacturer() << " ";
            noLine << "ProdId: " << serialPortInfo.productIdentifier()  << " ";
            noLine << "VendorId: " << serialPortInfo.vendorIdentifier() << "\n";
            //set the teensy
            /*if (serialPortInfo.manufacturer() == "FTDI"){
                connectCOM = serialPortInfo.portName();
            }*/
        }
    }

    if (connectCOM != ""){
        initArduino(connectCOM);
    }

    //ui->cmbCOM->clear();
   // ui->cmbCOM->addItems(comPORTS);
}

void MainWindow::initArduino(QString port){
    if (arduino->isOpen()){
        arduino->close();
    }
    arduino->setPortName(port);
    if (!arduino->open(QSerialPort::ReadWrite)){
        QMessageBox::information(this, "Serial Port Error", "Could not open serial port");
        return;
    }
    arduino->setBaudRate(QSerialPort::Baud115200);
    arduino->setDataBits(QSerialPort::Data8);
    arduino->setFlowControl(QSerialPort::NoFlowControl);
    arduino->setParity(QSerialPort::NoParity);
    arduino->setStopBits(QSerialPort::OneStop);
    QObject::connect(arduino, SIGNAL(readyRead()), this, SLOT(readSerial()));


    qDebug() << "***FOR TESTING ONLY*** Connected to Prolific FTDI on " << port.toStdString();
    com_available = true;
}


void MainWindow::readSerial()
{
    /*
     * readyRead() doesn't guarantee that the entire message will be received all at once.
     * The message can arrive split into parts.  Need to buffer the serial data and then parse for the temperature value.
     *
     */

    //sometimes you get
    //<Hell
    //o58>
    //which means it doesnt read everything in one go

   serialData = arduino->readAll();

   //read through everything you have, start a new message at every <, parse the message as command at every >,
   //build up the string otherwise
   for(int i =0; i < serialData.size(); i++){
       if (serialData[i] == '<'){
        serialBuffer = "";
       } else if (serialData[i] == '>'){
           parseArduinoCmd(serialBuffer);
       } else {
           serialBuffer += serialData[i];
       }
   }

   //see all incoming data for testing
   //qDebug() << "REC: " << serialBuffer;


}


void MainWindow::parseArduinoCmd(QString cmd){

     qDebug() << "USB IN: " << cmd;
     serialMonitor += cmd + "\n";//QString::fromStdString(in) + "\n";
     //ui->txtSerialMonitor->setText(serialMonitor);
     //ui->txtSerialMonitor->verticalScrollBar()->setValue(ui->txtSerialMonitor->verticalScrollBar()->maximum());
    // ui->txtSerialMonitor->moveCursor(QTextCursor::End);
     //QString cmd = QString::fromStdString(in);
     QChar id = cmd.at(0);
     QString val = cmd.mid(1);
     if ( id == 'c') {
           // Split the string at the comma
           QStringList values = cmd.mid(1).split(',');

           // Convert the values to doubles
           bool conversionSuccess;
           double xPerc = values[0].toDouble(&conversionSuccess);
           double yPerc = values[1].toDouble(&conversionSuccess);

           if (conversionSuccess) {
               int x = xPerc * SCREEN_WIDTH;
               int y = yPerc * SCREEN_HEIGHT;
               moveMouse(x,y);

               // Now you have xValue and yValue as doubles
               qDebug() << "Command: 'c', X: " << x << ", Y: " << y;
           } else {
               qDebug() << "Error converting values to doubles.";
           }
     } else if (id == 'l'){
         //the mouse get's an up and a down
         if (cmd.length() != 2){
             qDebug() << "Error bad left click command";
             return;
         }
         QChar status = cmd.at(1);
         if (status == '1'){
             INPUT input = {};
             input.type = INPUT_MOUSE;
             input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
             SendInput(1, &input, sizeof(INPUT));
             qDebug() << "Left mouse press";
         } else {
             INPUT input = {};
             input.type = INPUT_MOUSE;
             input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
             SendInput(1, &input, sizeof(INPUT));
             qDebug() << "Left mouse release";
         }


     } else if (id == 'r'){
         qDebug() << "Right mouse click";
         mouseRightClick();
     } else if (id == 'w'){
        if (cmd.length() == 1){
            //this means only w so click
            mouseWheelClick();
        } else {
            QChar dir = cmd.at(1);
            mouseWheelScroll(dir);
        }
    } else if (id == 'p'){
        std::istringstream converter(val.toStdString());
        int result;
        converter >> std::hex >> result;


        //vk menu is alt
        if (result != VK_SHIFT && result != VK_MENU && result != VK_CONTROL){
            qDebug() << "KeyStroke: " << result;
            simulateKeyStroke(result);
        } else {
             qDebug() << "KeyPress: " << result;
            simulateKeyPress(result);

        }
     } else if (id == 'u'){
         std::istringstream converter(val.toStdString());
         int result;
         converter >> std::hex >> result;
         qDebug() << "KeyRelease: " << result;
         simulateKeyRelease(result);
     }

     else {
         qDebug() << "Invalid command. The first character should be 'c'.";
     }

}


//---------MOUSE CONTROL---------

void MainWindow::simulateMouseClick(DWORD dwFlags, DWORD dwFlag2){
    INPUT input = {};
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = dwFlags;
    SendInput(1, &input, sizeof(INPUT));
    QThread::msleep(1);
    input.mi.dwFlags = dwFlag2;
    SendInput(1, &input, sizeof(INPUT));
}
void MainWindow::mouseLeftClick(){
   simulateMouseClick(MOUSEEVENTF_LEFTDOWN, MOUSEEVENTF_LEFTUP);
}

void MainWindow::mouseRightClick(){
    simulateMouseClick(MOUSEEVENTF_RIGHTDOWN, MOUSEEVENTF_RIGHTUP);
}

void MainWindow::moveMouse(int x, int y){
    //QCursor::setPos(x,y);
    //clamp to screen
    x = std::max(0, std::min(x, SCREEN_WIDTH - 1));
    y = std::max(0, std::min(y, SCREEN_HEIGHT - 1));
    SetCursorPos(x,y);
}

void MainWindow::mouseWheelClick(){
    simulateMouseClick(MOUSEEVENTF_MIDDLEDOWN,MOUSEEVENTF_MIDDLEUP);
}

void MainWindow::mouseWheelScroll(QChar dir){
   INPUT input = {};
   input.type = INPUT_MOUSE;
   input.mi.dwFlags = MOUSEEVENTF_WHEEL;  // Specify a wheel event
   if (dir == '1'){
        input.mi.mouseData = WHEEL_DELTA;
        qDebug() << "Wheel scroll up";
   } else {
       input.mi.mouseData = -WHEEL_DELTA;
       qDebug() << "Wheel scroll down";
   }
   SendInput(1, &input, sizeof(INPUT));
}

void MainWindow::displayStandardMessageBox(QString msg){
    QMessageBox msgBox(this);
    msgBox.setText(msg);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.exec();
}


void MainWindow::simulateKeyPress(WORD key)
{
    INPUT input = {};
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = key;
    SendInput(1, &input, sizeof(INPUT));
}

void MainWindow::simulateKeyRelease(WORD key)
{
    INPUT input = {};
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = key;
    input.ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(1, &input, sizeof(INPUT));
}
void MainWindow::simulateKeyStroke(WORD key)
{
    simulateKeyPress(key);
    Sleep(20);
    simulateKeyRelease(key);
}

void MainWindow::simulateMouseClick(DWORD dwFlags)
{
    INPUT input = {};
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = dwFlags;
    SendInput(1, &input, sizeof(INPUT));
}


