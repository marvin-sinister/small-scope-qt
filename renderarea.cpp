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
    antialiased = false;
    transformed = false;

    QPalette pal(palette());
    pal.setColor(QPalette::Background, "powderblue");
    setPalette(pal);
    setAutoFillBackground(true);
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
}

void RenderArea::drawData(unsigned char* data) {
    for (int x = 0; x < Settings::bufferSize; x++) {
        dots[x] = QPoint(x,data[x]*Settings::heightConstant);
    }
    update();
    emit fpsChanged(frames/(time.elapsed()/1000.0));
    if (!(frames % 100)) {
        time.start();
        frames = 0;
    }
    frames ++;
}

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

void RenderArea::mouseMoveEvent(QMouseEvent * event) {
    mousePosition = event->pos();
    emit mouseMoved(mousePosition.x ());
    update();
}

void RenderArea::paintEvent(QPaintEvent * /* event */) {
    QPainter painter(this);

    QMatrix m;
    m.translate(0, height()-1);
    m.scale(1, -1);
    painter.setMatrix(m);

    painter.setPen(divisionPen);

    int maxT = (width()/Settings::prescalerConstant)*Settings::prescaler;
    for (float t = 0; t < maxT/2 + 1; t += timePerDivision) {
        painter.drawLine(QPoint((width()/2)-(Settings::prescalerConstant*t/Settings::prescaler), 0), QPoint(QPoint((width()/2)-(Settings::prescalerConstant*t/Settings::prescaler), height())));
        painter.drawLine(QPoint((width()/2)+(Settings::prescalerConstant*t/Settings::prescaler), 0), QPoint(QPoint((width()/2)+(Settings::prescalerConstant*t/Settings::prescaler), height())));
    }
    for (int y = 50 * Settings::heightConstant; y < height(); y += 50 * Settings::heightConstant) {
        painter.drawLine(QPoint(0, y), QPoint(width(), y));
    }
    painter.setPen(centerPen);
    painter.drawLine(QPoint(0, height()/2), QPoint(width(), height()/2));
    painter.drawLine(QPoint(width()/2, 0), QPoint(width()/2, height()));

    painter.setPen(triggerPen);
    painter.drawLine(QPoint(0, Settings::triggerLevel*Settings::heightConstant), QPoint(width(), Settings::triggerLevel*Settings::heightConstant));
    painter.drawLine(QPoint(width()-Settings::holdoff, 0), QPoint(width()-Settings::holdoff, height()));

    painter.setPen(channelPen);
    painter.drawPolyline(dots, width());
    /*for (int x=0; x<Settings::bufferSize-1; ++x) {
        painter.drawLine(dots[x], dots[x+1]);
    }*/

    painter.setPen(lPen);
    painter.drawLine (QPoint(lPosition,0), QPoint(lPosition,height()));

    painter.setPen(rPen);
    painter.drawLine (QPoint(rPosition,0), QPoint(rPosition,height()));

    painter.setPen(cursorPen);
    painter.drawLine (QPoint(mousePosition.x(),0), QPoint(mousePosition.x(),height()));

    emit lPositionChanged(lPosition);
    emit rPositionChanged(rPosition);
    emit mouseMoved(mousePosition.x ());

}
