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
#include <WorkTimer.h>
#include <DesktopWidget.h>

// Qt
#include <QMainWindow>
#include <QSystemTrayIcon>

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

  protected:
    virtual void closeEvent(QCloseEvent *) override;

  private:
    /** \brief Helper method to connect signals to slots. 
     *
     */
    void connectSignals();

    /** \brief Applies the current configuration to the UI, widget and timer.
     *
     */
    void applyConfiguration();

    /** \brief Initalizes the task table.
     *
     */
    void initTables();

    /** \brief Initializes the tray icon and its menu.
     *
     */
    void initIconAndMenu();

  private slots:
    /** \brief Shows the About dialog.
     */
    void showAbout();

    /** \brief Opens the configuration dialog and applies it.
     */
    void openConfiguration();

    /** \brief Helper method to change the UI and start/pause the timer. 
     */
    void onPlayClicked();

    /** \brief Helper method to change the UI and stop the timer. 
     */
    void onStopClicked();

    /** \brief Helper method to change the task name and modify UI.
     */
    void onTaskNameClicked();

    /** \brief Updates the application progress in the widgets and icon.
     * \param[in] progress Progress value in [0,100]
     */
    void onProgressUpdated(unsigned int progress);

    /** \brief Helper method to update the session progress value.
     */
    void onUnitEnded();

    /** \brief Helper method to update the session progress value.
     */
    void onUnitStarted();

    /** \brief Shows the main dialog when activated by the tray icon.
     * \param[in] reason Activation reason.
     */
    void onTrayActivated(QSystemTrayIcon::ActivationReason reason = QSystemTrayIcon::DoubleClick);

    /** \brief Helper method to help with application close.
     */
    void quitApplication();

  private:
    Utils::Configuration m_configuration; /** application configuration. */
    DesktopWidget m_widget;               /** desktop widget. */
    WorkTimer m_timer;                    /** work timer. */
    unsigned int m_globalProgress = 0;    /** current global progress in minutes in [0-totalMinutes] */
    unsigned int m_totalMinutes = 0;      /** total minutes in the session including breaks. */
    QSystemTrayIcon* m_trayIcon;          /** tray icon. */
    QAction *m_timerEntry;                /** tray menu entry for play/pause. */
    QAction *m_stopEntry;                 /** tray menu entry for stop the timer. */
    QAction *m_taskEntry;                 /** tray menu entry for changing the task name. */
    bool m_needsExit = false;             /** true to exit application at close(), false otherwise. */
};

#endif