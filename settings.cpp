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

#include "settings.h"

float Settings::VPerUnit = 0.02f; // volts per unit (Vmax/2^ADCbits)
float Settings::heightConstant = 2.0f; // vertical stretch (RenderArea.height/2^ADCbits)
float Settings::PxPerUnit = Settings::VPerUnit * Settings::heightConstant; // pixels per unit

int Settings::bufferSize = 1024; // buffer size
//int Settings::bufferSize = 1280; // buffer size

float Settings::prescalerConstant = 1248.0f; // prescaler constant (samples/ms * prescaler)
int Settings::prescaler = 128; // prescaler
int Settings::triggerLevel = 128; // trigger level (Vcc/2)
int Settings::holdoff = 768; // sampling time after tigger
