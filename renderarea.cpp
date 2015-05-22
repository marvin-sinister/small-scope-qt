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
** pen colors: http://www.w3.org/TR/SVG/types.html#ColorKeywords
****************************************************************************/

#include <QtGui>

#include "renderarea.h"

RenderArea::RenderArea(QWidget *parent) : QWidget(parent) {
    // set background color
    QPalette pal(palette());
    pal.setColor(QPalette::Background, "powderblue");
    setPalette(pal);
    setAutoFillBackground(true);

    // initialize
    dots = new QPoint[Settings::bufferSize];
    centerPen.setColor("steelblue");
    divisionPen.setColor("skyblue");
    triggerPen.setColor("red");
    channelPen.setColor("darkgreen");
    cursorPen.setColor("white");
    lPen.setColor("orange");
    rPen.setColor("blue");
    lPosition = 0;
    rPosition = Settings::bufferSize - 1;
    timePerDivision = 10;
    transformMatrix.translate(0, 256*Settings::heightConstant-1);
    transformMatrix.scale(1, -1);
}

// draw the data received over serial
void RenderArea::drawData(unsigned char* data) {
    for (int x = 0; x < Settings::bufferSize; x++) {
        dots[x] = QPoint(x,data[x]*Settings::heightConstant);
    }
    update();

    // recalculate fps
    emit fpsChanged(frames/(time.elapsed()/1000.0));
    if (!(frames % 100)) {
        time.start();
        frames = 0;
    }
    frames ++;
}

// set the apropriate grid depending on prescaler
void RenderArea::setPrescalerValue(int value) {
    value = (int)pow(2, value);
    switch (value) {
        case 128: {
            timePerDivision = 10;
            break;
        }
        case 64: {
            timePerDivision = 5;
            break;
        }
        case 32: {
            timePerDivision = 2;
            break;
        }
        case 16: {
            timePerDivision = 1;
            break;
        }
        default: {
            timePerDivision = 1;
        }
    }
    update();
}

// handle mouse clicks
void RenderArea::mousePressEvent(QMouseEvent * event) {
    if (event->buttons() & Qt::LeftButton ) {
        lPosition = mousePosition.x();
        emit lPositionChanged(lPosition);
    }
    if (event->buttons() & Qt::RightButton ) {
        rPosition = mousePosition.x();
        emit rPositionChanged(rPosition);
    }
}

// handle mouse move
void RenderArea::mouseMoveEvent(QMouseEvent * event) {
    mousePosition = event->pos();
    emit mouseMoved(mousePosition.x ());
    update();
}

// draw the graph
void RenderArea::paintEvent(QPaintEvent * /* event */) {
    // create painter
    QPainter painter(this);

    // transform
    painter.setMatrix(transformMatrix);

    // draw the voltage and time divisions
    painter.setPen(divisionPen);
    int maxT = (width()/Settings::prescalerConstant)*Settings::prescaler;
    for (float t = 0; t < maxT/2 + 1; t += timePerDivision) {
        painter.drawLine(QPoint((width()/2)-(Settings::prescalerConstant*t/Settings::prescaler), 0), QPoint(QPoint((width()/2)-(Settings::prescalerConstant*t/Settings::prescaler), height())));
        painter.drawLine(QPoint((width()/2)+(Settings::prescalerConstant*t/Settings::prescaler), 0), QPoint(QPoint((width()/2)+(Settings::prescalerConstant*t/Settings::prescaler), height())));
    }
    for (int y = 50 * Settings::heightConstant; y < height(); y += 50 * Settings::heightConstant) {
        painter.drawLine(QPoint(0, y), QPoint(width(), y));
    }

    // draw the center lines
    painter.setPen(centerPen);
    painter.drawLine(QPoint(0, height()/2), QPoint(width(), height()/2));
    painter.drawLine(QPoint(width()/2, 0), QPoint(width()/2, height()));

    // draw trigger level and position
    painter.setPen(triggerPen);
    painter.drawLine(QPoint(0, Settings::triggerLevel*Settings::heightConstant), QPoint(width(), Settings::triggerLevel*Settings::heightConstant));
    painter.drawLine(QPoint(width()-Settings::holdoff, 0), QPoint(width()-Settings::holdoff, height()));

    // draw the actual data
    painter.setPen(channelPen);
    painter.drawPolyline(dots, width());

    // draw the first measuring line
    painter.setPen(lPen);
    painter.drawLine (QPoint(lPosition,0), QPoint(lPosition,height()));

    // draw the second measuring line
    painter.setPen(rPen);
    painter.drawLine (QPoint(rPosition,0), QPoint(rPosition,height()));

    // draw the cursor line
    painter.setPen(cursorPen);
    painter.drawLine (QPoint(mousePosition.x(),0), QPoint(mousePosition.x(),height()));

    // recalculate all values
    emit recalculateValues();
}
