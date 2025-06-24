/*
 File: PieChart.cpp
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

// Project
#include <PieChart.h>

// Qt
#include <QPieSlice>
#include <QPieLegendMarker>

//----------------------------------------------------------------------------
MainSlice::MainSlice(QPieSeries* breakdownSeries, QObject* parent) :
    QPieSlice(parent),
    m_breakdownSeries(breakdownSeries)
{
    connect(this, &MainSlice::percentageChanged, this, &MainSlice::updateLabel);
}

//----------------------------------------------------------------------------
void MainSlice::updateLabel()
{
    this->setLabel(QString("%1 %2%").arg(m_name).arg(percentage() * 100, 0, 'f', 2));
}

//----------------------------------------------------------------------------
PieChart::PieChart(QGraphicsItem* parent, Qt::WindowFlags wFlags) :
    QChart(QChart::ChartTypeCartesian, parent, wFlags)
{
    // create the series for main center pie
    m_mainSeries = new QPieSeries();
    m_mainSeries->setPieSize(0.7);
    QChart::addSeries(m_mainSeries);
}

//----------------------------------------------------------------------------
void PieChart::addBreakdownSeries(QPieSeries* breakdownSeries, QColor color)
{
    QFont font("Arial", 8);

    // add breakdown series as a slice to center pie
    MainSlice* mainSlice = new MainSlice(breakdownSeries);
    mainSlice->setName(breakdownSeries->name());
    mainSlice->setValue(breakdownSeries->sum());
    m_mainSeries->append(mainSlice);
    connect(breakdownSeries, SIGNAL(hovered(QPieSlice *, bool)), this, SIGNAL(hovered(QPieSlice *, bool)));
    connect(mainSlice, SIGNAL(hovered(bool)), this, SLOT(hoveredSlice(bool)));

    // customize the slice
    mainSlice->setBrush(color);
    mainSlice->setLabelVisible();
    mainSlice->setLabelColor(Qt::white);
    mainSlice->setLabelPosition(QPieSlice::LabelInsideHorizontal);
    mainSlice->setLabelFont(font);

    // position and customize the breakdown series
    breakdownSeries->setPieSize(0.8);
    breakdownSeries->setHoleSize(0.7);
    breakdownSeries->setLabelsVisible();
    for(auto slice: breakdownSeries->slices()) {
        color = color.lighter(115);
        slice->setBrush(color);
        slice->setLabelFont(font);
    }

    // add the series to the chart
    QChart::addSeries(breakdownSeries);

    // recalculate breakdown donut segments
    recalculateAngles();

    // update customize legend markers
    updateLegendMarkers();
}

//----------------------------------------------------------------------------
void PieChart::hoveredSlice(bool state)
{
    auto slice = qobject_cast<QPieSlice*>(sender());
    emit hovered(slice, state);
}

//----------------------------------------------------------------------------
void PieChart::recalculateAngles()
{
    qreal angle = 0;
    foreach (QPieSlice* slice, m_mainSeries->slices()) {
        QPieSeries* breakdownSeries = qobject_cast<MainSlice*>(slice)->breakdownSeries();
        breakdownSeries->setPieStartAngle(angle);
        angle += slice->percentage() * 360.0; // full pie is 360.0
        breakdownSeries->setPieEndAngle(angle);
    }
}

//----------------------------------------------------------------------------
void PieChart::updateLegendMarkers()
{
    // go through all markers
    foreach (QAbstractSeries* series, series()) {
        foreach (QLegendMarker* marker, legend()->markers(series)) {
            QPieLegendMarker* pieMarker = qobject_cast<QPieLegendMarker*>(marker);
            if (series == m_mainSeries) {
                // hide markers from main series
                pieMarker->setVisible(false);
            } else {
                // modify markers from breakdown series
                pieMarker->setLabel(QString("%1 %2%")
                                        .arg(pieMarker->slice()->label())
                                        .arg((pieMarker->slice()->angleSpan() / 360) * 100, 0, 'f', 2));
                pieMarker->setFont(QFont("Arial", 8));
            }
        }
    }
}
