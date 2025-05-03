/*
    File: MainWindow.cpp
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

// Project
#include <MainWindow.h>
#include <AboutDialog.h>
#include <ConfigurationDialog.h>
#include <ProgressWidget.h>

// Qt
#include <QDateTime>

//----------------------------------------------------------------------------
MainWindow::MainWindow(QWidget* p, Qt::WindowFlags f) :
    QMainWindow{p, f},
    m_widget{false, this}
{
    setupUi(this);

    connectSignals();

    m_configuration.load();

    applyConfiguration();
}

//----------------------------------------------------------------------------
MainWindow::~MainWindow()
{
    m_configuration.save();
}

//----------------------------------------------------------------------------
void MainWindow::connectSignals()
{
    connect(actionTimer, SIGNAL(triggered(bool)), this, SLOT(onPlayClicked()));
    connect(actionStop, SIGNAL(triggered(bool)), this, SLOT(onStopClicked()));
    connect(actionMinimize, SIGNAL(triggered(bool)), this, SLOT(onMinimizeClicked()));
    connect(actionAbout_WorkTimer, SIGNAL(triggered(bool)), this, SLOT(showAbout()));
    connect(actionConfiguration, SIGNAL(triggered(bool)), this, SLOT(openConfiguration()));
    connect(actionQuit, SIGNAL(triggered(bool)), this, SLOT(close()));
}

//----------------------------------------------------------------------------
void MainWindow::applyConfiguration()
{
    qobject_cast<ProgressWidget*>(m_progressBar)->setConfiguration(m_configuration);

    m_widget.setVisible(false);
    m_widget.setPosition(m_configuration.m_widgetPosition);
    m_widget.setColor(m_configuration.m_workColor);
    m_widget.setProgress(0);
    m_widget.setName("Task name");
    m_widget.setOpacity(m_configuration.m_widgetOpacity);

    m_timer.setContinuousTicTac(m_configuration.m_continuousTicTac);
    m_timer.setWorkUnitsBeforeBreak(2);
    m_timer.setLongBreakDuration(QTime{0, m_configuration.m_longBreakTime, 0, 0});
    m_timer.setShortBreakDuration(QTime{0, m_configuration.m_shortBreakTime, 0, 0});
    m_timer.setWorkDuration(QTime{0, m_configuration.m_workUnitTime, 0, 0});
    m_timer.setUseSounds(m_configuration.m_useSound);
    m_timer.setSessionWorkUnits(m_configuration.m_unitsPerSession);
}

//----------------------------------------------------------------------------
void MainWindow::showAbout()
{
    AboutDialog dialog;
    dialog.exec();
}

//----------------------------------------------------------------------------
void MainWindow::openConfiguration()
{
    ConfigurationDialog dialog(m_configuration, this);
    if (dialog.exec() != QDialog::Accepted) return;

    dialog.getConfiguration(m_configuration);
}

//----------------------------------------------------------------------------
void MainWindow::onPlayClicked()
{
    switch(m_timer.status())
    {
        case WorkTimer::Status::Stopped:
            m_timer.start();
            actionTimer->setIcon(QIcon(":/WorkTimer/pause.svg"));
            actionStop->setEnabled(true);
            break;
        case WorkTimer::Status::Work:
            m_timer.pause();
            actionTimer->setIcon(QIcon(":/WorkTimer/play.svg"));
            break;
        case WorkTimer::Status::ShortBreak:
            break;
        case WorkTimer::Status::LongBreak:
            break;
        case WorkTimer::Status::Paused:
            m_timer.pause();
            actionTimer->setIcon(QIcon(":/WorkTimer/pause.svg"));
            break;
        default:
            break;
    }
}

//----------------------------------------------------------------------------
void MainWindow::onStopClicked()
{
    m_timer.stop();
    actionStop->setEnabled(false);
    actionTimer->setIcon(QIcon(":/WorkTimer/play.svg"));
}

//----------------------------------------------------------------------------
void MainWindow::onMinimizeClicked()
{
}
