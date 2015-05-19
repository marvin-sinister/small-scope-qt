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

    settings = new SettingsDialog;
    aboutDialog = new About;
    serial = new QSerialPort(this);
    buffer = new unsigned char[Settings::bufferSize];
    memset(buffer, 0, Settings::bufferSize);
    bufferFilled = 0;

    QTime time = QTime::currentTime();
    qsrand((uint)time.msec());

    //QTimer *timer = new QTimer(this);
    //connect(timer, SIGNAL(timeout()), this, SLOT(immaginaryData()));
    //timer->start(10000);

    cursorPosition = 0;
    measure1Position = 0;
    measure2Position = 0;

    connect(ui->renderArea, SIGNAL(mouseMoved(int)), this, SLOT(setValuesCursor(int)));
    connect(ui->renderArea, SIGNAL(lPositionChanged(int)), this, SLOT(setValues1(int)));
    connect(ui->renderArea, SIGNAL(rPositionChanged(int)), this, SLOT(setValues2(int)));
    connect(ui->renderArea, SIGNAL(fpsChanged(float)), this, SLOT(setTitle(float)));

    connect(this, SIGNAL(valuesChanged()), this, SLOT(recalculateValues()));

    connect(serial, SIGNAL(error(QSerialPort::SerialPortError)), this, SLOT(handleError(QSerialPort::SerialPortError)));

    connect(serial, SIGNAL(readyRead()), this, SLOT(readData()));

    ui->renderArea->setMinimumSize(Settings::bufferSize,256*Settings::heightConstant);
    ui->renderArea->setMaximumSize(Settings::bufferSize,256*Settings::heightConstant);
    ui->renderArea->resize(Settings::bufferSize,256*Settings::heightConstant);

    resize(minimumWidth(), minimumHeight());
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::setPrescalerValue(int value) {
    value = (int)pow(2, value);
    Settings::prescaler = value;
    ui->sbPrescaler->setValue(value);
    QByteArray d;
    d = QByteArray("p",1);
    d.append(QByteArray::number(value, 10 ));
    writeData(d);
}

void MainWindow::setTriggerLevel(int value) {
    QByteArray d;
    d = QByteArray("t",1);
    d.append(QByteArray::number(value, 10 ));
    writeData(d);
    Settings::triggerLevel = value;
    ui->dsbThreshold->setValue(value*Settings::VPerUnit);
}

void MainWindow::setHoldoffValue (int value) {
    Settings::holdoff = value;
    QByteArray d;
    d = QByteArray("w",1);
    d.append(QByteArray::number(value, 10 ));
    writeData(d);
}

void MainWindow::recalculateValues()
{
    float voltageC = buffer[cursorPosition] * 0.02;
    ui->leVoltage->setText(QString::number(voltageC,'f',2));
    float voltage1 = buffer[measure1Position] * 0.02;
    ui->leVoltage1->setText(QString::number(voltage1,'f',2));
    float voltage2 = buffer[measure2Position] * 0.02;
    ui->leVoltage2->setText(QString::number(voltage2,'f',2));
    float timeC = ((cursorPosition- Settings::bufferSize/2)/Settings::prescalerConstant)*Settings::prescaler;
    ui->leTime->setText(QString::number(timeC,'f',2));
    float time1 = ((measure1Position - Settings::bufferSize/2)/Settings::prescalerConstant)*Settings::prescaler;
    ui->leTime1->setText(QString::number(time1,'f',2));
    float time2 = ((measure2Position - Settings::bufferSize/2)/Settings::prescalerConstant)*Settings::prescaler;
    ui->leTime2->setText(QString::number(time2,'f',2));

    ui->leDiffV21->setText(QString::number(fabs(voltage2 - voltage1),'f',2));
    ui->leDiffVc1->setText(QString::number(fabs(voltageC - voltage1),'f',2));
    ui->leDiffVc2->setText(QString::number(fabs(voltageC - voltage2),'f',2));
    ui->leDifft21->setText(QString::number(fabs(time2 - time1),'f',2));
    ui->leDifftc1->setText(QString::number(fabs(timeC - time1),'f',2));
    ui->leDifftc2->setText(QString::number(fabs(timeC - time2),'f',2));

    ui->dsbHoldoff->setValue(Settings::holdoff/Settings::prescalerConstant*Settings::prescaler);
    ui->dsbThreshold->setValue(Settings::triggerLevel*Settings::VPerUnit);

}

void MainWindow::setTriggerEventToggle() {
    writeData(QByteArray("e0",2));
}

void MainWindow::setTriggerEventFalling() {
    writeData(QByteArray("e2",2));
}

void MainWindow::setTriggerEventRising() {
    writeData(QByteArray("e3",2));
}

void MainWindow::setVRefAREF() {
    writeData(QByteArray("r0",2));
}

void MainWindow::setVRefAVCC() {
    writeData(QByteArray("r1",2));
}

void MainWindow::setVRefInternal() {
    writeData(QByteArray("r3",2));
}

void MainWindow::setValuesCursor(int value) {
    cursorPosition = value;
    emit recalculateValues();
}

void MainWindow::setValues1 (int value) {
    measure1Position = value;
    emit recalculateValues();
}

void MainWindow::setValues2 (int value) {
    measure2Position = value;
    emit recalculateValues();
}

void MainWindow::setTitle(float fps) {
    this->setWindowTitle (QString::number(fps));
}

void MainWindow::setupCOM() {
    settings->show();
}

void MainWindow::startSampling() {
    QByteArray d;
    d = QByteArray("s", 1);
    writeData(d);
}

void MainWindow::stopSampling(){
    QByteArray d;
    d = QByteArray("S", 1);
    writeData(d);
    bufferFilled = 0;
}

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

void MainWindow::disconnectCOM() {
    closeSerialPort();
}
void MainWindow::status() {
    QByteArray d;
    d = QByteArray("d", 1);
    writeData(d);
}

void MainWindow::openSerialPort() {
    SettingsDialog::Settings p = settings->settings();
    serial->setPortName(p.name);
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

void MainWindow::closeSerialPort() {
    serial->close();
}

void MainWindow::about() {

}

void MainWindow::writeData(const QByteArray &data) {
    if (serial->isOpen ()) {
        serial->write(data);
    }
    qDebug() << "Writing to serial: " << data;
}

void MainWindow::readData() {
    QByteArray data = serial->readAll();
    if ((bufferFilled + data.size()) > Settings::bufferSize) {
        bufferFilled = 0;
    }
    for (int i = 0; i < data.size(); ++i) {
        buffer[bufferFilled + i] = data[i];
    }
    bufferFilled+=data.size();
    ui->renderArea->drawData(buffer);
}

/*void MainWindow::immaginaryData() {
    for (int i = 0; i < BUFFER_SIZE; ++i) {
        buffer[i] = (unsigned char)((qrand() % 256));
    }
    ui->renderArea->drawData(buffer);
}*/


void MainWindow::handleError(QSerialPort::SerialPortError error) {
    if (error == QSerialPort::ResourceError) {
        qDebug() << "Critical Error" << serial->errorString();
        closeSerialPort();
    }
}

void MainWindow::on_actionExit_triggered() {
    close();
}

void MainWindow::on_actionAbout_triggered() {
    aboutDialog->show();
}

void MainWindow::on_actionCOM_Port_Settings_triggered() {
    setupCOM();
}
