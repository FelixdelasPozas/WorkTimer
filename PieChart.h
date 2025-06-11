/*
 File: PieChart.h
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

#ifndef PIECHART_H
#define PIECHART_H

// Qt
#include <QChart>
#include <QPieSeries>
#include <QStyledItemDelegate>
#include <QString>
#include <QTime>

#include <QChartView>
#include <QPainter>
#include <QAbstractSeries>

/** \class MainSlice
 * \brief Implements the main slice style. 
 */
class MainSlice : public QPieSlice
{
    Q_OBJECT
  public:
    /** \brief MainSlice class constructor. 
     */
    MainSlice(QPieSeries* breakdownSeries, QObject* parent = 0);

    /** \brief Returns the breakdownseries.
     */
    QPieSeries* breakdownSeries() const
    { return m_breakdownSeries; }

    /** \brief Sets the slice name. 
     */
    void setName(QString name)
    { m_name = name; }

    /** \brief Returns the slice name. 
     */
    QString name() const
    { return m_name; }

  public Q_SLOTS:
    void updateLabel();

  private:
    QPieSeries* m_breakdownSeries; /** slice series. */
    QString m_name;                /** slice name. */
};

/** \class PieChart 
 * \brief Implements the pie chart mechanics.
 */
class PieChart : public QChart
{
    Q_OBJECT
  public:
    /** \brief PieChart class constructor. 
     * \param[in] parent Raw pointer of the parent of this object.
     * \param[in] f Window flags. 
     */
    PieChart(QGraphicsItem* parent = 0, Qt::WindowFlags f = Qt::WindowFlags());

    /** \brief PieChart class virtual destructor. 
     */
    virtual ~PieChart()
    {};

    /** \brief Adds a categorized serie to the chart. 
     * \param[in] series QPieSeries class pointer. 
     * \param[in] color Color assigned to the series. 
     */
    void addBreakdownSeries(QPieSeries* series, QColor color);

  signals:
      void hovered(QPieSlice *slice, bool state);

  private slots:
    void hoveredSlice(bool state);      

  private:
    /** \brief Helper method to recompute the angles of the internal pie.
     */
    void recalculateAngles();

    /** \brief Helper method to update the legend.
     */
    void updateLegendMarkers();

  private:
    QPieSeries* m_mainSeries; /** main series. */
};

#endif // DONUTBREAKDOWNCHART_H