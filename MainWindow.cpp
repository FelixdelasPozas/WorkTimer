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

//----------------------------------------------------------------------------
MainWindow::MainWindow(QWidget* p, Qt::WindowFlags f) :
    QMainWindow{p, f}
{
    setupUi(this);

    connectSignals();

    m_configuration.load();
    qobject_cast<ProgressWidget*>(m_progressBar)->setConfiguration(m_configuration);
}

//----------------------------------------------------------------------------
MainWindow::~MainWindow()
{
    m_configuration.save();
}

//----------------------------------------------------------------------------
void MainWindow::connectSignals()
{
    connect(actionTimer, SIGNAL(triggered(bool)), this, SLOT(startWorkTimer()));
    connect(actionStatistics, SIGNAL(triggered(bool)), this, SLOT(showStatistics()));
    connect(actionAbout_WorkTimer, SIGNAL(triggered(bool)), this, SLOT(showAbout()));
    connect(actionConfiguration, SIGNAL(triggered(bool)), this, SLOT(openConfiguration()));
    connect(actionQuit, SIGNAL(triggered(bool)), this, SLOT(close()));
}

//----------------------------------------------------------------------------
void MainWindow::showAbout()
{
    AboutDialog dialog;
    dialog.exec();
}

//----------------------------------------------------------------------------
void MainWindow::showStatistics()
{
}

//----------------------------------------------------------------------------
void MainWindow::openConfiguration()
{
    ConfigurationDialog dialog(m_configuration, this);
    if (dialog.exec() != QDialog::Accepted) return;

    dialog.getConfiguration(m_configuration);
}

//----------------------------------------------------------------------------
void MainWindow::startWorkTimer()
{
}
