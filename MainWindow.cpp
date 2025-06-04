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
#include <PieChart.h>

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
#include <QChartView>
#include <QBarSet>
#include <QStackedBarSeries>
#include <QBarCategoryAxis>
#include <QValueAxis>

// SQLite
extern "C"
{
#include <sqlite3/sqlite3.h>
}

const QString TIME_FORMAT = "hh:mm:ss";
const QString SHORT_BREAK = "Short break";
const QString LONG_BREAK = "Long break";
const int CustomRole = Qt::UserRole+1;

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

    initCharts();

    updateChartsContents(QDateTime{}, QDateTime{});

    initTables();
}

//----------------------------------------------------------------------------
MainWindow::~MainWindow()
{
    m_configuration.save();
    if(m_configuration.m_database)
        sqlite3_shutdown();
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

    connect(tabWidget, SIGNAL(currentChanged(int)), this, SLOT(onTabChanged(int)));

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
void MainWindow::initCharts()
{
    // Pie tab
    auto pieTabContents = new QWidget();
    pieTabContents->setStyleSheet("QWidget{background-color: transparent;}");
    auto pieTabLayout = new QVBoxLayout();
    pieTabLayout->setContentsMargins(0,0,0,0);
    pieTabLayout->setSpacing(0);
    pieTabContents->setLayout(pieTabLayout);

    m_pieChart = new QChartView;
    m_pieChart->setRenderHint(QPainter::Antialiasing);
    m_pieChart->setBackgroundBrush(QBrush{Qt::transparent});
    m_pieChart->setRubberBand(QChartView::NoRubberBand);
    m_pieChart->setToolTip(tr("Statistics pie chart."));
    m_pieChart->setContentsMargins(0, 0, 0, 0);

    m_pieError = new QLabel(tr("No data found."));
    m_pieError->setVisible(false);

    pieTabLayout->addWidget(m_pieChart);
    pieTabLayout->addWidget(m_pieError);

    tabWidget->addTab(pieTabContents, QIcon(":/WorkTimer/chart.svg"), tr("Pie chart"));

    // Pie tab
    auto histogramTabContents = new QWidget();
    histogramTabContents->setStyleSheet("QWidget{background-color: transparent;}");
    auto histogramTabLayout = new QVBoxLayout();
    histogramTabLayout->setContentsMargins(0,0,0,0);
    histogramTabLayout->setSpacing(0);
    histogramTabContents->setLayout(histogramTabLayout);

    m_histogramChart = new QChartView;
    m_histogramChart->setRenderHint(QPainter::Antialiasing);
    m_histogramChart->setBackgroundBrush(QBrush{Qt::transparent});
    m_histogramChart->setRubberBand(QChartView::NoRubberBand);
    m_histogramChart->setToolTip(tr("Statistics pie chart."));
    m_histogramChart->setContentsMargins(0, 0, 0, 0);

    m_histogramError = new QLabel(tr("No data found."));
    m_histogramError->setVisible(false);

    histogramTabLayout->addWidget(m_histogramChart);
    histogramTabLayout->addWidget(m_histogramError);

    tabWidget->addTab(histogramTabContents, QIcon(":/WorkTimer/barchart.svg"), tr("Histogram"));    
}

//----------------------------------------------------------------------------
void MainWindow::updateItemTime(const QTime& timeToAdd, const int row, QTableWidget* table)
{
    if(!table || table->rowCount() < row)
        return;

    auto itemTime = QTime::fromString(table->item(row, 1)->text());
    itemTime = itemTime.addMSecs(QTime{0,0,0}.msecsTo(timeToAdd));
    table->item(row,1)->setText(itemTime.toString(TIME_FORMAT));

    if(table != m_unitTable)
    {
        const auto dateTime = table->item(row,3)->data(CustomRole).toDateTime();
        const auto taskMs = QTime{0,0,0}.msecsTo(itemTime);
        const auto taskName = table->item(row,0)->text().toStdString();
        
        Utils::insertUnitIntoDatabase(m_configuration, dateTime.toMSecsSinceEpoch(), taskName, taskMs);
    }
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
void MainWindow::insertItem(const QString& name)
{
    const auto rows = m_taskTable->rowCount();
    m_taskTable->insertRow(rows);
    auto item = new QTableWidgetItem(name);
    item->setTextAlignment(Qt::AlignCenter);
    m_taskTable->setItem(rows, 0, item);
    item = new QTableWidgetItem(QTime{0, 0, 0}.toString(TIME_FORMAT));
    item->setTextAlignment(Qt::AlignCenter);
    m_taskTable->setItem(rows, 1, item);
    item = new QTableWidgetItem("0");
    item->setTextAlignment(Qt::AlignCenter);
    m_taskTable->setItem(rows, 2, item);
    const auto dateTime = QDateTime::currentDateTime();
    item = new QTableWidgetItem(dateTime.time().toString(TIME_FORMAT));
    item->setData(CustomRole, dateTime);
    item->setTextAlignment(Qt::AlignCenter);
    m_taskTable->setItem(rows, 3, item);

    Utils::insertUnitIntoDatabase(m_configuration, QDateTime::currentDateTime().toMSecsSinceEpoch(), name.toStdString(), 0);
}

//----------------------------------------------------------------------------
void MainWindow::updateChartsContents(const QDateTime& , const QDateTime& )
{
    auto toSeconds = [](const QTime t){ return QTime{0,0,0}.secsTo(t); };
    
    auto today = QDateTime::currentDateTime();
    today.setTime(QTime{0,0,0});
    auto monday = today.addDays(1 - today.date().dayOfWeek());
    //const auto week = today.date().weekNumber();
    const auto units = Utils::taskHistogram(monday, monday.addDays(7), m_configuration);

    if(units.empty())
    {
        m_pieChart->hide();
        m_histogramChart->hide();
        m_pieError->show();
        m_histogramError->show();
        return;
    }

    // Pie chart
    std::map<QString, QTime> times;
    QTime totalTime = QTime{0,0,0};
    for (const auto &[t, values]: units) {
        for (const auto& unit : values) {

            const auto seconds = toSeconds(unit.duration);

            if(times.find(unit.name) == times.cend())
                times[unit.name] = QTime{0,0,0};
            
            times[unit.name] = times[unit.name].addSecs(seconds);
            totalTime = totalTime.addSecs(seconds);
        }
    }

    QPieSeries *restSeries = new QPieSeries();
    restSeries->setName("Rest");
    QPieSeries *workSeries = new QPieSeries();
    workSeries->setName("Work");

    for(const auto &[name, duration]: times)
    {
        auto serie = workSeries;
        if(name == LONG_BREAK || name == SHORT_BREAK)
            serie = restSeries;
        
        serie->append(Utils::toCamelCase(name), toSeconds(duration));
    }

    QFont font("Arial", 16);
    font.setBold(true);
    font.setUnderline(true);

    PieChart* donutBreakdown = new PieChart();
    donutBreakdown->setAnimationOptions(QChart::AllAnimations);
    donutBreakdown->setTitle(QString("Total time: %1").arg(totalTime.toString("hh:mm:ss")));
    donutBreakdown->setTitleFont(font);
    donutBreakdown->setBackgroundVisible(false);
    donutBreakdown->legend()->setAlignment(Qt::AlignRight);
    donutBreakdown->addBreakdownSeries(restSeries, QColor(79, 87, 112));
    donutBreakdown->addBreakdownSeries(workSeries, QColor(79, 112, 88));
    
    auto piechart = m_pieChart->chart();
    m_pieChart->setChart(donutBreakdown);
    if(piechart) delete piechart;

    // Histogram
    std::map<QString, QBarSet *> barsets;
    for(const auto &entry: times)
    {
        const auto color = entry.first == LONG_BREAK ? QColor(79, 87, 112) : (entry.first == SHORT_BREAK ? QColor(79, 87, 112).lighter() : QColor(79, 112, 88));
        barsets[entry.first] = new QBarSet(Utils::toCamelCase(entry.first));
        barsets[entry.first]->setColor(color);
        barsets[entry.first]->setBorderColor(color.darker());
    }

    auto fill = [](std::pair<QString, QBarSet *> entry){
        entry.second->insert(entry.second->count(), 0);
    };

    for (const auto &[t, values]: units) {
        std::for_each(barsets.begin(), barsets.end(), fill);
        const auto pos = barsets.begin()->second->count() - 1;

        for (const auto& unit : values) {
            const auto value = toSeconds(unit.duration) + barsets[unit.name]->at(pos);
            barsets[unit.name]->insert(pos, value);
        }
    }

    auto series = new QStackedBarSeries;
    for(auto &[name, barset]: barsets)
        series->append(barset);

    auto histChart = new QChart;
    histChart->addSeries(series);
    histChart->setTitle(QString("Histogram for the week %1 to %2").arg(monday.toString("dd/MM")).arg(monday.addDays(7).toString("dd/MM")));
    histChart->setTitleFont(font);
    histChart->setBackgroundVisible(false);
    histChart->setAnimationOptions(QChart::SeriesAnimations);
    histChart->legend()->setVisible(true);
    histChart->legend()->setAlignment(Qt::AlignBottom);

    QStringList dates;
    const unsigned long long ending = monday.addDays(7).toMSecsSinceEpoch();
    for(unsigned long long i = monday.toMSecsSinceEpoch(); i < ending;)
    {
        dates << monday.toString("dd/MM");
        monday = monday.addDays(1);
        i = monday.toMSecsSinceEpoch();
    }

    auto axisX = new QBarCategoryAxis;
    axisX->append(dates);
    axisX->setTitleText("Days");
    histChart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    auto axisY = new QValueAxis;
    series->attachAxis(axisY);
    axisY->setLabelFormat("%d");
    axisY->setTitleText("Hours");
    histChart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    auto histchart = m_histogramChart->chart();
    m_histogramChart->setChart(histChart);
    if(histchart) delete histchart;
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
void MainWindow::onTabChanged(int index)
{
    // TODO
    // refresh graphs
}

//----------------------------------------------------------------------------
void MainWindow::closeEvent(QCloseEvent* e)
{
    if (!m_needsExit) {
        hide();
        m_trayIcon->show();

        e->accept();
        return;
    }

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

        onStopClicked();
    }

    QApplication::exit(0);
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

    static bool taskChangeEnabled = true;

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
        case WorkTimer::Status::ShortBreak:
        case WorkTimer::Status::LongBreak:
            m_timer.pause();
            actionTimer->setIcon(QIcon(":/WorkTimer/play.svg"));
            m_timerEntry->setIcon(QIcon(":/WorkTimer/play.svg"));
            m_timerEntry->setText("Start timer");
            taskChangeEnabled = false;
            break;
        case WorkTimer::Status::Work:
            m_timer.pause();
            actionTimer->setIcon(QIcon(":/WorkTimer/play.svg"));
            m_timerEntry->setIcon(QIcon(":/WorkTimer/play.svg"));
            m_timerEntry->setText("Start timer");
            taskChangeEnabled = true;
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

    actionTask->setEnabled(taskChangeEnabled);
    m_taskEntry->setEnabled(taskChangeEnabled);
    m_widget.setVisible(m_configuration.m_useWidget);
}

//----------------------------------------------------------------------------
void MainWindow::onStopClicked()
{
    if(m_timer.status() != WorkTimer::Status::Stopped)
    {
        const auto elapsedMs = m_timer.elapsed();
        const auto elapsedTime = QTime{0, 0, 0}.addMSecs(elapsedMs);

        switch (m_timer.status()) {
            case WorkTimer::Status::Work:
                updateItemTime(elapsedTime, m_taskTable->rowCount() - 1, m_taskTable);
                break;
            case WorkTimer::Status::ShortBreak:
                Utils::insertUnitIntoDatabase(m_configuration, QDateTime::currentDateTime().toMSecsSinceEpoch() - elapsedMs,
                                            SHORT_BREAK.toStdString(), elapsedMs);
                break;
            case WorkTimer::Status::LongBreak:
                Utils::insertUnitIntoDatabase(m_configuration, QDateTime::currentDateTime().toMSecsSinceEpoch() - elapsedMs,
                                            LONG_BREAK.toStdString(), elapsedMs);
                break;
            default:
            case WorkTimer::Status::Paused:
                break;
        }
    }

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
        insertItem(taskName);
        m_timer.setTaskTitle(taskName);
        m_widget.setTitle(taskName);
        insertedNew = true;
    }

    if(insertedNew && rows != 0)
    {
        const auto elapsedTime = QTime{0,0,0}.addMSecs(elapsedMS);
        updateItemTime(elapsedTime, rows - 1, m_taskTable);
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
            Utils::insertUnitIntoDatabase(m_configuration, 1000 * (QDateTime::currentDateTime().toSecsSinceEpoch() - seconds), SHORT_BREAK.toStdString(), seconds * 1000);
            break;
        case WorkTimer::Status::LongBreak:
            seconds = m_configuration.m_longBreakTime * 60;
            m_globalProgress += m_configuration.m_longBreakTime;
            Utils::insertUnitIntoDatabase(m_configuration, 1000 * (QDateTime::currentDateTime().toSecsSinceEpoch() - seconds), LONG_BREAK.toStdString(), seconds * 1000);
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
    QString taskName;
    bool taskChangeEnabled = true;
    switch(m_timer.status())
    {
        case WorkTimer::Status::Stopped:
        case WorkTimer::Status::Work:
            minutes = m_configuration.m_workUnitTime;   
            m_widget.setColor(m_configuration.m_workColor);
            m_widget.setIcon(":/WorkTimer/work.svg");
            taskName = m_timer.getTaskTitle();
            iconMessage = "Started a work unit";
            break;
        case WorkTimer::Status::ShortBreak:
            minutes = m_configuration.m_shortBreakTime;
            m_widget.setColor(m_configuration.m_shortBreakColor);
            m_widget.setIcon(":/WorkTimer/rest.svg");
            taskName = SHORT_BREAK;
            iconMessage = "Started a short break";
            taskChangeEnabled = false;
            break;
        case WorkTimer::Status::LongBreak:
            minutes = m_configuration.m_longBreakTime;
            m_widget.setColor(m_configuration.m_longBreakColor);
            m_widget.setIcon(":/WorkTimer/rest.svg");
            taskName = LONG_BREAK;
            iconMessage = "Started a long break";
            taskChangeEnabled = false;
            break;
        default:
        case WorkTimer::Status::Paused:
            taskChangeEnabled = false;
            break;
    }

    m_widget.setProgress(0);
    m_widget.setTitle(taskName);
    actionTask->setEnabled(taskChangeEnabled);
    m_taskEntry->setEnabled(taskChangeEnabled);
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