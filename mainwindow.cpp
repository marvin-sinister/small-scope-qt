/****************************************************************************
**
** Copyright (C) 2015 Marvin Sinister <marvin@sinister.io>
**
** This file is part of small-scope-qt.
**
** small-scope-qt is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** small-scope-qt is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with small-scope-qt.  If not, see <http://www.gnu.org/licenses/>.
**
****************************************************************************/

/****************************************************************************
icons:
start/stop: http://www.virtuallnk.com/
connect/disconnect: http://p.yusukekamiyamane.com/
****************************************************************************/

#include <QTimer>
#include <QTime>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "settingsdialog.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        qDebug() << "Name        : " << info.portName();
    }

    // create settings dialog
    settings = new SettingsDialog;
    // create about window
    aboutDialog = new About;
    // create serial
    serial = new QSerialPort(this);
    // create buffer for serial communication
    buffer = new unsigned char[Settings::bufferSize];
    // clear buffer
    memset(buffer, 0, Settings::bufferSize);
    // initialize
    bufferFilled = 0;

    // time for random seed
    //QTime time = QTime::currentTime();
    // initialize random seed
    //qsrand((uint)time.msec());

    //QTimer *timer = new QTimer(this);
    //connect(timer, SIGNAL(timeout()), this, SLOT(immaginaryData()));
    //timer->start(10000);

    // initialize
    cursorPosition = 0;
    measure1Position = 0;
    measure2Position = 0;

    // set cursor values when mouse moves
    connect(ui->renderArea, SIGNAL(mouseMoved(int)), this, SLOT(setValuesCursor(int)));
    // set measure values on mouse click
    connect(ui->renderArea, SIGNAL(lPositionChanged(int)), this, SLOT(setValues1(int)));
    connect(ui->renderArea, SIGNAL(rPositionChanged(int)), this, SLOT(setValues2(int)));
    // update title with new fps value
    connect(ui->renderArea, SIGNAL(fpsChanged(float)), this, SLOT(setTitle(float)));

    // recalculate all values measured
    connect(this, SIGNAL(valuesChanged()), this, SLOT(recalculateValues()));
    connect(ui->renderArea, SIGNAL(recalculateValues()), this, SLOT(recalculateValues()));


    // serial port error
    connect(serial, SIGNAL(error(QSerialPort::SerialPortError)), this, SLOT(handleError(QSerialPort::SerialPortError)));

    // read data from serial port when ready
    connect(serial, SIGNAL(readyRead()), this, SLOT(readData()));

    // resize renderArea to buffer size
    ui->renderArea->setMinimumSize(Settings::bufferSize,256*Settings::heightConstant);
    ui->renderArea->setMaximumSize(Settings::bufferSize,256*Settings::heightConstant);
    ui->renderArea->resize(Settings::bufferSize,256*Settings::heightConstant);

    // resize window to minimum size
    resize(minimumWidth(), minimumHeight());
}

MainWindow::~MainWindow() {
    delete ui;
}

// set the prescaler value of the scope
void MainWindow::setPrescalerValue(int value) {
    // calculate the actual value
    value = (int)pow(2, value);

    // save to global variable
    Settings::prescaler = value;

    // update form field
    ui->sbPrescaler->setValue(value);

    // send over serial
    QByteArray d;
    d = QByteArray("p",1);
    d.append(QByteArray::number(value, 10 ));
    writeData(d);
}

// set the trigger level of the scope
void MainWindow::setTriggerLevel(int value) {
    // save to global variable
    Settings::triggerLevel = value;

    // update form field
    ui->dsbThreshold->setValue(value*Settings::VPerUnit);

    // send over serial
    QByteArray d;
    d = QByteArray("t",1);
    d.append(QByteArray::number(value, 10 ));
    writeData(d);
}

// set the holdoff value of the scope
// holdoff is the time recorded after trigger
void MainWindow::setHoldoffValue (int value) {
    // save to global variable
    Settings::holdoff = value;

    // send over serial
    QByteArray d;
    d = QByteArray("w",1);
    d.append(QByteArray::number(value, 10 ));
    writeData(d);
}

// recalculate all measured values
void MainWindow::recalculateValues()
{
    // voltage at cursor
    float voltageC = buffer[cursorPosition] * 0.02;
    ui->leVoltage->setText(QString::number(voltageC,'f',2));
    // voltage at first measure
    float voltage1 = buffer[measure1Position] * 0.02;
    ui->leVoltage1->setText(QString::number(voltage1,'f',2));
    // voltage at second measure
    float voltage2 = buffer[measure2Position] * 0.02;
    ui->leVoltage2->setText(QString::number(voltage2,'f',2));
    // time at cursor
    float timeC = ((cursorPosition- Settings::bufferSize/2)/Settings::prescalerConstant)*Settings::prescaler;
    ui->leTime->setText(QString::number(timeC,'f',2));
    // time at first measure
    float time1 = ((measure1Position - Settings::bufferSize/2)/Settings::prescalerConstant)*Settings::prescaler;
    ui->leTime1->setText(QString::number(time1,'f',2));
    //time at second measure
    float time2 = ((measure2Position - Settings::bufferSize/2)/Settings::prescalerConstant)*Settings::prescaler;
    ui->leTime2->setText(QString::number(time2,'f',2));

    // absolute voltage and time difference
    ui->leDiffV21->setText(QString::number(fabs(voltage2 - voltage1),'f',2));
    ui->leDiffVc1->setText(QString::number(fabs(voltageC - voltage1),'f',2));
    ui->leDiffVc2->setText(QString::number(fabs(voltageC - voltage2),'f',2));
    ui->leDifft21->setText(QString::number(fabs(time2 - time1),'f',2));
    ui->leDifftc1->setText(QString::number(fabs(timeC - time1),'f',2));
    ui->leDifftc2->setText(QString::number(fabs(timeC - time2),'f',2));

    // holdoff time
    ui->dsbHoldoff->setValue(Settings::holdoff/Settings::prescalerConstant*Settings::prescaler);

    // threshold level
    ui->dsbThreshold->setValue(Settings::triggerLevel*Settings::VPerUnit);

}

// set trigger event to toggle
void MainWindow::setTriggerEventToggle() {
    writeData(QByteArray("e0",2));
}

// set the trigger event to falling
void MainWindow::setTriggerEventFalling() {
    writeData(QByteArray("e2",2));
}

// set the trigger event to rising
void MainWindow::setTriggerEventRising() {
    writeData(QByteArray("e3",2));
}

// ignore the trigger and run in continuous mode
void MainWindow::setTriggerEventNone() {
    writeData(QByteArray("e4",2));
}

// set the voltage refference to external refference
void MainWindow::setVRefAREF() {
    writeData(QByteArray("r0",2));
}

// set the voltage refference to internal supply refference
void MainWindow::setVRefAVCC() {
    writeData(QByteArray("r1",2));
}

// set the voltage refference to internal 1.1V refference
void MainWindow::setVRefInternal() {
    writeData(QByteArray("r3",2));
}

// set the cursor position
void MainWindow::setValuesCursor(int value) {
    cursorPosition = value;
    emit recalculateValues();
}

// set the first measure position
void MainWindow::setValues1 (int value) {
    measure1Position = value;
    emit recalculateValues();
}

// set the second measure position
void MainWindow::setValues2 (int value) {
    measure2Position = value;
    emit recalculateValues();
}

// change the window title
void MainWindow::setTitle(float fps) {
    this->setWindowTitle ("small-scope-qt - " + QString::number(fps) + "fps");
}

// show the COM port configuration window
void MainWindow::setupCOM() {
    settings->show();
}

// start the data aquisition
void MainWindow::startSampling() {
    QByteArray d;
    d = QByteArray("s", 1);
    writeData(d);
}

// stop the data aquisition
void MainWindow::stopSampling(){
    QByteArray d;
    d = QByteArray("S", 1);
    writeData(d);
    bufferFilled = 0;
}

// connect to scope over COM port wrapper
void MainWindow::connectCOM() {
    qDebug() << "Connecting to serial.";
    openSerialPort();
    if (serial->isOpen ()) {
        qDebug() << "Connected";
    }
    else {
        qDebug() << "Failed";
    }
}

// disconnect the COM port wrapper
void MainWindow::disconnectCOM() {
    closeSerialPort();
    qDebug() << "Disconnected";
}

// ask the scope for it's status
void MainWindow::status() {
    QByteArray d;
    d = QByteArray("d", 1);
    writeData(d);
}

// connect the COM port
void MainWindow::openSerialPort() {
    // read the settings
    SettingsDialog::Settings p = settings->settings();
    // set port name
    serial->setPortName(p.name);
    // try to connect
    if (serial->open(QIODevice::ReadWrite)) {
        if (serial->setBaudRate(p.baudRate)
                && serial->setDataBits(p.dataBits)
                && serial->setParity(p.parity)
                && serial->setStopBits(p.stopBits)
                && serial->setFlowControl(p.flowControl)) {        }
        else {
            serial->close();
            qDebug() << "Error" << serial->errorString();
        }
    }
    else {
        qDebug() << "Error" << serial->errorString();
    }
}

// disconnect the com port
void MainWindow::closeSerialPort() {
    serial->close();
}

// show the about window
void MainWindow::about() {

}

// write data to serial port
void MainWindow::writeData(const QByteArray &data) {
    if (serial->isOpen ()) {
        serial->write(data);
    }
    qDebug() << "Writing to serial: " << data;
}

// read data from serial port
void MainWindow::readData() {
    // read the data from serial
    QByteArray data = serial->readAll();

    // if there is more data than there is space in buffer, start from the beggining of buffer
    if ((bufferFilled + data.size()) > Settings::bufferSize) {
        bufferFilled = 0;
    }

    // read the data to buffer
    for (int i = 0; i < data.size(); ++i) {
        buffer[bufferFilled + i] = data[i];
    }

    // increase the filled amount
    bufferFilled+=data.size();

    // render the data on form
    ui->renderArea->drawData(buffer);
}

// generate random data for testing
/*void MainWindow::immaginaryData() {
    for (int i = 0; i < BUFFER_SIZE; ++i) {
        buffer[i] = (unsigned char)((qrand() % 256));
    }
    ui->renderArea->drawData(buffer);
}*/

// handle serial port error
void MainWindow::handleError(QSerialPort::SerialPortError error) {
    if (error == QSerialPort::ResourceError) {
        qDebug() << "Critical Error" << serial->errorString();
        closeSerialPort();
    }
}

// exit action
void MainWindow::on_actionExit_triggered() {
    close();
}

// about action
void MainWindow::on_actionAbout_triggered() {
    aboutDialog->show();
}

// COM settings action
void MainWindow::on_actionCOM_Port_Settings_triggered() {
    setupCOM();
}
