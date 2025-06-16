/*
 File: ChartsTooltip.cpp
 Created on: 04/06/2025
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

// Qt
#include <QApplication>
#include <QToolTip>
#include <QPainter>
#include <QDateTime>
#include <QVBoxLayout>
#include <QPieSlice>
#include <QLabel>

// Project
#include <ChartsTooltip.h>

//----------------------------------------------------------------------------
ChartTooltip::ChartTooltip(const QString title, const qreal value)
: QWidget(nullptr)
{
  setWindowFlags(Qt::ToolTip|Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint|Qt::WindowTransparentForInput);
  setPalette(QToolTip::palette());

  auto layout = new QVBoxLayout(this);
  layout->setContentsMargins(QMargins{5,5,5,5});
  QFont font("Arial", 11);
  font.setBold(true);
  QString name = title;
  auto parts = name.split(' ');
  if(parts.size() > 1 && parts.last().contains('%'))
  {
    parts.removeLast();
    name = parts.join(' ');
  }
  auto titleLabel = new QLabel(name);
  titleLabel->setAlignment(Qt::AlignCenter);
  titleLabel->setFont(font);
  layout->addWidget(titleLabel);
  const auto timeText = QString("Duration ") + QTime{0,0,0}.addSecs(value).toString("hh:mm:ss");
  auto sliceTime = new QLabel(timeText);
  layout->addWidget(sliceTime);
  setLayout(layout);
}

//----------------------------------------------------------------------------
void ChartTooltip::paintEvent(QPaintEvent *event)
{
  QPainter painter(this);
  painter.drawRect(0, 0, width()-1, height()-1);
  painter.end();

  QWidget::paintEvent(event);
}