/*
    File: MainWindow.h
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

#ifndef _WTMAIN_H_
#define _WTMAIN_H_

// Project
#include "ui_MainWindow.h"
#include <Utils.h>

// Qt
#include <QMainWindow>

class MainWindow : public QMainWindow, private Ui::MainWindow
{
    Q_OBJECT
  public:
    /** \brief MainWindow class constructor.
     * \param[in] p Raw pointer of the QWidget parent of this one.
     * \param[in] f Window flags.
     */
    MainWindow(QWidget* p = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

    /** \brief MainWindow class virtual destructor. 
     *
     */
    virtual ~MainWindow();

  private:
    /** \brief Helper method to connect signals to slots. 
     *
     */
    void connectSignals();

  private slots:
    void startWorkTimer();
    void showAbout();
    void showStatistics();
    void openConfiguration();

  private:
    Utils::Configuration m_configuration; /** application configuration. */
};

#endif