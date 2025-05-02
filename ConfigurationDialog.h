/*
    File: ConfigurationDialog.h
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

#ifndef _CONFIGURATION_DIALOG_H_
#define _CONFIGURATION_DIALOG_H_

// Project
#include "ui_ConfigurationDialog.h"
#include <DesktopWidget.h>

// Qt
#include <QDialog>

namespace Utils
{
    struct Configuration;
}

/** \class ConfigurationDialog
 * \brief Implements the dialog to configure the application options.
 */
class ConfigurationDialog : public QDialog, private Ui::ConfigurationDialog
{
    Q_OBJECT
  public:
    /** \brief ConfigurationDialog class constructor.
     * \param[in] config Application configuration reference. 
     * \param[in] parent Raw pointer of the widget parent of this one. 
     * \param[in] f Dialog flags.
     */
    ConfigurationDialog(const Utils::Configuration& config, QWidget* parent = nullptr,
                        Qt::WindowFlags f = Qt::WindowFlags());

    /** \brief ConfigurationDialog class virtual destructor. 
     */
    virtual ~ConfigurationDialog() {};

    /** \brief Returns the configuration values.
     * \param[out] config Configuration struct.
     */
    void getConfiguration(Utils::Configuration& config);

  public slots:
    /** \brief Opens a color selection dialog to select a new color for the clicked unit button. 
     */
    void onColorButtonClicked();

    /** \brief Positions the desktop widget when the position combobox changes value.
     *
     */
    void onPositionChanged();

    /** \brief Modifies the UI when the user changes the value of the use checkbox.
     */
    void onWidgetCheckBoxChanged();

    /** \brief Modifies the UI when the user changes the value of the sounds checkbox.
     */
    void onUseSoundCheckBoxChanged();

  protected:
    virtual void showEvent(QShowEvent* e) override;

  private:
    /** \brief Helper method to connect signals to slots. 
     */
    void connectSignals();

    /** \brief Helper method to compute all the possible fixes positions of the desktop widget. 
     */
    void computeWidgetPositions();

    /** \brief Computes the position of the widget given the size of the screen.
     * \param[in] rect Screen size.
     * \param[in] screenName  Name of the current screen.
     * \param[out] positionNames Names of the positions.
     */
    void computePositions(const QRect& rect, const QString& screenName, QStringList& positionNames);

    /** \brief Modifies the UI with the configuration data.
     */
    void setConfiguration(const Utils::Configuration &config);

    QList<QPoint> m_widgetPositions; /** possible fixed desktop widget positions. */
    DesktopWidget m_widget;
};

#endif