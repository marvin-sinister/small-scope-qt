#############################################################################
##
## Copyright (C) 2015 Marvin Sinister <marvin@sinister.io>
##
## This file is part of small-scope-qt.
##
## small-scope-qt is free software: you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## small-scope-qt is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with small-scope-qt.  If not, see <http://www.gnu.org/licenses/>.
##
#############################################################################

QT       += core gui serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = small-scope-qt
TEMPLATE = app


SOURCES += main.cpp\
    settingsdialog.cpp \
    renderarea.cpp \
    settings.cpp \
    mainwindow.cpp \
    about.cpp

HEADERS  += settingsdialog.h \
    renderarea.h \
    settings.h \
    mainwindow.h \
    about.h

FORMS    += settingsdialog.ui \
    mainwindow.ui \
    about.ui
