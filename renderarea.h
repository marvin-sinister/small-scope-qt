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

#ifndef RENDERAREA_H
#define RENDERAREA_H

#include <QBrush>
#include <QPen>
#include <QGraphicsView>
#include <QWidget>
#include <QTime>

#include "settings.h"

class RenderArea : public QWidget {
    Q_OBJECT

public:
    RenderArea(QWidget *parent = 0);
    int lPosition;
    int rPosition;


public slots:
    void drawData(unsigned char* data);
    void setPrescalerValue(int value);


protected:
    void paintEvent(QPaintEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);

signals:
     void fpsChanged(float);
     void mouseMoved(int);
     void lPositionChanged(int);
     void rPositionChanged(int);

private:
    QPen centerPen;
    QPen divisionPen;
    QPen triggerPen;
    QPen channelPen;
    QPen lPen;
    QPen rPen;
    QPen cursorPen;
    QBrush brush;
    bool antialiased;
    bool transformed;
    QPoint* dots;
    QPoint mousePosition;
    int frames;
    QTime time;
    int timePerDivision;
};

#endif
