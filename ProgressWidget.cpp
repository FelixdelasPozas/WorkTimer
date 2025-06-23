/*
    File: ProgressWidget.cpp
    Created on: 02/05/2025
    Author: Felix de las Pozas Alvarez

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// Project
#include <ProgressWidget.h>

// QT
#include <QPainter>
#include <QBrush>
#include <QStyle>
#include <QPen>
#include <QColor>

//----------------------------------------------------------------------------
ProgressWidget::ProgressWidget(QWidget* parent) :
    QProgressBar{parent},
    m_config{Utils::Configuration()}
{
}

//----------------------------------------------------------------------------
void ProgressWidget::paintEvent(QPaintEvent*)
{
    QColor paintColor;
    float xMin = 0, xMax = 0;

    const int minutes = m_config.minutesInSession();
    const float minuteWidth = static_cast<float>(width()) / minutes;
    const int parts = 2 * m_config.m_unitsPerSession - 1;

    QPainter painter(this);
    for (int i = 1; i <= parts; ++i) {
        if (i % 2 == 0) {
            if (i % (2 * m_config.m_workUnitsBeforeBreak) == 0) {
                xMax += minuteWidth * m_config.m_longBreakTime;
                paintColor = m_config.m_longBreakColor;
            } else {
                xMax += minuteWidth * m_config.m_shortBreakTime;
                paintColor = m_config.m_shortBreakColor;
            }
        } else {
            xMax += minuteWidth * m_config.m_workUnitTime;
            paintColor = m_config.m_workColor;
        }

        const int pos = QStyle::sliderPositionFromValue(minimum(), maximum(), value(), width());

        if (xMin <= pos && xMax >= pos) {
            painter.setPen(paintColor);
            painter.setBrush(QBrush(paintColor));
            painter.drawRect(xMin, 0, pos, height());

            painter.setPen(paintColor.darker());
            painter.setBrush(QBrush(paintColor.darker()));
            painter.drawRect(pos, 0, xMax, height());
        } else {
            if (xMin > pos) {
                paintColor = paintColor.darker();
            }

            painter.setPen(paintColor);
            painter.setBrush(QBrush(paintColor));
            painter.drawRect(xMin, 0, xMax, height());
        }

        xMin = xMax;
    }

    painter.setPen(Qt::lightGray);
    painter.setBrush(QBrush(Qt::transparent));
    painter.drawRect(0, 0, width(), height());

    painter.setPen(Qt::white);
    painter.setBrush(QBrush(Qt::white));
    painter.drawText(0, 0, width(), height(), Qt::AlignCenter, QString::number(value()) + "%");
    painter.end();
}