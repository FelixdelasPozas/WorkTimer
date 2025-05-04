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
#include <QInputDialog>

//----------------------------------------------------------------------------
MainWindow::MainWindow(QWidget* p, Qt::WindowFlags f) :
    QMainWindow{p, f},
    m_widget{false, this}
{
    setupUi(this);

    connectSignals();

    m_configuration.load();

    applyConfiguration();

    initTable();
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
    connect(actionTask, SIGNAL(triggered(bool)), this, SLOT(onTaskNameClicked()));
    connect(&m_timer, SIGNAL(progress(unsigned int)), this, SLOT(onProgressUpdated(unsigned int)));
    connect(&m_timer, SIGNAL(sessionEnded()), this, SLOT(onStopClicked()));

    connect(&m_timer, SIGNAL(beginWorkUnit()), this, SLOT(onGlobalProgressUpdated()));
    connect(&m_timer, SIGNAL(beginShortBreak()), this, SLOT(onGlobalProgressUpdated()));
    connect(&m_timer, SIGNAL(beginLongBreak()), this, SLOT(onGlobalProgressUpdated()));
}

//----------------------------------------------------------------------------
void MainWindow::applyConfiguration()
{
    qobject_cast<ProgressWidget*>(m_progressBar)->setConfiguration(m_configuration);

    m_widget.setVisible(false);
    m_widget.setPosition(m_configuration.m_widgetPosition);
    m_widget.setColor(m_configuration.m_workColor);
    m_widget.setProgress(0);
    m_widget.setOpacity(m_configuration.m_widgetOpacity);

    m_timer.setContinuousTicTac(m_configuration.m_continuousTicTac);
    m_timer.setWorkUnitsBeforeBreak(2);
    m_timer.setLongBreakDuration(QTime{0, m_configuration.m_longBreakTime, 0, 0});
    m_timer.setShortBreakDuration(QTime{0, m_configuration.m_shortBreakTime, 0, 0});
    m_timer.setWorkDuration(QTime{0, m_configuration.m_workUnitTime, 0, 0});
    m_timer.setUseSounds(m_configuration.m_useSound);
    m_timer.setSessionWorkUnits(m_configuration.m_unitsPerSession);

    const int workMinutes = m_configuration.m_unitsPerSession * m_configuration.m_workUnitTime;
    const int breaksMinutes = (m_configuration.m_unitsPerSession / 4) *
                              (3 * m_configuration.m_shortBreakTime + m_configuration.m_longBreakTime);
    m_totalMinutes = workMinutes + breaksMinutes;
}

//----------------------------------------------------------------------------
void MainWindow::initTable()
{
    m_table->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
    m_table->horizontalHeader()->setSectionsMovable(false);
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_table->verticalHeader()->setVisible(false);
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
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    dialog.getConfiguration(m_configuration);
}

//----------------------------------------------------------------------------
void MainWindow::onPlayClicked()
{
    if (m_widget.getName().isEmpty()) {
        onTaskNameClicked();

        if (m_table->rowCount() == 0) {
            return;
        }
    }

    switch (m_timer.status()) {
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

    actionTask->setEnabled(true);
    m_widget.setVisible(m_configuration.m_useWidget);
}

//----------------------------------------------------------------------------
void MainWindow::onStopClicked()
{
    m_timer.stop();
    actionStop->setEnabled(false);
    actionTimer->setIcon(QIcon(":/WorkTimer/play.svg"));
    m_widget.setVisible(false);
}

//----------------------------------------------------------------------------
void MainWindow::onTaskNameClicked()
{
    QInputDialog dialog{this};
    dialog.setInputMode(QInputDialog::InputMode::TextInput);
    dialog.setWindowIcon(QIcon(":/WorkTimer/pencil.svg"));
    dialog.setWindowTitle("Task");
    dialog.setLabelText("Enter task name:");
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    auto taskName = dialog.textValue();
    if (taskName.isEmpty()) {
        return;
    }

    m_widget.setName(taskName);
    m_timer.setTaskTitle(taskName);

    const auto rows = m_table->rowCount();
    if(rows == 0 || m_table->item(rows-1, 0)->text().compare(taskName, Qt::CaseInsensitive) != 0)
    {
        m_table->insertRow(rows);
        auto item = new QTableWidgetItem(taskName);
        item->setTextAlignment(Qt::AlignCenter);
        m_table->setItem(rows, 0, item);
        item = new QTableWidgetItem(QTime{0,0,0}.toString("mm:ss"));
        item->setTextAlignment(Qt::AlignCenter);
        m_table->setItem(rows, 1, item);
        item = new QTableWidgetItem("0");
        item->setTextAlignment(Qt::AlignCenter);
        m_table->setItem(rows, 2, item);
    }
}

//----------------------------------------------------------------------------
void MainWindow::onMinimizeClicked()
{
}

//----------------------------------------------------------------------------
void MainWindow::onProgressUpdated(unsigned int value)
{
    m_widget.setProgress(value);

    unsigned int progressMinutes = 0;
    switch (m_timer.status()) {
        default:
        case WorkTimer::Status::Work:
            progressMinutes = value / 100.f * m_configuration.m_workUnitTime;
            break;
        case WorkTimer::Status::ShortBreak:
            progressMinutes = value / 100.f * m_configuration.m_shortBreakTime;
            break;
        case WorkTimer::Status::LongBreak:
            progressMinutes = value / 100.f * m_configuration.m_longBreakTime;
            break;
    }

    m_progressBar->setValue(((m_globalProgress + progressMinutes) * 100) / m_totalMinutes);
}

//----------------------------------------------------------------------------
void MainWindow::onGlobalProgressUpdated()
{
    static WorkTimer::Status previousStatus = WorkTimer::Status::Stopped;

    switch (previousStatus) {
        case WorkTimer::Status::Work:
            m_globalProgress += m_configuration.m_workUnitTime;
            m_widget.setColor(m_timer.status() == WorkTimer::Status::ShortBreak ? m_configuration.m_shortBreakColor
                                                                                : m_configuration.m_longBreakColor);
            break;
        case WorkTimer::Status::ShortBreak:
            m_globalProgress += m_configuration.m_shortBreakTime;
            m_widget.setColor(m_configuration.m_workColor);
            break;
        case WorkTimer::Status::LongBreak:
            m_globalProgress += m_configuration.m_longBreakTime;
            m_widget.setColor(m_configuration.m_workColor);
            break;
        default:
        case WorkTimer::Status::Paused:
        case WorkTimer::Status::Stopped:
            break;
    }

    previousStatus = m_timer.status();
    m_widget.setProgress(0);
    m_progressBar->setValue((m_globalProgress * 100) / m_totalMinutes);
}
