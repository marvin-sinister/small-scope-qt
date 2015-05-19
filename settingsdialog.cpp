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

#include "settingsdialog.h"
#include "ui_settingsdialog.h"

#include <QtSerialPort/QSerialPortInfo>
#include <QIntValidator>
#include <QLineEdit>

QT_USE_NAMESPACE

SettingsDialog::SettingsDialog(QWidget *parent) : QDialog(parent), ui(new Ui::SettingsDialog) {

    ui->setupUi(this);

    intValidator = new QIntValidator(0, 4000000, this);

    ui->lbBaudRate->setInsertPolicy(QComboBox::NoInsert);

    connect(ui->pbOk, SIGNAL(clicked()), this, SLOT(ok()));
    connect(ui->pbCancel, SIGNAL(clicked()), this, SLOT(cancel()));
    connect(ui->pbRefresh, SIGNAL(clicked()), this, SLOT(refresh()));
    connect(ui->lbSerialPortInfo, SIGNAL(currentIndexChanged(int)), this, SLOT(showPortInfo(int)));
    connect(ui->lbBaudRate, SIGNAL(currentIndexChanged(int)), this, SLOT(checkCustomBaudRatePolicy(int)));

    fillPortsParameters();
    fillPortsInfo();

    updateSettings();
}

SettingsDialog::~SettingsDialog() {
    delete ui;
}

SettingsDialog::Settings SettingsDialog::settings() const {
    return currentSettings;
}

void SettingsDialog::showPortInfo(int idx) {
    if (idx != -1) {
        QStringList list = ui->lbSerialPortInfo->itemData(idx).toStringList();
        ui->descriptionLabel->setText(tr("Description: %1").arg(list.at(1)));
        ui->manufacturerLabel->setText(tr("Manufacturer: %1").arg(list.at(2)));
        ui->locationLabel->setText(tr("Location: %1").arg(list.at(3)));
        ui->vidLabel->setText(tr("Vendor Identifier: %1").arg(list.at(4)));
        ui->pidLabel->setText(tr("Product Identifier: %1").arg(list.at(5)));
    }
}

void SettingsDialog::ok() {
    updateSettings();
    close();
}

void SettingsDialog::cancel() {
    close();
}

void SettingsDialog::checkCustomBaudRatePolicy(int idx) {
    bool isCustomBaudRate = !ui->lbBaudRate->itemData(idx).isValid();
    ui->lbBaudRate->setEditable(isCustomBaudRate);
    if (isCustomBaudRate) {
        ui->lbBaudRate->clearEditText();
        QLineEdit *edit = ui->lbBaudRate->lineEdit();
        edit->setValidator(intValidator);
    }
}

void SettingsDialog::refresh() {
    loadSettings();
    fillPortsInfo();
}

void SettingsDialog::loadSettings() {
    qDebug() << "name " << currentSettings.name;
    qDebug() << "baud rate " << currentSettings.baudRate;
    qDebug() << "data bits " << currentSettings.dataBits;
    qDebug() << "parity " << currentSettings.parity;
    qDebug() << "stop bits " << currentSettings.stopBits;
    qDebug() << "flow control " << currentSettings.flowControl;
    qDebug() << "local echo " << currentSettings.localEchoEnabled;
}

void SettingsDialog::fillPortsParameters() {
    // fill baud rate (is not the entire list of available values,
    // desired values??, add your independently)
    ui->lbBaudRate->addItem(QLatin1String("9600"), QSerialPort::Baud9600);
    ui->lbBaudRate->addItem(QLatin1String("19200"), QSerialPort::Baud19200);
    ui->lbBaudRate->addItem(QLatin1String("38400"), QSerialPort::Baud38400);
    ui->lbBaudRate->addItem(QLatin1String("115200"), QSerialPort::Baud115200);
    ui->lbBaudRate->addItem(QLatin1String("500000"), 500000);
    ui->lbBaudRate->addItem(QLatin1String("Custom"));

    // fill data bits
    ui->lbDataBits->addItem(QLatin1String("5"), QSerialPort::Data5);
    ui->lbDataBits->addItem(QLatin1String("6"), QSerialPort::Data6);
    ui->lbDataBits->addItem(QLatin1String("7"), QSerialPort::Data7);
    ui->lbDataBits->addItem(QLatin1String("8"), QSerialPort::Data8);
    ui->lbDataBits->setCurrentIndex(3);

    // fill parity
    ui->lbParity->addItem(QLatin1String("None"), QSerialPort::NoParity);
    ui->lbParity->addItem(QLatin1String("Even"), QSerialPort::EvenParity);
    ui->lbParity->addItem(QLatin1String("Odd"), QSerialPort::OddParity);
    ui->lbParity->addItem(QLatin1String("Mark"), QSerialPort::MarkParity);
    ui->lbParity->addItem(QLatin1String("Space"), QSerialPort::SpaceParity);

    // fill stop bits
    ui->lbStopBits->addItem(QLatin1String("1"), QSerialPort::OneStop);
#ifdef Q_OS_WIN
    ui->lbStopBits->addItem(QLatin1String("1.5"), QSerialPort::OneAndHalfStop);
#endif
    ui->lbStopBits->addItem(QLatin1String("2"), QSerialPort::TwoStop);

    // fill flow control
    ui->lbFlowControl->addItem(QLatin1String("None"), QSerialPort::NoFlowControl);
    ui->lbFlowControl->addItem(QLatin1String("RTS/CTS"), QSerialPort::HardwareControl);
    ui->lbFlowControl->addItem(QLatin1String("XON/XOFF"), QSerialPort::SoftwareControl);
}

void SettingsDialog::fillPortsInfo() {
    ui->lbSerialPortInfo->clear();
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        QStringList list;
        list << info.portName()
             << info.description()
             << info.manufacturer()
             << info.systemLocation()
             << (info.vendorIdentifier() ? QString::number(info.vendorIdentifier(), 16) : QString())
             << (info.productIdentifier() ? QString::number(info.productIdentifier(), 16) : QString());

        ui->lbSerialPortInfo->addItem(list.first(), list);
    }
}

void SettingsDialog::updateSettings() {
    currentSettings.name = ui->lbSerialPortInfo->currentText();

    // Baud Rate
    if (ui->lbBaudRate->currentIndex() == 4) {
        // custom baud rate
        currentSettings.baudRate = 500000;
    }
    else if (ui->lbBaudRate->currentIndex() == 5) {
        // custom baud rate
        currentSettings.baudRate = ui->lbBaudRate->currentText().toInt();
    } else {
        // standard baud rate
        currentSettings.baudRate = static_cast<QSerialPort::BaudRate>(
                    ui->lbBaudRate->itemData(ui->lbBaudRate->currentIndex()).toInt());
    }
    currentSettings.stringBaudRate = QString::number(currentSettings.baudRate);

    // Data bits
    currentSettings.dataBits = static_cast<QSerialPort::DataBits>(
                ui->lbDataBits->itemData(ui->lbDataBits->currentIndex()).toInt());
    currentSettings.stringDataBits = ui->lbDataBits->currentText();

    // Parity
    currentSettings.parity = static_cast<QSerialPort::Parity>(
                ui->lbParity->itemData(ui->lbParity->currentIndex()).toInt());
    currentSettings.stringParity = ui->lbParity->currentText();

    // Stop bits
    currentSettings.stopBits = static_cast<QSerialPort::StopBits>(
                ui->lbStopBits->itemData(ui->lbStopBits->currentIndex()).toInt());
    currentSettings.stringStopBits = ui->lbStopBits->currentText();

    // Flow control
    currentSettings.flowControl = static_cast<QSerialPort::FlowControl>(
                ui->lbFlowControl->itemData(ui->lbFlowControl->currentIndex()).toInt());
    currentSettings.stringFlowControl = ui->lbFlowControl->currentText();

    // Additional options
    currentSettings.localEchoEnabled = ui->cbLocalEcho->isChecked();
}
