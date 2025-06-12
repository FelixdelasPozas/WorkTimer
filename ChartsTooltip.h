/*
 File: ChartsTooltip.h
 Created on: 11/06/2025
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

#ifndef _CHARTS_TOOLTIP_H_
#define _CHARTS_TOOLTIP_H_

// Qt
#include <QWidget>

/** \class ChartTooltip
 * \brief Widget that acts as a tooltip for the charts.
 *
 */
class ChartTooltip
: public QWidget
{
    Q_OBJECT
  public:
    /** \brief ChartTooltip class constructor.
     * \param[in] title Tooltip title text.
     * \param[in] value Seconds of the entry.
     *
     */
    explicit ChartTooltip(const QString title, const qreal value);

    /** \brief ChartTooltip class virtual destructor.
     *
     */
    virtual ~ChartTooltip()
    {};

  protected:
    virtual void paintEvent(QPaintEvent *event) override;
};

#endif