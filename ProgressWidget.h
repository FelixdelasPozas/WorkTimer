/*
    File: ProgressWidget.h
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

#ifndef _PROGRESS_WIDGET_H_
#define _PROGRESS_WIDGET_H_

// Qt
#include <QProgressBar>

// Project
#include <Utils.h>

class ProgressWidget : public QProgressBar
{
    Q_OBJECT
  public:
    /** \brief ProgressWidget class constructor.
     * \param[in] parent Raw pointer of the widget parent of this one.
     *
     */
    explicit ProgressWidget(QWidget* parent = nullptr);

    /** \brief ProgressWidget class virtual destructor. 
     */
    virtual ~ProgressWidget() {};

    /** \brief Sets the configuration values.
     * \param[in] config Application configuration reference.
     *
     */
    void setConfiguration(const Utils::Configuration &config)
    { m_config = config; }

  protected:
    void paintEvent(QPaintEvent*) Q_DECL_OVERRIDE;

  private:
    Utils::Configuration m_config; /** reference to application configuration. */
};

#endif