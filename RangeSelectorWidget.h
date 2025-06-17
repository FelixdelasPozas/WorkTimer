/*
    File: RangeSelectorWidget.h
    Created on: 27/04/2025
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

#ifndef _RANGE_SELECTOR_WIDGET_H_
#define _RANGE_SELECTOR_WIDGET_H_

// Qt
#include <QWidget>
#include "ui_RangeSelectorWidget.h"

class RangeSelectorWidget : public QWidget, private Ui::RangeSelectorWidget
{
    Q_OBJECT
  public:
    enum class Button : char
    {
        WEEK = 0,
        MONTH,
        YEAR,
        CUSTOM
    };

    /** \brief RangeSelectorWidget class constructor.
     * \param[in] parent Raw pointer of the widget parent of this one.
     * \param[in] f Widget flags.
     */
    RangeSelectorWidget(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

    /** \brief RangeSelectorWidget class virtual destructor. 
     *
     */
    virtual ~RangeSelectorWidget()
    {
    }

    /** \brief Sets the range of the from and to date widgets. 
     * \param[in] from From date. 
     * \param[in] to To date. 
     */
    void setRange(const QDateTime& from, const QDateTime& to, const bool emitSignal = true);

    /** \brief Sets the selected button without emitting a range signal. 
     * \param[in] button Button enum. 
     */
    void setButton(const Button button);

    /** \brief Returns the current checked button. 
     */
    Button button() const;

  signals:
    void rangeChanged(const QDateTime&, const QDateTime&);
    void exportDataCSV(const QDateTime&, const QDateTime&);
    void exportDataExcel(const QDateTime&, const QDateTime&);

  private slots:
    /** \brief Updates the range when a button gets clicked. 
     */
    void onButtonClicked();

    /** \brief Updates the range with a date changes.
     */
    void onDateChanged();

    /** \brief Sends the signal to export data to a CSV file or an Excel file. 
     */
    void onExportClicked();

  private:
    /** \brief Helper method to connect signals to slots. 
     */
    void connectSignals();
};

#endif