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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDebug>
#include <math.h>

#include "renderarea.h"
#include "settings.h"
#include "about.h"

namespace Ui {
class MainWindow;
}

class SettingsDialog;
class About;

class MainWindow : public QMainWindow {

    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:

private slots:
    void startSampling();
    void stopSampling();
    void setupCOM();
    void connectCOM();
    void disconnectCOM();
    void status();

    void setTriggerEventToggle();
    void setTriggerEventFalling();
    void setTriggerEventRising();
    void setTriggerEventNone();
    void setVRefAREF();
    void setVRefAVCC();
    void setVRefInternal();

    void openSerialPort();
    void closeSerialPort();
    void writeData(const QByteArray &data);
    void readData();
    //void immaginaryData();
    void handleError(QSerialPort::SerialPortError error);

    void about();

    void setPrescalerValue(int value);
    void setTriggerLevel(int value);
    void setHoldoffValue(int value);
    void recalculateValues();

    void setValuesCursor(int);
    void setTitle(float fps);
    void setValues1(int);
    void setValues2(int);

    void on_actionExit_triggered();

    void on_actionAbout_triggered();

    void on_actionCOM_Port_Settings_triggered();

signals:
    void valuesChanged();

private:
    Ui::MainWindow *ui;
    QSerialPort* serial;
    SettingsDialog *settings;
    About *aboutDialog;
    unsigned char* buffer;
    int bufferFilled;
    unsigned char* plotData;
    int cursorPosition;
    int measure1Position;
    int measure2Position;
};

#endif // MAINWINDOW_H
