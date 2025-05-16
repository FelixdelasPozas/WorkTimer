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
#include <QEvent>
#include <QMessageBox>
#include <QCloseEvent>
#include <QMenu>
#include <QVBoxLayout>
#include <QLabel>
#include <QDialogButtonBox>

const QString TIME_FORMAT = "hh:mm:ss";

//----------------------------------------------------------------------------
MainWindow::MainWindow(QWidget* p, Qt::WindowFlags f) :
    QMainWindow{p, f},
    m_widget{false, this},
    m_taskBarButton{this}
{
    setupUi(this);

    m_taskBarButton.setMaximum(100);
    m_taskBarButton.setMinimum(100);
    m_taskBarButton.setState(QTaskBarButton::State::Invisible);
    m_taskBarButton.setValue(0);

    connectSignals();

    m_configuration.load();

    applyConfiguration();

    initIconAndMenu();

    initTables();
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
    connect(actionMinimize, SIGNAL(triggered(bool)), this, SLOT(close()));
    connect(actionAbout_WorkTimer, SIGNAL(triggered(bool)), this, SLOT(showAbout()));
    connect(actionConfiguration, SIGNAL(triggered(bool)), this, SLOT(openConfiguration()));
    connect(actionQuit, SIGNAL(triggered(bool)), this, SLOT(quitApplication()));
    connect(actionTask, SIGNAL(triggered(bool)), this, SLOT(onTaskNameClicked()));
    connect(&m_timer, SIGNAL(progress(unsigned int)), this, SLOT(onProgressUpdated(unsigned int)));
    connect(&m_timer, SIGNAL(sessionEnded()), this, SLOT(onSessionEnded()));

    connect(&m_timer, SIGNAL(workUnitEnded()), this, SLOT(onUnitEnded()));
    connect(&m_timer, SIGNAL(shortBreakEnded()), this, SLOT(onUnitEnded()));
    connect(&m_timer, SIGNAL(longBreakEnded()), this, SLOT(onUnitEnded()));
    connect(&m_timer, SIGNAL(beginWorkUnit()), this, SLOT(onUnitStarted()));
    connect(&m_timer, SIGNAL(beginShortBreak()), this, SLOT(onUnitStarted()));
    connect(&m_timer, SIGNAL(beginLongBreak()), this, SLOT(onUnitStarted()));
}

//----------------------------------------------------------------------------
void MainWindow::applyConfiguration()
{
    qobject_cast<ProgressWidget*>(m_progressBar)->setConfiguration(m_configuration);
    m_progressBar->repaint();

    m_widget.setVisible(false);
    m_widget.setPosition(m_configuration.m_widgetPosition);
    m_widget.setColor(m_configuration.m_workColor);
    m_widget.setIcon(":/WorkTimer/work.svg");
    m_widget.setProgress(0);
    m_widget.setOpacity(m_configuration.m_widgetOpacity);

    m_timer.setWorkUnitsBeforeBreak(4);
    m_timer.setLongBreakDuration(QTime{0, m_configuration.m_longBreakTime, 0, 0});
    m_timer.setShortBreakDuration(QTime{0, m_configuration.m_shortBreakTime, 0, 0});
    m_timer.setWorkDuration(QTime{0, m_configuration.m_workUnitTime, 0, 0});
    m_timer.setUseSounds(m_configuration.m_useSound);
    m_timer.setContinuousTicTac(m_configuration.m_continuousTicTac);
    m_timer.setSessionWorkUnits(m_configuration.m_unitsPerSession);

    m_totalMinutes = m_configuration.minutesInSession();
}

//----------------------------------------------------------------------------
void MainWindow::initTables()
{
    m_unitTable->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
    m_unitTable->horizontalHeader()->setSectionsMovable(false);
    m_unitTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_unitTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_unitTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_unitTable->verticalHeader()->setVisible(false);

    m_taskTable->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
    m_taskTable->horizontalHeader()->setSectionsMovable(false);
    m_taskTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_taskTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_taskTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_taskTable->verticalHeader()->setVisible(false);
}

//----------------------------------------------------------------------------
void MainWindow::initIconAndMenu()
{
    m_trayIcon = new QSystemTrayIcon{m_widget.asIcon(m_configuration.m_workUnitTime), this};
    m_trayIcon->hide();

    connect(m_trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this,
            SLOT(onTrayActivated(QSystemTrayIcon::ActivationReason)));

    auto menu = new QMenu(tr("Menu"));

    auto showAction = new QAction(QIcon(":/WorkTimer/maximize.svg"), tr("Restore..."));
    connect(showAction, SIGNAL(triggered(bool)), this, SLOT(onTrayActivated()));

    m_timerEntry = new QAction(QIcon(":/WorkTimer/play.svg"), tr("Start timer"));
    connect(m_timerEntry, SIGNAL(triggered(bool)), this, SLOT(onPlayClicked()));

    m_stopEntry = new QAction(QIcon(":/WorkTimer/stop.svg"), tr("Stop timer"));
    connect(m_stopEntry, SIGNAL(triggered(bool)), this, SLOT(onStopClicked()));
    m_stopEntry->setEnabled(false);

    m_taskEntry = new QAction(QIcon(":/WorkTimer/pencil.svg"), tr("Change task..."));
    connect(m_taskEntry, SIGNAL(triggered(bool)), this, SLOT(onTaskNameClicked()));
    m_taskEntry->setEnabled(false);

    auto aboutAction = new QAction(QIcon(":/WorkTimer/info.svg"), tr("About..."));
    connect(aboutAction, SIGNAL(triggered(bool)), this, SLOT(showAbout()));

    auto quitAction = new QAction(QIcon(":/WorkTimer/quit.svg"), tr("Quit"));
    connect(quitAction, SIGNAL(triggered(bool)), this, SLOT(quitApplication()));

    menu->addAction(showAction);
    menu->addSeparator();
    menu->addAction(m_timerEntry);
    menu->addAction(m_stopEntry);
    menu->addAction(m_taskEntry);
    menu->addSeparator();
    menu->addAction(aboutAction);
    menu->addSeparator();
    menu->addAction(quitAction);

    m_trayIcon->setContextMenu(menu);
    m_trayIcon->setIcon(QIcon(":/WorkTimer/clock.svg"));
    m_trayIcon->setToolTip(m_timer.statusMessage());
    m_trayIcon->hide();
}

//----------------------------------------------------------------------------
void MainWindow::updateItemTime(const QTime& timeToAdd, const int row, QTableWidget* table)
{
    if(!table || table->rowCount() < row)
        return;

    auto itemTime = QTime::fromString(table->item(row, 1)->text());
    itemTime = itemTime.addMSecs(QTime{0,0,0}.msecsTo(timeToAdd));
    table->item(row,1)->setText(itemTime.toString(TIME_FORMAT));
}

//----------------------------------------------------------------------------
void MainWindow::updateItemUnits(const int unitToAdd, const int row, QTableWidget* table)
{
    if(!table || table->rowCount() < row)
        return;

    auto itemUnits = atoi(table->item(row, 2)->text().toStdString().c_str());
    itemUnits += unitToAdd;
    table->item(row,2)->setText(QString("%1").arg(itemUnits));
}

//----------------------------------------------------------------------------
void MainWindow::onTrayActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (m_trayIcon->isVisible() && reason == QSystemTrayIcon::DoubleClick) {
        showNormal();
        m_trayIcon->hide();
    }
}

//----------------------------------------------------------------------------
void MainWindow::quitApplication()
{
    m_needsExit = true;
    if (this->isVisible()) {
        close();
    } else {
        closeEvent(nullptr);
    }
}

//----------------------------------------------------------------------------
void MainWindow::closeEvent(QCloseEvent* e)
{
    if (!m_needsExit) {
        hide();
        m_trayIcon->show();

        e->accept();
    } else {
        if (m_timer.status() != WorkTimer::Status::Stopped) {
            QMessageBox msgBox{this};
            msgBox.setWindowIcon(QIcon(":/WorkTimer/clock.svg"));
            msgBox.setIcon(QMessageBox::Icon::Information);
            msgBox.setText("The timer is still running. Do you really want to exit?");
            msgBox.setDefaultButton(QMessageBox::StandardButton::Ok);
            msgBox.setStandardButtons(QMessageBox::StandardButton::Cancel | QMessageBox::StandardButton::Ok);

            if (msgBox.exec() != QMessageBox::Ok) {
                return;
            }
        }

        if (e) {
            QMainWindow::closeEvent(e);
        }
        QApplication::exit(0);
    }
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
    applyConfiguration();
    m_progressBar->setValue(0);
}

//----------------------------------------------------------------------------
void MainWindow::onPlayClicked()
{
    if (m_taskTable->rowCount() == 0) {
        onTaskNameClicked();

        if (m_taskTable->rowCount() == 0) {
            return;
        }
    }

    switch (m_timer.status()) {
        case WorkTimer::Status::Stopped:
            m_timer.start();
            m_progressBar->setValue(0);
            actionTimer->setIcon(QIcon(":/WorkTimer/pause.svg"));
            m_timerEntry->setIcon(QIcon(":/WorkTimer/pause.svg"));
            m_timerEntry->setText("Pause timer");
            actionStop->setEnabled(true);
            m_stopEntry->setEnabled(true);
            actionConfiguration->setEnabled(false);
            break;
        case WorkTimer::Status::Work:
        case WorkTimer::Status::ShortBreak:
        case WorkTimer::Status::LongBreak:
            m_timer.pause();
            actionTimer->setIcon(QIcon(":/WorkTimer/play.svg"));
            m_timerEntry->setIcon(QIcon(":/WorkTimer/play.svg"));
            m_timerEntry->setText("Start timer");
            break;
        case WorkTimer::Status::Paused:
            m_timer.pause();
            actionTimer->setIcon(QIcon(":/WorkTimer/pause.svg"));
            m_timerEntry->setIcon(QIcon(":/WorkTimer/pause.svg"));
            m_timerEntry->setText("Pause timer");
            break;
        default:
            break;
    }

    actionTask->setEnabled(true);
    m_taskEntry->setEnabled(true);
    m_widget.setVisible(m_configuration.m_useWidget);
}

//----------------------------------------------------------------------------
void MainWindow::onStopClicked()
{
    m_timer.stop();
    actionStop->setEnabled(false);
    actionTimer->setIcon(QIcon(":/WorkTimer/play.svg"));
    m_widget.setVisible(false);
    m_widget.setProgress(0);
    m_trayIcon->setToolTip(m_timer.statusMessage());
    m_trayIcon->setIcon(QIcon(":/WorkTimer/clock.svg"));
    actionConfiguration->setEnabled(true);
    m_taskBarButton.setState(QTaskBarButton::State::Invisible);
    m_taskBarButton.setValue(0);
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

    const auto elapsedMS = m_timer.elapsed();

    bool insertedNew = false;
    const auto rows = m_taskTable->rowCount();
    if (rows == 0 || m_taskTable->item(rows - 1, 0)->text().compare(taskName, Qt::CaseInsensitive) != 0) {
        m_taskTable->insertRow(rows);
        auto item = new QTableWidgetItem(taskName);
        item->setTextAlignment(Qt::AlignCenter);
        m_taskTable->setItem(rows, 0, item);
        item = new QTableWidgetItem(QTime{0, 0, 0}.toString(TIME_FORMAT));
        item->setTextAlignment(Qt::AlignCenter);
        m_taskTable->setItem(rows, 1, item);
        item = new QTableWidgetItem("0");
        item->setTextAlignment(Qt::AlignCenter);
        m_taskTable->setItem(rows, 2, item);
        m_timer.setTaskTitle(taskName);
        m_widget.setTitle(taskName);
        insertedNew = true;
    }

    if(insertedNew && rows != 0)
    {
        const auto elapsedTime = QTime{0,0,0}.addMSecs(elapsedMS);
        updateItemTime(elapsedTime, rows - 1, m_taskTable);
        qDebug() << "enter" << rows-1 << elapsedTime << elapsedMS;
    }
}

//----------------------------------------------------------------------------
void MainWindow::onProgressUpdated(unsigned int value)
{
    m_widget.setProgress(value);

    const QTaskBarButton::State tbState = m_taskBarButton.state();
    const QTaskBarButton::State state = (value == 0) ? QTaskBarButton::State::Invisible : QTaskBarButton::State::Normal;

    if (tbState != state) {
        m_taskBarButton.setState(state);
    }
    m_taskBarButton.setValue(value);

    unsigned int progressSeconds = 0;
    unsigned int invMinutes = 0;
    switch (m_timer.status()) {
        default:
        case WorkTimer::Status::Work:
            progressSeconds = value / 100.f * m_configuration.m_workUnitTime * 60;
            invMinutes = m_configuration.m_workUnitTime - (progressSeconds / 60);
            break;
        case WorkTimer::Status::ShortBreak:
            progressSeconds = value / 100.f * m_configuration.m_shortBreakTime * 60;
            invMinutes = m_configuration.m_shortBreakTime - (progressSeconds / 60);
            break;
        case WorkTimer::Status::LongBreak:
            progressSeconds = value / 100.f * m_configuration.m_longBreakTime * 60;
            invMinutes = m_configuration.m_longBreakTime - (progressSeconds/60);
            break;
    }

    int globalProgress = std::nearbyint((static_cast<float>((m_globalProgress * 60) + progressSeconds) * 100) / (m_totalMinutes * 60));
    m_progressBar->setValue(globalProgress);
    m_trayIcon->setIcon(m_widget.asIcon(invMinutes));
    m_trayIcon->setToolTip(m_timer.statusMessage() + "\nTask: " + m_timer.getTaskTitle());
}

//----------------------------------------------------------------------------
void MainWindow::onUnitEnded()
{
    int row = 0;
    int seconds = 0;
    switch(m_timer.status())
    {
        case WorkTimer::Status::Work:
        {
            row = 1;
            seconds = m_configuration.m_workUnitTime * 60;
            m_globalProgress += m_configuration.m_workUnitTime;

            updateItemTime(QTime{0,0,0}.addSecs(seconds), m_taskTable->rowCount()-1, m_taskTable);
            updateItemUnits(1, m_taskTable->rowCount()-1, m_taskTable);
        }
            break;
        case WorkTimer::Status::ShortBreak:
            seconds = m_configuration.m_shortBreakTime * 60;
            m_globalProgress += m_configuration.m_shortBreakTime;
            break;
        case WorkTimer::Status::LongBreak:
            seconds = m_configuration.m_longBreakTime * 60;
            m_globalProgress += m_configuration.m_longBreakTime;
            break;
        case WorkTimer::Status::Stopped:
        case WorkTimer::Status::Paused:
            break;
    }

    m_progressBar->setValue((m_globalProgress * 100) / m_totalMinutes);

    updateItemTime(QTime{0,0,0}.addSecs(seconds), row, m_unitTable);
    updateItemUnits(1, row, m_unitTable);
}

//----------------------------------------------------------------------------
void MainWindow::onUnitStarted()
{
    unsigned int minutes = 0;
    QString iconMessage;
    switch(m_timer.status())
    {
        case WorkTimer::Status::Stopped:
        case WorkTimer::Status::Work:
            minutes = m_configuration.m_workUnitTime;   
            m_widget.setColor(m_configuration.m_workColor);
            m_widget.setIcon(":/WorkTimer/work.svg");
            m_widget.setTitle(m_timer.getTaskTitle());
            iconMessage = "Started a work unit";
            break;
        case WorkTimer::Status::ShortBreak:
            minutes = m_configuration.m_shortBreakTime;
            m_widget.setColor(m_configuration.m_shortBreakColor);
            m_widget.setIcon(":/WorkTimer/rest.svg");
            m_widget.setTitle("Short break");
            iconMessage = "Started a short break";
            break;
        case WorkTimer::Status::LongBreak:
            minutes = m_configuration.m_longBreakTime;
            m_widget.setColor(m_configuration.m_longBreakColor);
            m_widget.setIcon(":/WorkTimer/rest.svg");
            m_widget.setTitle("Long break");
            iconMessage = "Started a long break";
            break;
        default:
        case WorkTimer::Status::Paused:
            break;
    }

    m_widget.setProgress(0);
    m_trayIcon->setIcon(m_widget.asIcon(minutes));
    m_trayIcon->setToolTip(m_timer.statusMessage());

    if(m_trayIcon->isVisible() && m_configuration.m_iconMessages)
    {
        m_trayIcon->showMessage("WorkTimer", iconMessage, QSystemTrayIcon::MessageIcon::Information);
    }
}

//----------------------------------------------------------------------------
void MainWindow::onSessionEnded()
{
    onStopClicked();

    if(m_trayIcon->isVisible())
    {
        showNormal();
        m_trayIcon->hide();
    }

    FinishDialog d(this);
    d.exec();
}

//----------------------------------------------------------------------------
FinishDialog::FinishDialog(QWidget* parent, Qt::WindowFlags f)
: QDialog{parent,f}
{
    setWindowIcon(QIcon(":/WorkTimer/clock.svg"));

    setLayout(new QVBoxLayout());
    layout()->addWidget(new QLabel("The session has ended!", this));
    auto buttonBox = new QDialogButtonBox(this);
    buttonBox->setObjectName("buttonBox");
    buttonBox->setMaximumSize(QSize(575, 23));
    buttonBox->setOrientation(Qt::Orientation::Horizontal);
    buttonBox->setStandardButtons(QDialogButtonBox::StandardButton::Ok);

    layout()->addWidget(buttonBox);

    QObject::connect(buttonBox, &QDialogButtonBox::accepted, this, qOverload<>(&QDialog::accept));
    QObject::connect(buttonBox, &QDialogButtonBox::rejected, this, qOverload<>(&QDialog::reject));

    QMetaObject::connectSlotsByName(this);
}

//----------------------------------------------------------------------------
void FinishDialog::showEvent(QShowEvent *e)
{
    QDialog::showEvent(e);
    Utils::scaleDialog(this);
}